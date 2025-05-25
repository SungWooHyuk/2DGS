#include "pch.h"
#include "RedisManager.h"
#include "GLogger.h"
#include "DBGameSession.h"
#include <sstream>

bool RedisManager::Init() {
    context = redisConnect("127.0.0.1", 6379);
    if (context == nullptr || context->err) {
        if (context) {
            printf("Error: %s\n", context->errstr);
        }
        else {
            printf("Can't allocate redis context\n");
        }
        GLogger::Log(spdlog::level::err, "[RedisManager] Init Fail");
        return false;
    }

    const char* password = "1234";
    redisReply* authReply = (redisReply*)redisCommand(context, "AUTH %s", password);
    if (!authReply || authReply->type == REDIS_REPLY_ERROR) {
        if (authReply) {
            printf("AUTH failed: %s\n", authReply->str);
            freeReplyObject(authReply);
        }
        GLogger::Log(spdlog::level::err, "[RedisManager] Auth Fail");
        return false;
    }

    freeReplyObject(authReply);
    return true;
}

void RedisManager::Close() {
    if (context) {
        redisFree(context);
        context = nullptr;
    }
}

bool RedisManager::UpdatePlayerGold(const string& _playerName, uint64 _newGold, vector<RankingData>& _outUpdatedTop5) {
    WRITE_LOCK;
    if (!context) {
        GLogger::Log(spdlog::level::err, "[RedisManager][UpdatePlayerGold] context Fail");
        return false;
    }
    auto oldTop5 = GetTop5Ranking();

    redisReply* reply = (redisReply*)redisCommand(context, "ZADD %s %llu %s", RANKING_KEY, _newGold, _playerName.c_str());

    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        printf("ZADD failed: %s\n", reply ? reply->str : "No reply");
        GLogger::Log(spdlog::level::err, "[RedisManager][UpdatePlayerGold] reply Fail");
        return false;
    }
    freeReplyObject(reply);

    auto newTop5 = GetTop5Ranking();

    bool wasInOldTop5 = any_of(oldTop5.begin(), oldTop5.end(),
        [&](const RankingData& data) {
            return data.playerName == _playerName;
        });

    bool isInNewTop5 = any_of(newTop5.begin(), newTop5.end(),
        [&](const RankingData& data) {
            return data.playerName == _playerName;
        });

    if (!wasInOldTop5 && isInNewTop5) {
        _outUpdatedTop5 = move(newTop5);
        return true;
    }

    if (oldTop5 != newTop5) {
        _outUpdatedTop5 = move(newTop5);
        return true;
    }

    return false;
}

void RedisManager::RegisterPlayerRanking(const string& _playerName, uint64 _gold)
{
    WRITE_LOCK;
    if (!context) {
        GLogger::Log(spdlog::level::err, "[RedisManager][RegisterPlayerRanking] context Fail");
        return;
    }
    redisReply* reply = (redisReply*)redisCommand(context,
        "ZADD %s %llu %s", RANKING_KEY, _gold, _playerName.c_str());

    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        GLogger::Log(spdlog::level::err, "[RedisManager][RegisterPlayerRanking] reply NULL || REDIS_REPLY_INTEGER");
        //printf("ZADD failed: %s\n", reply ? reply->str : "No reply");
    }

    if (reply) freeReplyObject(reply);
}

vector<RankingData> RedisManager::GetTop5Ranking() const{
    vector<RankingData> result;
    WRITE_LOCK;
    if (!context) {
        GLogger::Log(spdlog::level::err, "[RedisManager][GetTop5Ranking] context Fail");
        return result;
    }
    // ZREVRANGE를 사용하여 상위 5명 조회 (내림차순)
    redisReply* reply = (redisReply*)redisCommand(context, "ZREVRANGE %s 0 4 WITHSCORES",
        RANKING_KEY);

    if (!reply || reply->type != REDIS_REPLY_ARRAY) {
        printf("ZREVRANGE failed\n");
        if (reply) freeReplyObject(reply);
        return result;
    }

    for (size_t i = 0; i < reply->elements; i += 2) {
        RankingData data;
        data.playerName = reply->element[i]->str;
        data.gold = std::stoi(reply->element[i + 1]->str);
        result.push_back(data);
    }

    freeReplyObject(reply);
    return result;
}

bool RedisManager::EnqueuePacket(SendBufferRef _buffer)
{
    WRITE_LOCK;
    if (!context) {
        GLogger::Log(spdlog::level::err, "[RedisManager][EnqueuePacket] context Fail");
        return false;
    }

    redisReply* reply = (redisReply*)redisCommand(context,
        "RPUSH %s %b", PACKET_QUEUE_KEY, _buffer->Buffer(), _buffer->WriteSize());

    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        //printf("RPUSH failed: %s\n", reply ? reply->str : "No reply");
        GLogger::Log(spdlog::level::err, "[RedisManager][EnqueuePacket] reply NULL || REDIS_REPLY_INTEGER");
        if (reply) freeReplyObject(reply);
        return false;
    }

    freeReplyObject(reply);
    return true;
}

bool RedisManager::DequeuePacket(string& deSerializedPacket)
{
    WRITE_LOCK;
    if (!context) {
        GLogger::Log(spdlog::level::err, "[RedisManager][DequeuePacket] context Fail");
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand(context, "LPOP %s", PACKET_QUEUE_KEY);

    if (!reply || reply->type != REDIS_REPLY_STRING) {
        GLogger::Log(spdlog::level::err, "[RedisManager][DequeuePacket] reply NULL || REDIS_REPLY_STRING");
        if (reply) freeReplyObject(reply);
        return false;
    }

    if (reply->type == REDIS_REPLY_NIL) {
        GLogger::Log(spdlog::level::err, "[RedisManager][DequeuePacket] reply type REDIS_REPLY_NIL");
        freeReplyObject(reply);
        return false;
    }

    deSerializedPacket.assign(reply->str, reply->len);
    freeReplyObject(reply);
    return true;
}

bool RedisManager::IsPacketQueueEmpty()
{
    WRITE_LOCK;
    if (!context) {
        GLogger::Log(spdlog::level::err, "[RedisManager][IsPacketQueueEmpty] context Fail");
        return false;
    }
    redisReply* reply = (redisReply*)redisCommand(context, "LLEN %s", PACKET_QUEUE_KEY);

    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        GLogger::Log(spdlog::level::err, "[RedisManager][IsPacketQueueEmpty] reply Fail");
        if (reply) freeReplyObject(reply);
        return true;
    }

    bool isEmpty = (reply->integer == 0);
    freeReplyObject(reply);
    return isEmpty;
}

bool RedisManager::FlushQueue(DBGameSessionRef _dbsession)
{
    WRITE_LOCK;

    if (_dbsession == nullptr || _dbsession->IsConnected() == false || IsPacketQueueEmpty()) {   
        GLogger::Log(spdlog::level::err, "[RedisManager][FlushQueue] Fail");
        return false;
    }

    string rawPacket;

    while (DequeuePacket(rawPacket))
    {
        const int32 size = static_cast<int32>(rawPacket.size());

        SendBufferRef sendBuffer = GSendBufferManager->Open(size);
        ::memcpy(sendBuffer->Buffer(), rawPacket.data(), size);
        sendBuffer->Close(size);

        _dbsession->Send(sendBuffer);

        /*if (!_dbsession->Send(sendBuffer))
        {
            EnqueuePacket(sendBuffer);
            return false;
            break;
        }*/
    }
    return true;
}

bool RedisManager::ResetRanking()
{
    WRITE_LOCK;
    if (!context) {
        GLogger::Log(spdlog::level::err, "[RedisManager][ResetRanking] context Fail");
        return false;
    }
    redisReply* reply = (redisReply*)redisCommand(context, "DEL %s", RANKING_KEY);

    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        //printf("DEL ranking failed: %s\n", reply ? reply->str : "No reply");
        GLogger::Log(spdlog::level::err, "[RedisManager][ResetRanking] DEL ranking failed");
        if (reply) freeReplyObject(reply);
        return false;
    }

    bool success = (reply->integer > 0);
    freeReplyObject(reply);
    return success;
}
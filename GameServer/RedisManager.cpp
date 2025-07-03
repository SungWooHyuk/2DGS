#include "pch.h"
#include "RedisManager.h"
#include "GLogger.h"
#include "DBGameSession.h"
#include <sstream>

bool RedisManager::Init() {
    // 랭킹용 연결 초기화
    rankingContext = redisConnect("127.0.0.1", 6379);
    if (rankingContext == nullptr || rankingContext->err) {
        if (rankingContext) {
            printf("Ranking Redis Error: %s\n", rankingContext->errstr);
        }
        else {
            printf("Can't allocate ranking redis context\n");
        }
        GLogger::Log(spdlog::level::err, "[RedisManager] Ranking Init Fail");
        return false;
    }

    // 큐용 연결 초기화
    queueContext = redisConnect("127.0.0.1", 6379);
    if (queueContext == nullptr || queueContext->err) {
        if (queueContext) {
            printf("Queue Redis Error: %s\n", queueContext->errstr);
        }
        else {
            printf("Can't allocate queue redis context\n");
        }
        GLogger::Log(spdlog::level::err, "[RedisManager] Queue Init Fail");
        return false;
    }

    // 각 연결에 대해 인증
    if (!AuthenticateConnection(rankingContext) || !AuthenticateConnection(queueContext)) {
        return false;
    }

    return true;
}

bool RedisManager::AuthenticateConnection(redisContext* context) {
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

bool RedisManager::EnsureRankingConnection() {
    if (!rankingContext || rankingContext->err) {
        if (rankingRetryCount >= MAX_RETRY) {
            GLogger::Log(spdlog::level::err, "[RedisManager] Ranking max retry exceeded");
            return false;
        }

        rankingRetryCount++;

        if (rankingContext) {
            redisFree(rankingContext);
            rankingContext = nullptr;
        }

        rankingContext = redisConnect("127.0.0.1", 6379);
        if (!rankingContext || rankingContext->err) {
            GLogger::Log(spdlog::level::err, "[RedisManager] Ranking reconnection failed");
            return false;
        }

        if (!AuthenticateConnection(rankingContext)) {
            return false;
        }
    }

    rankingRetryCount = 0; // 성공시 리셋
    return true;
}

bool RedisManager::EnsureQueueConnection() {
    if (!queueContext || queueContext->err) {
        if (queueRetryCount >= MAX_RETRY) {
            GLogger::Log(spdlog::level::err, "[RedisManager] Queue max retry exceeded");
            return false;
        }

        queueRetryCount++;

        if (queueContext) {
            redisFree(queueContext);
            queueContext = nullptr;
        }

        queueContext = redisConnect("127.0.0.1", 6379);
        if (!queueContext || queueContext->err) {
            GLogger::Log(spdlog::level::err, "[RedisManager] Queue reconnection failed");
            return false;
        }

        if (!AuthenticateConnection(queueContext)) {
            return false;
        }
    }

    queueRetryCount = 0; // 성공시 리셋
    return true;
}

void RedisManager::Close() {
    if (rankingContext) {
        redisFree(rankingContext);
        rankingContext = nullptr;
    }
    if (queueContext) {
        redisFree(queueContext);
        queueContext = nullptr;
    }
}

bool RedisManager::UpdatePlayerGold(const string& _playerName, uint64 _newGold, vector<RankingData>& _outUpdatedTop5) {
    WRITE_LOCK;
    if (!EnsureRankingConnection()) {
        GLogger::Log(spdlog::level::err, "[RedisManager][UpdatePlayerGold] ranking context Fail");
        return false;
    }

    auto oldTop5 = GetTop5Ranking();

    redisReply* reply = (redisReply*)redisCommand(rankingContext, "ZADD %s %llu %s", RANKING_KEY, _newGold, _playerName.c_str());

    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        printf("ZADD failed: %s\n", reply ? reply->str : "No reply");
        GLogger::Log(spdlog::level::err, "[RedisManager][UpdatePlayerGold] reply Fail");
        if (reply) freeReplyObject(reply);
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
    if (!EnsureRankingConnection()) {
        GLogger::Log(spdlog::level::err, "[RedisManager][RegisterPlayerRanking] ranking context Fail");
        return;
    }

    redisReply* reply = (redisReply*)redisCommand(rankingContext,
        "ZADD %s %llu %s", RANKING_KEY, _gold, _playerName.c_str());

    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        GLogger::Log(spdlog::level::err, "[RedisManager][RegisterPlayerRanking] reply NULL || REDIS_REPLY_INTEGER");
    }

    if (reply) freeReplyObject(reply);
}

vector<RankingData> RedisManager::GetTop5Ranking() const {
    vector<RankingData> result;
    WRITE_LOCK;
    if (!const_cast<RedisManager*>(this)->EnsureRankingConnection()) {
        GLogger::Log(spdlog::level::err, "[RedisManager][GetTop5Ranking] ranking context Fail");
        return result;
    }

    redisReply* reply = (redisReply*)redisCommand(rankingContext, "ZREVRANGE %s 0 4 WITHSCORES", RANKING_KEY);

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
    if (!_buffer || _buffer->WriteSize() == 0) {
        GLogger::Log(spdlog::level::err, "[RedisManager][EnqueuePacket] Invalid buffer");
        return false;
    }

    WRITE_LOCK;

    if (!EnsureQueueConnection()) {
        GLogger::Log(spdlog::level::err, "[RedisManager][EnqueuePacket] queue context Fail");
        return false;
    }

    printf("Enqueueing packet with size: %d\n", _buffer->WriteSize());

    redisReply* reply = (redisReply*)redisCommand(queueContext,
        "RPUSH %s %b", PACKET_QUEUE_KEY, _buffer->Buffer(), _buffer->WriteSize());

    if (!reply) {
        GLogger::Log(spdlog::level::err, "[RedisManager][EnqueuePacket] reply is NULL");
        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER) {
        GLogger::Log(spdlog::level::err, "[RedisManager][EnqueuePacket] reply type: %d", reply->type);
        freeReplyObject(reply);
        return false;
    }

    cout << "EnqueuePacket Success, queue length: " << reply->integer << endl;
    freeReplyObject(reply);
    return true;
}

bool RedisManager::DequeuePacket(string& deSerializedPacket)
{
    WRITE_LOCK;
    if (!EnsureQueueConnection()) {
        GLogger::Log(spdlog::level::err, "[RedisManager][DequeuePacket] queue context Fail");
        return false;
    }

    redisReply* reply = (redisReply*)redisCommand(queueContext, "LPOP %s", PACKET_QUEUE_KEY);

    if (!reply) {
        GLogger::Log(spdlog::level::err, "[RedisManager][DequeuePacket] reply is NULL");
        return false;
    }

    if (reply->type == REDIS_REPLY_NIL) {
        GLogger::Log(spdlog::level::info, "[RedisManager][DequeuePacket] Queue is empty");
        freeReplyObject(reply);
        return false;
    }

    if (reply->type != REDIS_REPLY_STRING) {
        GLogger::Log(spdlog::level::err, "[RedisManager][DequeuePacket] reply type: %d", reply->type);
        freeReplyObject(reply);
        return false;
    }

    deSerializedPacket.assign(reply->str, reply->len);
    freeReplyObject(reply);
    cout << "DequeuePacket Success" << endl;
    return true;
}

bool RedisManager::IsPacketQueueEmpty()
{
    WRITE_LOCK;
    if (!EnsureQueueConnection()) {
        GLogger::Log(spdlog::level::err, "[RedisManager][IsPacketQueueEmpty] queue context Fail");
        return true;
    }

    redisReply* reply = (redisReply*)redisCommand(queueContext, "LLEN %s", PACKET_QUEUE_KEY);

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
    }
    return true;
}

bool RedisManager::ResetRanking()
{
    WRITE_LOCK;
    if (!EnsureRankingConnection()) {
        GLogger::Log(spdlog::level::err, "[RedisManager][ResetRanking] ranking context Fail");
        return false;
    }

    redisReply* reply = (redisReply*)redisCommand(rankingContext, "DEL %s", RANKING_KEY);

    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        GLogger::Log(spdlog::level::err, "[RedisManager][ResetRanking] DEL ranking failed");
        if (reply) freeReplyObject(reply);
        return false;
    }

    bool success = (reply->integer > 0);
    freeReplyObject(reply);
    return success;
}
#include "pch.h"
#include "RedisManager.h"
#include "GLogger.h"
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
        return false;
    }

    const char* password = "1234";
    redisReply* authReply = (redisReply*)redisCommand(context, "AUTH %s", password);
    if (!authReply || authReply->type == REDIS_REPLY_ERROR) {
        if (authReply) {
            printf("AUTH failed: %s\n", authReply->str);
            freeReplyObject(authReply);
        }
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
    if (!context) return false;

    auto oldTop5 = GetTop5Ranking();

    redisReply* reply = (redisReply*)redisCommand(context, "ZADD %s %llu %s", RANKING_KEY, _newGold, _playerName.c_str());

    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        printf("ZADD failed: %s\n", reply ? reply->str : "No reply");
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
    if (!context) return;

    redisReply* reply = (redisReply*)redisCommand(context,
        "ZADD %s %llu %s", RANKING_KEY, _gold, _playerName.c_str());

    if (!reply || reply->type != REDIS_REPLY_INTEGER)
        printf("ZADD failed: %s\n", reply ? reply->str : "No reply");

    if (reply) freeReplyObject(reply);
}

vector<RankingData> RedisManager::GetTop5Ranking() const{
    vector<RankingData> result;
    READ_LOCK;
    if (!context) return result;

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

bool RedisManager::ResetRanking()
{
    WRITE_LOCK;
    if (!context) return false;

    redisReply* reply = (redisReply*)redisCommand(context, "DEL %s", RANKING_KEY);

    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        printf("DEL ranking failed: %s\n", reply ? reply->str : "No reply");
        if (reply) freeReplyObject(reply);
        return false;
    }

    bool success = (reply->integer > 0);
    freeReplyObject(reply);
    return success;
}
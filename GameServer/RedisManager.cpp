#include "pch.h"
#include "RedisManager.h"
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
    return true;
}

void RedisManager::Close() {
    if (context) {
        redisFree(context);
        context = nullptr;
    }
}

bool RedisManager::UpdatePlayerLevel(const string& playerName, int32 level) {
    if (!context) return false;

    // 현재 플레이어의 점수를 확인
    redisReply* scoreReply = (redisReply*)redisCommand(context, "ZSCORE %s %s", RANKING_KEY, playerName.c_str());

    if (scoreReply->type == REDIS_REPLY_STRING) {
        // 플레이어가 존재함
        // 기존 점수를 삭제하고 새로운 점수로 업데이트
        redisReply* delReply = (redisReply*)redisCommand(context, "ZREM %s %s", RANKING_KEY, playerName.c_str());
        freeReplyObject(delReply);
    }
    
    freeReplyObject(scoreReply);

    std::stringstream ss;
    ss << level << ":" << playerName;  // 레벨:플레이어이름 형식으로 저장 (정렬을 위해)

    redisReply* reply = (redisReply*)redisCommand(context, "ZADD %s %s %s",
        RANKING_KEY, ss.str().c_str(), playerName.c_str());
    
    if (!reply) return false;

    bool result = (reply->type == REDIS_REPLY_INTEGER);
    freeReplyObject(reply);
    return result;
}

vector<RankingData> RedisManager::GetTop5Ranking() {
    vector<RankingData> result;
    if (!context) return result;

    // ZREVRANGE를 사용하여 상위 5명 조회 (내림차순)
    redisReply* reply = (redisReply*)redisCommand(context, "ZREVRANGE %s 0 4 WITHSCORES",
        RANKING_KEY);

    if (!reply || reply->type != REDIS_REPLY_ARRAY) {
        if (reply) freeReplyObject(reply);
        return result;
    }

    for (size_t i = 0; i < reply->elements; i += 2) {
        RankingData data;
        data.playerName = reply->element[i]->str;
        data.level = std::stoi(reply->element[i + 1]->str);
        result.push_back(data);
    }

    freeReplyObject(reply);
    return result;
}
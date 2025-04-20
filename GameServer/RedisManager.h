#pragma once
#include <hiredis.h>

struct RankingData {
    string playerName;
    int32 level;
};

class RedisManager {
public:
    static RedisManager& GetInstance() {
        static RedisManager instance;
        return instance;
    }

    bool Init();
    void Close();

    // 플레이어 레벨 업데이트
    bool UpdatePlayerLevel(const string& playerName, int32 level);
    
    // 상위 5명의 랭킹 조회
    vector<RankingData> GetTop5Ranking();

private:
    RedisManager() = default;
    ~RedisManager() { Close(); }
    RedisManager(const RedisManager&) = delete;
    RedisManager& operator=(const RedisManager&) = delete;

    redisContext* context = nullptr;
    const char* RANKING_KEY = "player_ranking";
}; 
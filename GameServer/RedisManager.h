#pragma once
#include <hiredis.h>
#include "utils.h"

class RedisManager {
public:
    static RedisManager& GetInstance() {
        static RedisManager instance;
        return instance;
    }

    bool Init();
    void Close();

    // 플레이어 골드 업데이트
    bool UpdatePlayerGold(const string& _playerName, uint64 _newGold, vector<RankingData>& _outUpdatedTop5);
    void RegisterPlayerRanking(const string& _playerName, uint64 _gold);
    bool ResetRanking();
    // 상위 5명의 랭킹 조회
    vector<RankingData> GetTop5Ranking() const;


private:
    RedisManager() = default;
    ~RedisManager() { Close(); }
    RedisManager(const RedisManager&) = delete;
    RedisManager& operator=(const RedisManager&) = delete;

    mutable     USE_LOCK;
    redisContext* context = nullptr;
    const char* RANKING_KEY = "player_ranking";

}; 
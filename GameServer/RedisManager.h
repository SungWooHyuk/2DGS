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


    bool EnqueuePacket(SendBufferRef _buffer); // Redis 큐에 패킷 push
    bool DequeuePacket(string& deSerializedPacket);    // Redis 큐에서 패킷 pop
    bool IsPacketQueueEmpty();          // 큐가 비었는지 확인
    bool FlushQueue(DBGameSessionRef _dbsession);

private:
    RedisManager() = default;
    ~RedisManager() { Close(); }
    RedisManager(const RedisManager&) = delete;
    RedisManager& operator=(const RedisManager&) = delete;

    // 연결 재시도 함수들
    bool EnsureRankingConnection();
    bool EnsureQueueConnection();
    bool AuthenticateConnection(redisContext* context);

    mutable USE_LOCK;

    // 분리된 Redis 연결
    redisContext* rankingContext = nullptr;  // 골드랭킹 전용
    redisContext* queueContext = nullptr;    // 패킷큐 전용

    // 재시도 카운터
    int queueRetryCount = 0;
    int rankingRetryCount = 0;
    const int MAX_RETRY = 3;

    const char* RANKING_KEY = "player_ranking";
    const char* PACKET_QUEUE_KEY = "packet_queue";  // 콜론 제거
};
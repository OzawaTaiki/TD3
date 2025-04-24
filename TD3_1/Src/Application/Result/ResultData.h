#pragma once


#include <cstdint>


enum class GameResult {
    Clear,
    GameOver
};

class ResultData
{
public:

    static ResultData* GetInstance();

    void Reset();

    void SetGameResult(GameResult result) { gameResult_ = result; }
    GameResult GetGameResult() const { return gameResult_; }

    void SetEnemyKillCount(int32_t count) { enemyKillCount_ = count; }
    int32_t GetEnemyKillCount() const { return enemyKillCount_; }

    void SetOtohimeCount(int32_t count) { otohimeCount_ = count; }
    int32_t GetOtohimeCount() const { return otohimeCount_; }

private:

    GameResult gameResult_ = GameResult::GameOver;
    int32_t enemyKillCount_ = 0;
    int32_t otohimeCount_ = 0;

};
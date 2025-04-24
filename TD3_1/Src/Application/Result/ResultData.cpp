#include "ResultData.h"

ResultData* ResultData::GetInstance()
{
    static ResultData instance;
    return &instance;
}


void ResultData::Reset()
{
    gameResult_ = GameResult::GameOver;
    enemyKillCount_ = 0;
    otohimeCount_ = 0;
}

#pragma once
#include <Features/Event/EventData.h>

struct WaveChangeData : public EventData
{
    int waveNumber = 0; // ウェーブ番号
};

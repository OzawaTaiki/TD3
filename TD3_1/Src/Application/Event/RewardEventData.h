#pragma once

#include <Features/Event/EventData.h>


enum class RewardItem
{
    None = 0 ,
    MovableObject,
    Haelth,

    ItemCount
};

static const char* rewardNames[] = {
    "None",
    "MovableObject",
    "Haelth"
};


struct ReawardEventData : public EventData
{
    ReawardEventData(RewardItem _item, int _count) : item(_item), count(_count) {}
    ~ReawardEventData() override = default;

    RewardItem item;
    int count;
};


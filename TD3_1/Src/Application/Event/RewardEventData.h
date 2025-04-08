#pragma once

#include <Features/Event/EventData.h>


enum class RewardItem
{
    MovableObject,

    ItemCount
};

struct ReawardEventData : public EventData
{
    ReawardEventData(RewardItem _item, int _count) : item(_item), count(_count) {}
    ~ReawardEventData() override = default;

    RewardItem item;
    int count;
};
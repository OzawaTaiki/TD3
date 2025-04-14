#pragma once

#include <Features/Event/EventData.h>

#include <string>

struct EnemyAttackInfo : public EventData
{
    std::string name;
    float damage;
};

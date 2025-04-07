#pragma once

#include <Features/Event/EventListener.h>

class LaunchKillEvent : public iEventListener
{
public:
    LaunchKillEvent();
    ~LaunchKillEvent() override;

    void OnEvent(const GameEvent& _event) override;

private:

};
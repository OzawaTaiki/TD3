#include "LaunchKillEvent.h"

#include <Features/Event/EventManager.h>


LaunchKillEvent::LaunchKillEvent()
{
    EventManager::GetInstance()->AddEventListener("LaunchKill", this);
}

LaunchKillEvent::~LaunchKillEvent()
{
    EventManager::GetInstance()->RemoveEventListener("LaunchKill", this);
}

void LaunchKillEvent::OnEvent(const GameEvent& _event)
{
    if (_event.GetEventType() == "LaunchKill")
    {
    }
}

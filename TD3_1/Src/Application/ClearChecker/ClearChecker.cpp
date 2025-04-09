#include "ClearChecker.h"

#include <Features/Event/EventManager.h>

ClearChecker::ClearChecker()
{
    EventManager::GetInstance()->AddEventListener("EnemyCleared", this);
    EventManager::GetInstance()->AddEventListener("WaveCompleted", this);
    EventManager::GetInstance()->AddEventListener("WaveStart", this);

    waveCompleted = false;
    enemysCleared = false;
}

ClearChecker::~ClearChecker()
{
    EventManager::GetInstance()->RemoveEventListener("EnemyCleared", this);
    EventManager::GetInstance()->RemoveEventListener("WaveCompleted", this);
    EventManager::GetInstance()->RemoveEventListener("WaveStart", this);
}

void ClearChecker::OnEvent(const GameEvent& _event)
{
    if (_event.GetEventType() == "EnemyCleared")
    {
        enemysCleared = true;
    }
    else if (_event.GetEventType() == "WaveCompleted")
    {
        waveCompleted = true;
    }
    else if (_event.GetEventType() == "WaveStart")
    {
        waveCompleted = false;
        enemysCleared = false;
    }
}

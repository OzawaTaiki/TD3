#pragma once

#include <Features/Event/EventListener.h>

class ClearChecker : public iEventListener
{
public:
    ClearChecker();
    ~ClearChecker();
    void OnEvent(const GameEvent& _event) override;

    bool IsCleared() const { return waveCompleted && enemysCleared; }

private:

    bool waveCompleted = false;
    bool enemysCleared = false;

};

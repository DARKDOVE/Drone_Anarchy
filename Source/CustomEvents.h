#ifndef CUSTOMEVENTS_H
#define CUSTOMEVENTS_H

#include <Object.h>



//Custom Events
EVENT(E_PLAYERHIT, PlayerHit)
{
    PARAM(P_CURRENTHEALTHFRACTION,CurrentHealthFraction );
}

EVENT(E_DRONEDESTROYED, DroneDestroyed)
{
    PARAM(P_DRONEPOSITION, DronePosition);
}

EVENT(E_DRONEHIT, DroneHit)
{
}

EVENT(E_COUNTFINISHED, CountFinished)
{
}

#endif // CUSTOMEVENTS_H

#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

#include <LogicComponent.h>
#include <Engine.h>
#include "CustomEvents.h"

using namespace Urho3D;

class PlayerObject : public LogicComponent
{
    OBJECT(PlayerObject)

public:
    PlayerObject(Context* context);
    void OnHit();


private:
    float maximumHealth_;
    float currentHealth_;
    float healthDeductionRate_;
};



class DroneObject : public LogicComponent
{
    OBJECT(DroneObject)

public:
    DroneObject(Context* context);
    void OnHit();
    void DelayedStart();
    void FixedUpdate(float timeStep);
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);

private:
    void Destroy();
    void SetupNodeAnimation();
    void Attack();


    float currentHealthLevel_;
    float attackTime_;
    float attackTimer_;

    bool hasAttacked_;
};



class BulletObject : public LogicComponent
{
    OBJECT(BulletObject)

public:
    BulletObject(Context* context);
    void Start();
    void FixedUpdate(float timeStep);
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);


private:

    void Destroy();

    float termTime_;
    float termTimeCounter_;

};


class ExplosionObject : public LogicComponent
{
    OBJECT(ExplosionObject)

public:
    ExplosionObject(Context* context);
    void FixedUpdate(float timeStep);

private:
    float duration_;

};

#endif // GAMEOBJECTS_H

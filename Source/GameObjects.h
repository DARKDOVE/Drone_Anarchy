//
// Copyright (c) 2014 - 2015 Drone Anarchy.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H


using namespace Urho3D;

//Drone Types Enum
enum DroneType
{
    DT_NORMAL_DRONE,
    DT_ALPHA_DRONE,
    DT_SUPREME_DRONE,
    DT_TOKEN_DRONE
};

//Bullet Type Enum
enum BulletObjectType
{
    BOT_LOW,
    BOT_NORMAL,
    BOT_POWERFUL
};


//Bullet Object Base
class BulletObjectBase : public LogicComponent
{

    OBJECT(BulletObjectBase);

public:
    BulletObjectBase(Context* context);
    void DelayedStart();
    virtual void Initialise(){}
    void FixedUpdate(float timestep);
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);

protected:
    void Destroy();

    float termTime_;
    float termTimeCounter_;
    BulletObjectType bulletObjectType_;
    float damagePoint_;

};


//Low Level Bullet Object
class LowLevelBullet : public BulletObjectBase
{
    OBJECT(LowLevelBullet);

public:
    LowLevelBullet(Context* context);

private:
    void Initialise();

};




//Weapon Base Class
class WeaponObjectBase : public Object
{
    OBJECT(WeaponObjectBase);

public:
    WeaponObjectBase(Context *context, Node* refNode);
    virtual void Fire(){}

    Node* refNode_;
};


//Ordinary Weapon
class OrdinaryWeapon : public WeaponObjectBase
{
    OBJECT(OrdinaryWeapon);

public:
    OrdinaryWeapon(Context* context,Node* refNode);
    void Fire();
    void SpawnBullet(bool first);
};




// Explosion Object base

class ExplosionObjectBase : public LogicComponent
{
    OBJECT(ExplosionObjectBase);

public:
    ExplosionObjectBase(Context* context);
    void DelayedStart();

protected:
    virtual void Initialise(){}

    float duration_;
};


//Simple Explosion Object
class SimpleExplosion : public ExplosionObjectBase
{

    OBJECT(SimpleExplosion);

public:
    SimpleExplosion(Context* context);
    void FixedUpdate(float timestep);

private:
    void Initialise();


};


class PlayerObject : public LogicComponent
{
    OBJECT(PlayerObject);

public:
    PlayerObject(Context* context);
    void OnHit(float damagePoint);
    void DelayedStart();

    void HandleActivateWeapon(StringHash eventType, VariantMap& eventData);
    void HandlePlayerRotation(StringHash eventType, VariantMap& eventData);

private:
    void Initialise();
    void SetWeapon(WeaponObjectBase* weapon);

    float maximumHealth_;
    float currentHealth_;
    WeaponObjectBase* weapon_;



};

class DroneObjectBase : public LogicComponent
{
  OBJECT(DroneObjectBase);

public:
    DroneObjectBase(Context* context);
    DroneType GetType(){return droneType_;}
    void OnHit(float damagePoint);
    void DelayedStart();
    void FixedUpdate(float timeStep);
    virtual void HandleNodeCollision(StringHash eventType, VariantMap& eventData){}

protected:
    virtual void Initialise() = 0;
    virtual void OnDestroyed(){}
    void Destroy();
    virtual void Attack() {}


    float currentHealthLevel_;
    float attackTime_;
    float attackTimer_;
    DroneType droneType_;

    bool hasAttacked_;
};


///Low Level Drone Object
class LowLevelDrone : public DroneObjectBase
{

    OBJECT(LowLevelDrone);

public:
    LowLevelDrone(Context* context);
    void DelayedStart();
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);

protected:
    virtual void Initialise();

private:
    void SetupNodeAnimation();
    void Attack();
    void OnDestroyed();
    void SpawnExplosion();

    int dronePoint_;
    int damagePoint_;


};

#endif // GAMEOBJECTS_H

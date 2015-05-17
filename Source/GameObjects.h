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

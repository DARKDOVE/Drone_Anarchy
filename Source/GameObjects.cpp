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




#include "GameObjects.h"

#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/UI/Sprite.h>


PlayerObject::PlayerObject(Context* context):
    LogicComponent(context)
{
    maximumHealth_ = currentHealth_ = 100;
    healthDeductionRate_ = 2;
}

void PlayerObject::OnHit()
{
    currentHealth_ -= healthDeductionRate_;
    if(currentHealth_ < 0.0f )
    {
        currentHealth_ = 0.0f;
    }

    float healthFraction = currentHealth_ / maximumHealth_;
    VariantMap eventData;
    using namespace PlayerHit;

    eventData[P_CURRENTHEALTHFRACTION] = healthFraction;
    SendEvent(E_PLAYERHIT, eventData);
}





//Drone Object

DroneObject::DroneObject(Context *context):
    LogicComponent(context)
{
    currentHealthLevel_ = 4;
    attackTime_ = 37;
    attackTimer_ = 0;
    hasAttacked_ = false;
    SetUpdateEventMask(USE_FIXEDUPDATE);
}


void DroneObject::DelayedStart()
{
    SetupNodeAnimation();
    SubscribeToEvent(GetNode(), E_NODECOLLISION, HANDLER(DroneObject, HandleNodeCollision));
}


void DroneObject::SetupNodeAnimation()
{
    ValueAnimation* valAnim = new ValueAnimation(context_);

    valAnim->SetKeyFrame(0.0f, Variant(GetNode()->GetPosition()));
    valAnim->SetKeyFrame(90.0f, Variant(GetNode()->GetRotation() * Vector3(0,0,-35)));
    GetNode()->SetAttributeAnimation("Position", valAnim);
}



void DroneObject::FixedUpdate(float timeStep)
{
    if(currentHealthLevel_ <= 0)
    {
        using namespace DroneDestroyed;

        //Add explosion Effects
        VariantMap eventData;
        eventData[P_DRONEPOSITION] = GetNode()->GetWorldPosition();
        SendEvent(E_DRONEDESTROYED, eventData);
        Destroy();
    }
    else if(!hasAttacked_)
    {
        attackTimer_ += timeStep;
        if(attackTimer_ >= attackTime_)
        {
            Attack();
        }
    }
}



void DroneObject::HandleNodeCollision(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    using namespace NodeCollision;
    Node* otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());
    PlayerObject* playerObj = static_cast<PlayerObject*>(otherNode->GetComponent<PlayerObject>());

    if(playerObj != NULL)
    {
        playerObj->OnHit();
        Destroy();
    }
}




void DroneObject::Attack()
{
    GetNode()->SetAnimationEnabled(false);

    AnimationController* animController = GetComponent<AnimationController>();
    animController->PlayExclusive("DroneAnarchy/Resources/Models/close_arm.ani", 0, false);
    RigidBody* objectBody = GetComponent<RigidBody>();
    objectBody->SetKinematic(false);

    objectBody->SetLinearVelocity(GetNode()->GetRotation() * Vector3(0,-15, -35));
    hasAttacked_ = true;
}


void DroneObject::Destroy()
{
    Sprite* nodeSprite = static_cast<Sprite*>(GetNode()->GetVar("Sprite").GetPtr());
    nodeSprite->Remove();
    GetNode()->Remove();
}


void DroneObject::OnHit()
{
    currentHealthLevel_ -= 1;
}




//Bullet Object

BulletObject::BulletObject(Context *context):
    LogicComponent(context)
{
    termTime_ = 1;
    termTimeCounter_ = 0;
}

void BulletObject::Start()
{
    SubscribeToEvent(GetNode(), E_NODECOLLISION, HANDLER(BulletObject, HandleNodeCollision));
}

void BulletObject::FixedUpdate(float timeStep)
{
    termTimeCounter_ += timeStep;

    if(termTimeCounter_ >= termTime_)
    {
        Destroy();
    }
}

void BulletObject::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
    using namespace NodeCollision;

    Node* otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());
    DroneObject* droneObj = static_cast<DroneObject*>(otherNode->GetComponent<DroneObject>());

    if(droneObj != NULL)
    {
        using namespace DroneHit;

        droneObj->OnHit();
        SendEvent(E_DRONEHIT);
    }

    Destroy();
}


void BulletObject::Destroy()
{
    GetNode()->Remove();
}





//Explosion Object

ExplosionObject::ExplosionObject(Context *context):
    LogicComponent(context)
{
    duration_ = 0.78;
}

void ExplosionObject::FixedUpdate(float timeStep)
{
    duration_ -= timeStep;

    if(duration_ < 0.0f)
    {
        GetNode()->Remove();
    }
}

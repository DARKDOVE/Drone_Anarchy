#include "GameObjects.h"
#include <Node.h>
#include <PhysicsEvents.h>
#include <ValueAnimation.h>
#include <AnimationController.h>
#include <RigidBody.h>
#include <Object.h>
#include <Sprite.h>


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

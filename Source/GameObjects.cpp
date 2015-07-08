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




#include <Urho3D/Urho3D.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/AnimatedModel.h>

#include <Urho3D/Scene/Scene.h>

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Material.h>

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Engine/Engine.h>

#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>

#include <Urho3D/IO/Log.h>

#include "EventsAndDefs.h"
#include "GameObjects.h"


PlayerObject::PlayerObject(Context* context):
    LogicComponent(context)
{
    maximumHealth_ = currentHealth_ = 100;
}

void PlayerObject::DelayedStart()
{
    SubscribeToEvent(E_ACTIVATEWEAPON, HANDLER(PlayerObject, HandleActivateWeapon));
    SubscribeToEvent(E_PLAYERROTATION, HANDLER(PlayerObject, HandlePlayerRotation));
    Initialise();
}

void PlayerObject::Initialise()
{
    Node* node = GetNode();

    RigidBody* playerBody  = node->CreateComponent<RigidBody>();
    playerBody->SetCollisionLayerAndMask(PLAYER_COLLISION_LAYER, DRONE_COLLISION_LAYER);
    CollisionShape* playerColShape = node->CreateComponent<CollisionShape>();
    playerColShape->SetSphere(2);

    Node* cameraNode = node->GetChild("CameraNode");

    SetWeapon(new OrdinaryWeapon(context_,cameraNode));

    Node* lightNode = cameraNode->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
    Light* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
}

void PlayerObject::HandleActivateWeapon(StringHash eventType, VariantMap& eventData)
{
    weapon_->Fire();
}

void PlayerObject::HandlePlayerRotation(StringHash eventType, VariantMap& eventData)
{
    using namespace PlayerRotation;

    int dx = eventData[P_DX].GetInt();
    int dy = eventData[P_DY].GetInt();

    Node* node = GetNode();

    Node* cameraNode = node->GetChild("CameraNode");

    float playerYaw = node->GetRotation().YawAngle() + (dx * 0.25f);
    float camPitch = cameraNode->GetRotation().PitchAngle() + (dy * 0.25f);

    camPitch = Clamp(camPitch, -20.0f, 70.0f);

    cameraNode->SetRotation( Quaternion(camPitch, 0.0f, 0.0f));
    node->SetRotation(Quaternion(0.0f, playerYaw, 0.0f));
}


void PlayerObject::SetWeapon(WeaponObjectBase* weapon)
{
    weapon_ = weapon;
}
void PlayerObject::OnHit(float damagePoint)
{
    currentHealth_ -= damagePoint;
    if(currentHealth_ < 0.0f )
    {
        //if health is equal to or below zero notify that the player
        //has been destroyed
        currentHealth_ = 0.0f;

        using namespace PlayerDestroyed;

        VariantMap eventData;

        Node* node = GetNode();
        Node* cameraNode = node->GetChild("CameraNode");
        eventData[P_CAMPOSITION] = cameraNode->GetWorldPosition();
        eventData[P_CAMROTATION] = cameraNode->GetWorldRotation();

        eventData[P_PLAYERPOSITION] = node->GetWorldPosition();
        eventData[P_PLAYERROTATION] = node->GetWorldRotation();

        SendEvent(E_PLAYERDESTROYED,eventData);

        node->GetChild("CameraNode")->GetChild("DirectionalLight")->SetEnabled(false);
    }

    using namespace PlayerHit;

    float healthFraction = currentHealth_ / maximumHealth_;

    VariantMap eventData;
    eventData[P_CURRENTHEALTHFRACTION] = healthFraction;

    SendEvent(E_PLAYERHIT, eventData);

}


//Drone Object base

DroneObjectBase::DroneObjectBase(Context *context)
    :LogicComponent(context)
{

}

void DroneObjectBase::OnHit(float damagePoint)
{
    currentHealthLevel_ -= damagePoint;
}

void DroneObjectBase::DelayedStart()
{
    SubscribeToEvent(GetNode(), E_NODECOLLISION, HANDLER(DroneObjectBase, HandleNodeCollision));
    Initialise();
}

void DroneObjectBase::FixedUpdate(float timestep)
{
    if(currentHealthLevel_ <= 0)
    {
        //Add explosion Effects
        OnDestroyed();
        Destroy();
    }
    else if(!hasAttacked_)
    {
        attackTimer_ += timestep;
        if(attackTimer_ >= attackTime_)
        {
            Attack();
        }
    }
}

void DroneObjectBase::Destroy()
{
    Sprite* nodeSprite = static_cast<Sprite*>(GetNode()->GetVar("Sprite").GetPtr());
    nodeSprite->Remove();
    GetNode()->Remove();
}



//Low Level Drone

LowLevelDrone::LowLevelDrone(Context *context)
    :DroneObjectBase(context)
{
    droneType_ = DT_NORMAL_DRONE;
    currentHealthLevel_ = 6;
    attackTime_ = 37;
    attackTimer_ = 0;
    hasAttacked_ = false;
    dronePoint_ = 2;
    damagePoint_ = 2;

}


void LowLevelDrone::DelayedStart()
{

    DroneObjectBase::DelayedStart();
    SetupNodeAnimation();
}

void LowLevelDrone::SetupNodeAnimation()
{
    ValueAnimation* valAnim = new ValueAnimation(context_);

    valAnim->SetKeyFrame(0.0f, Variant(GetNode()->GetPosition()));
    valAnim->SetKeyFrame(90.0f, Variant(GetNode()->GetRotation() * Vector3(0,0,-35)));
    GetNode()->SetAttributeAnimation("Position", valAnim);
}


void LowLevelDrone::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
    using namespace NodeCollision;
    Node* otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());
    PlayerObject* playerObj = static_cast<PlayerObject*>(otherNode->GetComponent<PlayerObject>());

    if(playerObj != NULL)
    {
        playerObj->OnHit(damagePoint_);
        Destroy();
    }
}

void LowLevelDrone::Attack()
{
    GetNode()->SetAnimationEnabled(false);

    AnimationController* animController = GetComponent<AnimationController>();
    animController->PlayExclusive("Resources/Models/close_arm.ani", 0, false);
    RigidBody* objectBody = GetComponent<RigidBody>();
    objectBody->SetKinematic(false);

    objectBody->SetLinearVelocity(GetNode()->GetRotation() * Vector3(0,-15, -35));
    hasAttacked_ = true;
}

void LowLevelDrone::Initialise()
{
    Node* droneNode = GetNode();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    AnimatedModel* droneBody = droneNode->CreateComponent<AnimatedModel>();
    droneBody->SetModel(cache->GetResource<Model>("DroneAnarchy/Resources/Models/drone_body.mdl"));
    droneBody->SetMaterial(cache->GetResource<Material>("DroneAnarchy/Resources/Materials/drone_body.xml"));

    AnimatedModel* droneArm = droneNode->CreateComponent<AnimatedModel>();
    droneArm->SetModel(cache->GetResource<Model>("DroneAnarchy/Resources/Models/drone_arm.mdl"));
    droneArm->SetMaterial(cache->GetResource<Material>("DroneAnarchy/Resources/Materials/drone_arm.xml"));

    RigidBody* droneRB = droneNode->CreateComponent<RigidBody>();
    droneRB->SetMass(1.0f);
    droneRB->SetCollisionLayerAndMask(DRONE_COLLISION_LAYER, BULLET_COLLISION_LAYER | PLAYER_COLLISION_LAYER | FLOOR_COLLISION_LAYER);
    droneRB->SetKinematic(true);

    CollisionShape* droneCS = droneNode->CreateComponent<CollisionShape>();
    droneCS->SetSphere(0.3f);

    AnimationController* animController = droneNode->CreateComponent<AnimationController>();
    animController->PlayExclusive("DroneAnarchy/Resources/Models/open_arm.ani", 0, false);
}

void LowLevelDrone::OnDestroyed()
{
    using namespace DroneDestroyed;
    VariantMap eventData;
    eventData[P_DRONEPOINT] = dronePoint_;
    SendEvent(E_DRONEDESTROYED, eventData);

    SpawnExplosion();
}

void LowLevelDrone::SpawnExplosion()
{
    Node* explosionNode = GetNode()->GetScene()->CreateChild("ExplosionNode");
    explosionNode->SetWorldPosition(GetNode()->GetWorldPosition());
    explosionNode->CreateComponent<SimpleExplosion>();


}


// Explosion Object Base

ExplosionObjectBase::ExplosionObjectBase(Context *context)
    :LogicComponent(context)
{

}

void ExplosionObjectBase::DelayedStart()
{
    Initialise();
}

SimpleExplosion::SimpleExplosion(Context *context)
    : ExplosionObjectBase(context)
{
    duration_ = 0.78f;
}

void SimpleExplosion::FixedUpdate(float timestep)
{
    duration_ -= timestep;
    if(duration_ < 0.0f)
    {
        GetNode()->Remove();
    }
}

void SimpleExplosion::Initialise()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Node* explosionNode = GetNode()->CreateChild("ExplosionNode");
    explosionNode->SetWorldPosition(GetNode()->GetWorldPosition());

    ParticleEmitter* pEmitter = explosionNode->CreateComponent<ParticleEmitter>();
    pEmitter->SetEffect(cache->GetResource<ParticleEffect>("DroneAnarchy/Resources/Particles/explosion.xml"));
    pEmitter->SetEnabled(true);

    using namespace SoundGenerated;

    VariantMap eventData;
    eventData[P_SOUNDNODE] = GetNode();
    eventData[P_SOUNDNAME] = "Resources/Sounds/explosion.ogg";
    SendEvent(E_SOUNDGENERATED, eventData);
}



BulletObjectBase::BulletObjectBase(Context *context)
    : LogicComponent(context)
{

}

void BulletObjectBase::DelayedStart()
{
    SubscribeToEvent(GetNode(), E_NODECOLLISION, HANDLER(BulletObjectBase, HandleNodeCollision));
    Initialise();
}


void BulletObjectBase::FixedUpdate(float timestep)
{
    termTimeCounter_ += timestep;

    if(termTimeCounter_ >= termTime_)
    {
        Destroy();
    }
}


void BulletObjectBase::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{

    using namespace NodeCollision;

    Node* otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());
    DroneObjectBase* droneObj = otherNode->GetDerivedComponent<DroneObjectBase>();

    if(droneObj != NULL)
    {
        droneObj->OnHit(damagePoint_);
    }

    Destroy();
}


void BulletObjectBase::Destroy()
{
    GetNode()->Remove();
}




LowLevelBullet::LowLevelBullet(Context *context)
    : BulletObjectBase(context)
{
    bulletObjectType_ = BOT_LOW;
    termTime_ = 1;
    termTimeCounter_ = 0;
    damagePoint_ = 1;
}


void LowLevelBullet::Initialise()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    BillboardSet* bbSet = GetNode()->CreateComponent<BillboardSet>();
    bbSet->SetNumBillboards(1);
    bbSet->SetMaterial(cache->GetResource<Material>("DroneAnarchy/Resources/Materials/bullet_particle.xml"));

    ParticleEmitter* pEmitter = GetNode()->CreateComponent<ParticleEmitter>();
    pEmitter->SetEffect(cache->GetResource<ParticleEffect>("DroneAnarchy/Resources/Particles/bullet_particle.xml"));
    pEmitter->SetEnabled(true);

    RigidBody* bulletRB = GetNode()->CreateComponent<RigidBody>();
    bulletRB->SetMass(1.0f);
    bulletRB->SetTrigger(true);
    bulletRB->SetUseGravity(false);

    bulletRB->SetCcdRadius(0.05f);
    bulletRB->SetCcdMotionThreshold(0.15f);
    bulletRB->SetCollisionLayerAndMask(BULLET_COLLISION_LAYER, DRONE_COLLISION_LAYER | FLOOR_COLLISION_LAYER);

    CollisionShape* bulletCS = GetNode()->CreateComponent<CollisionShape>();
    bulletCS->SetSphere(0.3f);

    bulletRB->SetLinearVelocity(GetNode()->GetRotation() * Vector3(0,0,70));


}


WeaponObjectBase::WeaponObjectBase(Context *context, Node* refNode)
    : Object(context)
{
    refNode_ = refNode;
}



OrdinaryWeapon::OrdinaryWeapon(Context *context,Node* refNode)
    : WeaponObjectBase(context, refNode)
{

}


void OrdinaryWeapon::Fire()
{
    SpawnBullet(true);
    SpawnBullet(false);

    using namespace SoundGenerated;

    VariantMap eventData;
    eventData[P_SOUNDNODE] = refNode_;
    eventData[P_SOUNDNAME] = "Resources/Sounds/boom1.wav";
    SendEvent(E_SOUNDGENERATED, eventData);
}

void OrdinaryWeapon::SpawnBullet(bool first)
{
    Node* bulletNode = refNode_->GetScene()->CreateChild();
    bulletNode->SetWorldPosition(refNode_->GetWorldPosition());
    bulletNode->SetRotation(refNode_->GetWorldRotation());

    float xOffSet = 0.3f * (first ? 1 : -1);
    bulletNode->Translate(Vector3(xOffSet,-0.2,0));

    bulletNode->CreateComponent<LowLevelBullet>();
}

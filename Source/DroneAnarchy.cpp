
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


#include <ctime>

#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Engine/Application.h>

#include <Urho3D/Script/ScriptInstance.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/ValueAnimation.h>


#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Resource/Image.h>

#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Math/Color.h>
#include <Urho3D/Graphics/BillboardSet.h>
#include <Urho3D/Graphics/Camera.h>

#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/IO/FileSystem.h>

#include <Urho3D/Core/CoreEvents.h>

#include <Urho3D/UI/UI.h>
#include <Urho3D/Core/Variant.h>

#include "EventsAndDefs.h"
#include "InputController.h"
#include "DroneAnarchy.h"


#ifdef USE_SCRIPT_OBJECT
    #include <Urho3D/Script/Script.h>
    #include <Urho3D/Script/ScriptFile.h>
#else
    #include <Urho3D/Scene/LogicComponent.h>
    #include "GameObjects.h"
#endif




const int SCORE_ADDITION_RATE = 1;

const unsigned int MAX_DRONE_COUNT = 15;

const float EASY_PHASE = 0;
const float MODERATE_PHASE = 60;
const float CRITICAL_PHASE = 120;
const float EASY_PHASE_RATE = 5;
const float MODERATE_PHASE_RATE = 3;
const float CRITICAL_PHASE_RATE = 1;
const float SCENE_TO_UI_SCALE = 1.6f;
const float SPRITE_UPDATE_TIME = 0.04f;

const String OPTIONS_MESSAGE = "<SPACE> To Replay | <ESC> To Quit";


DroneAnarchy::DroneAnarchy(Urho3D::Context *context) : Application(context)
{
    myjoystick_ = new virtualController();
    spriteUpdateCounter_ = droneSpawnCounter_ = 0;
    playerScore_ = 0;
    onQuit_ = false;
    gameState_ = GS_OUTGAME;
    playerDestroyed_ = false;


#ifdef USE_SCRIPT_OBJECT
    context_->RegisterSubsystem(new Script(context_));
#else
    context_->RegisterFactory<PlayerObject>();
    context_->RegisterFactory<LowLevelDrone>();
    context_->RegisterFactory<LowLevelBullet>();
    context_->RegisterFactory<SimpleExplosion>();
#endif


}


void DroneAnarchy::Setup()
{
    srand(time(NULL) % 1000);

    engineParameters_["ResourcePaths"] = "CoreData;Data;DroneAnarchy";
    engineParameters_["WindowResizable"] = true;
    engineParameters_["FullScreen"] = false;

    FileSystem* filesystem = GetSubsystem<FileSystem>();
    String dirName = filesystem->GetCurrentDir() + "AppLog";

    if(!filesystem->DirExists(dirName))
    {
        filesystem->CreateDir(dirName);
    }

    engineParameters_["LogName"] = dirName + "/DroneAnarchy.log";
}

void DroneAnarchy::Start()
{
    SetRandomSeed(rand());

    SetWindowTitleAndIcon();

    CreateGameControllers();
    CreateDebugHud();
    CreateValueAnimation();
    CreateInterface();

    //This is to prevent the pause that occurs in loading a resource for the first time
    LoadBackgroundResources();

    CreateScene();

    CreateCameraAndLight();

    SubscribeToEvents();
    CreateAudioSystem();

    StartGame();

}

void DroneAnarchy::Stop()
{
    delete myjoystick_;
}

void DroneAnarchy::HandleKeyDown(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    using namespace Urho3D::KeyDown;
    int key = eventData["key"].GetInt();

    if(key == KEY_ESC)
        onQuit_ = true;
    else if(key == KEY_F2)
        GetSubsystem<DebugHud>()->ToggleAll();
    else if(gameState_ == GS_OUTGAME)
    {
        HandleKeyOnOutGame(key);
    }
    else
    {
        HandleKeyOnInGame(key);
    }

}


void DroneAnarchy::PauseGame()
{
    scene_->SetUpdateEnabled(!scene_->IsUpdateEnabled());

    if(scene_->IsUpdateEnabled())
    {
        statusText_->SetText(String::EMPTY);
        gameState_ = GS_INGAME;
    }
    else
    {
        statusText_->SetText("PAUSED");
        gameState_ = GS_PAUSED;
    }

    targetSprite_->SetVisible(scene_->IsUpdateEnabled());
}


void DroneAnarchy::HandleKeyOnOutGame(int key)
{
    if(key == KEY_SPACE)
    {
        StartGame();
    }
}


void DroneAnarchy::HandleKeyOnInGame(int key)
{
    if(key == KEY_P)
    {
        PauseGame();
    }
}


void DroneAnarchy::HandleMouseMove(StringHash eventType, VariantMap &eventData)
{
    if(gameState_ != GS_INGAME)
    {
        return;
    }

    using namespace MouseMove;

    int dx = eventData[P_DX].GetInt();
    int dy = eventData[P_DY].GetInt();

    RotatePlayer(dx, dy);
}

void DroneAnarchy::RotatePlayer(int dx, int dy)
{
    using namespace PlayerRotation;
    VariantMap eventData;
    eventData[P_DX] = dx;
    eventData[P_DY] = dy;
    SendEvent(E_PLAYERROTATION, eventData);

    radarScreenBase_->SetRotation(-playerNode_->GetWorldRotation().YawAngle());
}


void DroneAnarchy::HandleMouseClick(StringHash eventType, VariantMap &eventData)
{
    if(gameState_ != GS_INGAME)
    {
        return;
    }

    int mouseButton = eventData["Button"].GetInt();

    if(mouseButton == MOUSEB_LEFT)
    {
        Fire();
    }
}

void DroneAnarchy::HandleFixedUpdate(StringHash eventType, VariantMap &eventData)
{
    float timeStep = eventData["TimeStep"].GetFloat();
    float droneSpawnRate = 0.0;

    gamePhaseCounter_ += timeStep;
    if(gamePhaseCounter_ >= CRITICAL_PHASE)
    {
        droneSpawnRate = CRITICAL_PHASE_RATE;
        gamePhaseCounter_ = CRITICAL_PHASE;
    }
    else if(gamePhaseCounter_ >= MODERATE_PHASE)
    {
        droneSpawnRate = MODERATE_PHASE_RATE;
    }
    else
    {
        droneSpawnRate = EASY_PHASE_RATE;
    }

    droneSpawnCounter_ +=timeStep;
    if(droneSpawnCounter_ >= droneSpawnRate)
    {
        if(droneRootNode_->GetChildren().Size() < MAX_DRONE_COUNT)
        {
            SpawnDrone();
            UpdateDroneSprites();
            droneSpawnCounter_ = 0;
        }
    }


    spriteUpdateCounter_ += timeStep;

    if(spriteUpdateCounter_ >= SPRITE_UPDATE_TIME)
    {
        UpdateDroneSprites();
        spriteUpdateCounter_ = 0;
    }


}

void DroneAnarchy::HandleUpdate(StringHash eventType, VariantMap &eventData)
{

    if(onQuit_)
    {
        engine_->Exit();
    }
    else if(playerDestroyed_ && gameState_ == GS_INGAME)
    {
        InitiateGameOver();
    }
    else if(gameState_ == GS_INGAME)
    {
        JoystickUpdate(joydirection_);
    }
}

void DroneAnarchy::HandleDroneDestroyed(StringHash eventType, VariantMap &eventData)
{
    using namespace DroneDestroyed;

    playerScore_ += eventData[P_DRONEPOINT].GetInt();
    UpdateScoreDisplay();
}

void DroneAnarchy::HandlePlayerHit(StringHash eventType, VariantMap &eventData)
{
    using namespace PlayerHit;
    //Update Health
    float playerHealthFraction = eventData[P_CURRENTHEALTHFRACTION].GetFloat();

    int range = 512 - ( 512 * playerHealthFraction);
    healthFillSprite_->SetImageRect(IntRect(range, 0, 512 + range, 64));
    UpdateHealthTexture(playerHealthFraction);

    //Show Warning
    radarScreenBase_->SetAttributeAnimation("Color", valAnim_, WM_ONCE);
    PlaySoundFX(cameraNode_,"Resources/Sounds/boom5.ogg");

    if(playerHealthFraction == 0)
    {
        playerDestroyed_ = true;
    }
}

void DroneAnarchy::HandlePlayerDestroyed(StringHash eventType, VariantMap &eventData)
{
    using namespace PlayerDestroyed;

    Vector3 camPosition = eventData[P_CAMPOSITION].GetVector3();
    Quaternion camRotation = eventData[P_CAMROTATION].GetQuaternion();

    cameraNode_->SetWorldPosition(camPosition);
    cameraNode_->SetWorldRotation(camRotation);

    SetViewportCamera(cameraNode_);
    cameraNode_->GetChild("DirectionalLight")->SetEnabled(true);

    SetSoundListener(cameraNode_);
}

void DroneAnarchy::HandleSoundGenerated(StringHash eventType, VariantMap& eventData)
{
    using namespace SoundGenerated;

    Node* soundNode = static_cast<Node*>(eventData[P_SOUNDNODE].GetPtr());
    String soundName = eventData[P_SOUNDNAME].GetString();

    PlaySoundFX(soundNode, soundName);
}


void DroneAnarchy::HandleCountFinished(StringHash eventType, VariantMap &eventData)
{
    CreatePlayer();

    cameraNode_->GetChild("DirectionalLight")->SetEnabled(false);

    //The following two lines come into play when restarting the game
    healthFillSprite_->SetImageRect(IntRect(0, 0, 512, 64));
    UpdateHealthTexture(1);

    scene_->SetUpdateEnabled(true);
    gameState_ = GS_INGAME;

    targetSprite_->SetVisible(true);
    enemyCountText_->SetText("0");
    playerScoreText_->SetText("0");
}



void DroneAnarchy::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);
    scene_->SetUpdateEnabled(false);
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<PhysicsWorld>();

    droneRootNode_ = scene_->CreateChild("DroneRootNode");

    // Create a Zone component for ambient lighting & fog control
    Node* zoneNode = scene_->CreateChild("ZoneNode");
    Zone* zone = zoneNode->CreateComponent<Zone>();

    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(0.2f, 0.2f, 0.2f));
    zone->SetFogColor(Color(0.5f, 0.5f, 1.0f));
    zone->SetFogStart(5.0f);
    zone->SetFogEnd(300.f);


    //Create a plane
    Node* planeNode = scene_->CreateChild("PlaneNode");
    StaticModel* plane = planeNode->CreateComponent<StaticModel>();
    plane->SetModel(cache->GetResource<Model>("Resources/Models/floor.mdl"));
    plane->SetMaterial(cache->GetResource<Material>("Resources/Materials/floor.xml"));

    //Add physics Components to the plane
    RigidBody* planeRB = planeNode->CreateComponent<RigidBody>();
    planeRB->SetCollisionLayerAndMask(FLOOR_COLLISION_LAYER, DRONE_COLLISION_LAYER | BULLET_COLLISION_LAYER);

    CollisionShape* planeCS = planeNode->CreateComponent<CollisionShape>();
    planeCS->SetTriangleMesh(cache->GetResource<Model>("Resources/Models/floor.mdl"));

}


void DroneAnarchy::CreatePlayer()
{
    playerNode_ = scene_->CreateChild("PlayerNode");

    Node* cameraNode = playerNode_->CreateChild("CameraNode");
    cameraNode->CreateComponent<Camera>();
    cameraNode->Translate(Vector3(0,1.7,0));

#ifdef USE_SCRIPT_OBJECT
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    ScriptInstance* sInstance = playerNode_->CreateComponent<ScriptInstance>();
    sInstance->CreateObject(cache->GetResource<ScriptFile>("Resources/Scripts/GameObjects.as"),"PlayerObject");
#else
    playerNode_->CreateComponent<PlayerObject>();
#endif

    cameraNode->CreateComponent<SoundListener>();
    SetSoundListener(cameraNode);

    SetViewportCamera(cameraNode);

    playerDestroyed_ = false;

}

void DroneAnarchy::CreateCameraAndLight()
{
    cameraNode_ = scene_->CreateChild("CameraNode");
    cameraNode_->CreateComponent<Camera>();
    cameraNode_->Translate(Vector3(0,1.7,0));

    Node* lightNode = cameraNode_->CreateChild("DirectionalLight");
    lightNode->SetDirection( Vector3(0.6f, -1.0f, 0.8f));
    Light* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);

    cameraNode_->CreateComponent<SoundListener>();

    Renderer* renderer = GetSubsystem<Renderer>();
    Viewport* viewport_ = new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>());
    renderer->SetViewport(0,viewport_);
}


void DroneAnarchy::SetViewportCamera(Node *cameraNode)
{
    Renderer* renderer = GetSubsystem<Renderer>();
    Viewport* viewport = renderer->GetViewport(0);

    viewport->SetCamera(cameraNode->GetComponent<Camera>());
}


void DroneAnarchy::Fire()
{
    using namespace ActivateWeapon;

    SendEvent(E_ACTIVATEWEAPON);
}

void DroneAnarchy::StartGame()
{
    playerScoreMessageText_->SetText(String::EMPTY);
    optionsInfoText_->SetText(String::EMPTY);
    gamePhaseCounter_ = 0.0f;
    droneSpawnCounter_ = 0.0f;
    playerScore_ = 0;

    SetSoundListener(cameraNode_);

    PlayBackgroundMusic("Resources/Sounds/cyber_dance.ogg");
    StartCounterToGame();
}

void DroneAnarchy::StartCounterToGame()
{
    ValueAnimation* textAnimation = new ValueAnimation(context_);

    textAnimation->SetKeyFrame(0.0f, "5");
    textAnimation->SetKeyFrame(1.0f, "4");
    textAnimation->SetKeyFrame(2.0f, "3");
    textAnimation->SetKeyFrame(3.0f, "2");
    textAnimation->SetKeyFrame(4.0f, "1");
    textAnimation->SetKeyFrame(5.0f, "PLAY");
    textAnimation->SetKeyFrame(6.0f, "");

    //Trigger  CountFinished event at the end of the animation
    textAnimation->SetEventFrame(6.0f,E_COUNTFINISHED);

    statusText_->SetAttributeAnimation("Text", textAnimation,WM_ONCE);
}



void DroneAnarchy::InitiateGameOver()
{
    scene_->SetUpdateEnabled( false);
    gameState_ = GS_OUTGAME;

    CleanupScene();
    PlayBackgroundMusic("Resources/Sounds/Defeated.ogg");

    targetSprite_->SetVisible(false);
    statusText_->SetText("YOU FAILED");
    playerScoreMessageText_->SetText("Score : " + String(playerScore_));
    optionsInfoText_->SetText(OPTIONS_MESSAGE) ;

}



void DroneAnarchy::CleanupScene()
{
    //Remove the player Node
    playerNode_->Remove();

    Vector<SharedPtr<Node> > droneNodes = droneRootNode_->GetChildren();

    for(unsigned int i = 0; i < droneNodes.Size(); i++)
    {
        Node* droneNode = droneNodes[i];
        Sprite* droneSprite = static_cast<Sprite*>(droneNode->GetVar("Sprite").GetPtr());
        droneSprite->Remove();
    }

    droneRootNode_->RemoveAllChildren();

#ifdef USE_SCRIPT_OBJECT
    //Cleanup any bullet still remaining in the scene
    PODVector< Node * >  scriptedNodes;
    scene_->GetChildrenWithComponent<ScriptInstance>(scriptedNodes);

    for(int i = 0; i < scriptedNodes.Size(); i++)
    {
        scriptedNodes[i]->Remove();
    }
#else
    PODVector<Node*> bulletNodes;
    scene_->GetChildrenWithComponent<LowLevelBullet>(bulletNodes);

    for(int i=0; i<bulletNodes.Size();i++)
    {
        bulletNodes[i]->Remove();
    }

    PODVector<Node*> explosionNodes;
    scene_->GetChildrenWithComponent<SimpleExplosion>(explosionNodes);

    for(int i=0; i<explosionNodes.Size();i++)
    {
        explosionNodes[i]->Remove();
    }

#endif

    //Hide the enemy counter and player score texts
    enemyCountText_->SetText(String::EMPTY);
    playerScoreText_->SetText(String::EMPTY);

}



void DroneAnarchy::SpawnDrone()
{

    Node* droneNode = droneRootNode_->CreateChild();
    droneNode->SetScale(3.0f);

#ifdef USE_SCRIPT_OBJECT
    ScriptInstance* sInstance = droneNode->CreateComponent<ScriptInstance>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    sInstance->CreateObject(cache->GetResource<ScriptFile>("Resources/Scripts/GameObjects.as"),"LowLevelDrone");
#else
    droneNode->CreateComponent<LowLevelDrone>();
#endif

    float nodeYaw = Random(360);
    droneNode->SetRotation( Quaternion(0,nodeYaw, 0));
    droneNode->Translate(Vector3(0,7,40));
    droneNode->SetVar("Sprite",CreateDroneSprite());


}

Sprite* DroneAnarchy::CreateDroneSprite()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Sprite* droneSprite = radarScreenBase_->CreateChild<Sprite>();

    droneSprite->SetTexture(cache->GetResource<Texture2D>("Resources/Textures/drone_sprite.png"));
    droneSprite->SetSize(6,6);
    droneSprite->SetAlignment(HA_CENTER, VA_CENTER);
    droneSprite->SetHotSpot(3,3);
    droneSprite->SetBlendMode(BLEND_ALPHA);
    droneSprite->SetPriority(1);

    return droneSprite;
}

void DroneAnarchy::UpdateDroneSprites()
{
    Vector<SharedPtr<Node> > droneNodes = droneRootNode_->GetChildren();

    for(unsigned int i = 0; i < droneNodes.Size(); i++)
    {
        Node* droneNode = droneNodes[i];
        Sprite* droneSprite = static_cast<Sprite*>(droneNode->GetVar("Sprite").GetPtr());

        Vector3 relativePos = droneNode->GetWorldPosition() - playerNode_->GetWorldPosition();
        droneSprite->SetPosition(Vector2(relativePos.x_ , -(relativePos.z_)) * SCENE_TO_UI_SCALE);
    }

    enemyCountText_->SetText(String(droneNodes.Size()));
}

void DroneAnarchy::UpdateHealthTexture(float healthFraction)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    if(healthFraction > 0.5)
    {
        healthFillSprite_->SetTexture(cache->GetResource<Texture2D>( "Resources/Textures/health_bar_green.png"));
    }
    else if(healthFraction > 0.2)
    {
        healthFillSprite_->SetTexture(cache->GetResource<Texture2D>( "Resources/Textures/health_bar_yellow.png"));
    }
    else
    {
       healthFillSprite_->SetTexture(cache->GetResource<Texture2D>( "Resources/Textures/health_bar_red.png"));;
    }
}

void DroneAnarchy::UpdateScoreDisplay()
{
    playerScoreText_->SetText(String(playerScore_));
}


void DroneAnarchy::CreateInterface()
{
    CreateHUD();
    CreateEnemyCountUI();
    CreatePlayerScoreUI();
    CreateDisplayTexts();
}

void DroneAnarchy::CreateHUD()
{
    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Sprite* hudSprite = ui->GetRoot()->CreateChild<Sprite>();
    hudSprite->SetTexture(cache->GetResource<Texture2D>("Resources/Textures/hud.png"));
    hudSprite->SetAlignment(HA_CENTER, VA_BOTTOM);
    hudSprite->SetSize(512, 256);
    hudSprite->SetHotSpot(256, 256);
    hudSprite->SetBlendMode(BLEND_ALPHA);
    hudSprite->SetPriority(3);

    Sprite* hudSpriteBG = ui->GetRoot()->CreateChild<Sprite>();
    hudSpriteBG->SetTexture(cache->GetResource<Texture2D>("Resources/Textures/hud_bg.png"));
    hudSpriteBG->SetAlignment(HA_CENTER, VA_BOTTOM);
    hudSpriteBG->SetSize(512, 256);
    hudSpriteBG->SetHotSpot(256, 256);
    hudSpriteBG->SetOpacity(0.6f);
    hudSpriteBG->SetBlendMode(BLEND_ALPHA);
    hudSpriteBG->SetPriority(-3);


    Sprite* healthBaseSprite = ui->GetRoot()->CreateChild<Sprite>();
    healthBaseSprite->SetTexture(cache->GetResource<Texture2D>("Resources/Textures/health_bg.png"));
    healthBaseSprite->SetAlignment(HA_CENTER, VA_BOTTOM);
    healthBaseSprite->SetSize(512, 128);
    healthBaseSprite->SetHotSpot(256, 64);
    healthBaseSprite->SetOpacity(0.9f);
    healthBaseSprite->SetBlendMode(BLEND_ALPHA);
    healthBaseSprite->SetPriority(1);


    healthFillSprite_ = healthBaseSprite->CreateChild<Sprite>();
    healthFillSprite_->SetTexture(cache->GetResource<Texture2D>("Resources/Textures/health_bar_green.png"));
    healthFillSprite_->SetAlignment(HA_CENTER, VA_CENTER);
    healthFillSprite_->SetSize(256, 25);
    healthFillSprite_->SetHotSpot(128, 25);
    healthFillSprite_->SetOpacity(0.5f);
    healthFillSprite_->SetImageRect(IntRect(0,0,512,64));
    healthFillSprite_->SetBlendMode(BLEND_ALPHA);


    radarScreenBase_ = ui->GetRoot()->CreateChild<Sprite>();
    radarScreenBase_->SetTexture(cache->GetResource<Texture2D>("Resources/Textures/radar_screen_base_.png"));
    radarScreenBase_->SetAlignment(HA_CENTER, VA_BOTTOM);
    radarScreenBase_->SetSize(128, 128);
    radarScreenBase_->SetHotSpot(64, 64);
    radarScreenBase_->SetOpacity(0.9f);
    radarScreenBase_->SetPosition(Vector2(0, -99));
    radarScreenBase_->SetPriority(2);
    radarScreenBase_->SetColor(Color(0.0, 0.4, 0.3, 0.7));


    Sprite* scopeScreen = ui->GetRoot()->CreateChild<Sprite>();
    scopeScreen->SetTexture(cache->GetResource<Texture2D>("Resources/Textures/radar_screen.png"));
    scopeScreen->SetSize(128, 128);
    scopeScreen->SetAlignment(HA_CENTER, VA_BOTTOM);
    scopeScreen->SetHotSpot(64, 64);
    scopeScreen->SetPosition(Vector2(0, -99));
    scopeScreen->SetBlendMode(BLEND_ALPHA);
    scopeScreen->SetPriority(4);

    targetSprite_ = ui->GetRoot()->CreateChild<Sprite>();
    targetSprite_->SetTexture(cache->GetResource<Texture2D>("Resources/Textures/target.png"));
    targetSprite_->SetSize(70, 70);
    targetSprite_->SetAlignment(HA_CENTER, VA_CENTER);
    targetSprite_->SetHotSpot(35, 35);
    targetSprite_->SetBlendMode(BLEND_ALPHA);
    targetSprite_->SetOpacity(0.6f);
    targetSprite_->SetVisible(false);


}

void DroneAnarchy::CreateEnemyCountUI()
{
    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    enemyCountText_ = ui->GetRoot()->CreateChild<Text>();
    enemyCountText_->SetFont(cache->GetResource<Font>("Resources/Fonts/segment7standard.otf"),15);
    enemyCountText_->SetAlignment(HA_CENTER, VA_BOTTOM);

    enemyCountText_->SetColor(Color(0.7f, 0.0f, 0.0f));
    enemyCountText_->SetPosition(IntVector2(-140,-72));
    enemyCountText_->SetPriority(1);
}

void DroneAnarchy::CreatePlayerScoreUI()
{
    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    playerScoreText_ = ui->GetRoot()->CreateChild<Text>();
    playerScoreText_->SetFont(cache->GetResource<Font>("Resources/Fonts/segment7standard.otf"),15);
    playerScoreText_->SetAlignment(HA_CENTER, VA_BOTTOM);

    playerScoreText_->SetColor(Color(0.0f, 0.9f, 0.2f));
    playerScoreText_->SetPosition(IntVector2(140,-72));
    playerScoreText_->SetPriority(1);
}

void DroneAnarchy::CreateDisplayTexts()
{
    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    statusText_ = ui->GetRoot()->CreateChild<Text>();
    statusText_->SetFont(cache->GetResource<Font>("Resources/Fonts/gtw.ttf"),70);
    statusText_->SetAlignment(HA_CENTER, VA_TOP);
    statusText_->SetColor(Color(0.2f, 0.8f, 1.0f));
    statusText_->SetPriority(1);
    statusText_->SetTextEffect(TE_SHADOW);

    playerScoreMessageText_ =  ui->GetRoot()->CreateChild<Text>();
    playerScoreMessageText_->SetFont(cache->GetResource<Font>("Resources/Fonts/gtw.ttf"),50);
    playerScoreMessageText_->SetAlignment(HA_CENTER, VA_TOP);
    playerScoreMessageText_->SetPosition(0, 150);
    playerScoreMessageText_->SetColor(Color(0.2f, 0.8f, 1.0f));
    playerScoreMessageText_->SetTextEffect(TE_SHADOW);


    optionsInfoText_ =  ui->GetRoot()->CreateChild<Text>();
    optionsInfoText_->SetFont(cache->GetResource<Font>("Resources/Fonts/gtw.ttf"),20);
    optionsInfoText_->SetAlignment(HA_CENTER, VA_CENTER);
    optionsInfoText_->SetPosition(0,50);
    optionsInfoText_->SetColor(Color(0.2f, 0.8f, 1.0f));
    optionsInfoText_->SetTextEffect(TE_SHADOW);
}



void DroneAnarchy::CreateDebugHud()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");

    if(file == NULL)
        return;

    DebugHud* debugHud = engine_->CreateDebugHud();
    debugHud->SetDefaultStyle(file);

}

void DroneAnarchy::SetWindowTitleAndIcon()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Graphics* graphics = GetSubsystem<Graphics>();
    Image* icon = cache->GetResource<Image>("Resources/Textures/drone_anarchy_icon.png");
    graphics->SetWindowIcon(icon);
    graphics->SetWindowTitle("Drone Anarchy");
}

void DroneAnarchy::CreateValueAnimation()
{
    valAnim_ = new ValueAnimation(context_);
    valAnim_->SetKeyFrame(0.0f, Color(0.0, 0.4, 0.3, 0.7));
    valAnim_->SetKeyFrame(0.3, Color(0.3,0.0,0.0));
    valAnim_->SetKeyFrame(1.0f, Color(0.0, 0.4, 0.3, 0.7));
}

void DroneAnarchy::CreateAudioSystem()
{
     Audio* audio = GetSubsystem<Audio>();
     audio->SetMasterGain(SOUND_MASTER,0.75);
     audio->SetMasterGain(SOUND_MUSIC,0.13);
     audio->SetMasterGain(SOUND_EFFECT, 0.5);

     Node* backgroundMusicNode = scene_->CreateChild();
     backgroundMusicSource_ = backgroundMusicNode->CreateComponent<SoundSource>();
     backgroundMusicSource_->SetSoundType(SOUND_MUSIC);
}

void DroneAnarchy::SetSoundListener(Node *listenerNode)
{
    if(listenerNode->GetComponent<SoundListener>() == NULL)
    {
        return;
    }

    Audio* audio = GetSubsystem<Audio>();
    audio->SetListener(listenerNode->CreateComponent<SoundListener>());
}

void DroneAnarchy::PlaySoundFX(Node *soundNode, String soundName)
{
    // Create the sound channel
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SoundSource3D* source = soundNode->CreateComponent<SoundSource3D>();
    Sound* sound = cache->GetResource<Sound>( soundName);

    source->SetDistanceAttenuation(2, 50, 1);
    source->SetSoundType(SOUND_EFFECT);
    source->Play(sound);
    source->SetAutoRemove(true);
}

void DroneAnarchy::PlayBackgroundMusic(String musicName)
{
    Sound* musicFile = GetSubsystem<ResourceCache>()->GetResource<Sound>(musicName);

    if(musicFile == NULL)
        return;

    musicFile->SetLooped(true);
    backgroundMusicSource_->Play(musicFile);

}

void DroneAnarchy::LoadBackgroundResources()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    cache->BackgroundLoadResource<Model>("Resources/Models/drone_body.mdl");
    cache->BackgroundLoadResource<Model>("Resources/Models/drone_arm.mdl");
    cache->BackgroundLoadResource<Animation>("Resources/Models/open_arm.ani");
    cache->BackgroundLoadResource<Animation>("Resources/Models/close_arm.ani");

    cache->BackgroundLoadResource<Texture2D>("Resources/Textures/explosion.png");

    cache->BackgroundLoadResource<ParticleEffect>("Resources/Particles/bullet_particle.xml");
    cache->BackgroundLoadResource<ParticleEffect>("Resources/Particles/explosion.xml");

    cache->BackgroundLoadResource<Material>("Resources/Materials/drone_arm.xml");
    cache->BackgroundLoadResource<Material>("Resources/Materials/drone_body.xml");
    cache->BackgroundLoadResource<Material>("Resources/Materials/bullet_particle.xml");
    cache->BackgroundLoadResource<Material>("Resources/Materials/explosion.xml");

    cache->BackgroundLoadResource<Texture2D>("Resources/Textures/drone_sprite.png");
    cache->BackgroundLoadResource<Texture2D>("Resources/Textures/health_bar_green.png");
    cache->BackgroundLoadResource<Texture2D>("Resources/Textures/health_bar_red.png");
    cache->BackgroundLoadResource<Texture2D>("Resources/Textures/health_bar_yellow.png");

    cache->BackgroundLoadResource<Sound>("Resources/Sounds/boom1.wav");
    cache->BackgroundLoadResource<Sound>("Resources/Sounds/boom5.ogg");

}

void DroneAnarchy::SubscribeToEvents()
{
    SubscribeToEvent(E_KEYDOWN, HANDLER(DroneAnarchy, HandleKeyDown));
    SubscribeToEvent(E_MOUSEMOVE,HANDLER(DroneAnarchy,HandleMouseMove));
    SubscribeToEvent(E_MOUSEBUTTONDOWN, HANDLER(DroneAnarchy,HandleMouseClick));
    SubscribeToEvent(E_UPDATE, HANDLER(DroneAnarchy, HandleUpdate));
    SubscribeToEvent((Object*)scene_->GetComponent<PhysicsWorld>(),E_PHYSICSPRESTEP,HANDLER(DroneAnarchy,HandleFixedUpdate));

    SubscribeToEvent(E_DRONEDESTROYED, HANDLER(DroneAnarchy, HandleDroneDestroyed));
    SubscribeToEvent(E_PLAYERHIT, HANDLER(DroneAnarchy, HandlePlayerHit));
    SubscribeToEvent(E_PLAYERDESTROYED, HANDLER(DroneAnarchy, HandlePlayerDestroyed));
    SubscribeToEvent(E_COUNTFINISHED, HANDLER(DroneAnarchy, HandleCountFinished));
    SubscribeToEvent(E_SOUNDGENERATED, HANDLER(DroneAnarchy, HandleSoundGenerated));

}


// look for a config file to normalize the controller button functions.
// and set up the event handlers in case 1 joystick is connected
void DroneAnarchy::CreateGameControllers()
{
    if ( GetSubsystem<Input>()->GetNumJoysticks() > 0 )  // is there a game controller plugged in?
    {
        myjoystick_ = new virtualController();  // make a controller
        myjoystick_->load_user_settings( GetSubsystem<ResourceCache>() );
        SubscribeToEvent(E_JOYSTICKBUTTONDOWN, HANDLER(DroneAnarchy, HandleButtonDown));
        SubscribeToEvent(E_JOYSTICKBUTTONUP, HANDLER(DroneAnarchy, HandleButtonUp ));
        if ( myjoystick_->button(DA_HAT_UP) > -1 ) // xbox uses both dpad and hat, (and analoge)!
            SubscribeToEvent(E_JOYSTICKHATMOVE, HANDLER(DroneAnarchy, HandleHatMove) );
    }
}

// continuous action function. This is needed because unlike keyboard keys,
// the joystick buttons do not auto-repeat. That not being bad enough, if
// a linear function is used to move around, it wont be fast enough to get
// to whats behind you, so the longer you hold down a movement button, the
// faster you move, up unto a point. You also want fine control to dial in
// aiming, so the first couple of updates after a button is held tries to
// limit the number of steps moved, hopefully to improve the aiming.
void DroneAnarchy::JoystickUpdate ( int position )
{
    if (GetSubsystem<Input>()->GetNumJoysticks() == 0 || position == -1 || ( myjoystick_ == NULL  ) ) return;

    // reset the counter if the controller emits 0, or the button pressed changes
    if ( position == 0 || myjoystick_->updatevalue_ != position )
        myjoystick_->updatecounter_ = 0;

    int move = 1; // start out with 1 click of movement
    int dx = 0;
    int dy = 0;

    // we need a real easing function...

    // fine control starting out, step 0, 4, 7, 9 on are let thru.
    if ( myjoystick_->updatecounter_ == 1 ) move = 0;
    if ( myjoystick_->updatecounter_ == 2 ) move = 0;
    if ( myjoystick_->updatecounter_ == 3 ) move = 0;
    if ( myjoystick_->updatecounter_ == 5 ) move = 0;
    if ( myjoystick_->updatecounter_ == 6 ) move = 0;
    if ( myjoystick_->updatecounter_ == 8 ) move = 0;

    // values to speed up, if the same button is held
    if ( myjoystick_->updatecounter_ > 20 ) move = 2;
    if ( myjoystick_->updatecounter_ > 80 ) move = 3;
    if ( myjoystick_->updatecounter_ > 170 ) move = 5;
    if ( myjoystick_->updatecounter_ > 223 ) move = 7;
    if ( myjoystick_->updatecounter_ > 666 ) move = 10;
    if ( myjoystick_->updatecounter_ > 999 ) move = 12;

    // find out where to go for direction buttons
    if ( position == myjoystick_->button(CONTROLLER_BUTTON_DPAD_UP))
    {
        dx= 0;
        dy =-move;
    }
    else if ( position == myjoystick_->button(CONTROLLER_BUTTON_DPAD_LEFT) )
    {
        dx= -move;
        dy =0;
    }
    else if ( position == myjoystick_->button(CONTROLLER_BUTTON_DPAD_DOWN) )
    {
        dx= 0;
        dy =move;
    }
    else if ( position == myjoystick_->button(CONTROLLER_BUTTON_DPAD_RIGHT) )
    {
        dx= move;
        dy =0;
    }
    // or hat direction
    else if ( position == myjoystick_->button(DA_HAT_UP))
    {
        dx= 0;
        dy =-move;
    }
    else if ( position == myjoystick_->button(DA_HAT_LEFT) )
    {
        dx= -move;
        dy =0;
    }
    else if ( position == myjoystick_->button(DA_HAT_DOWN) )
    {
        dx= 0;
        dy =move;
    }
    else if ( position == myjoystick_->button(DA_HAT_RIGHT) )
    {
        dx= move;
        dy =0;
    }

    if ( move > 0 )  // adjust the camera and hud settings
    {
        RotatePlayer(dx,dy);
    }

    // keep track of pseudo easing values
    myjoystick_->updatevalue_ = position;
    myjoystick_->updatecounter_ ++;
}

// Some types of joysticks handle the hat, which is the direction pad,
// as a separate unit from the buttons. Why, I dont know. If the hat is
// used then the controller will not emit DPAD button presses. The hat
// presses will be handled separately because the numbers it produces
// conflict with button numbers.
void DroneAnarchy::HandleHatMove(StringHash eventType, VariantMap& eventData)
{
    if(gameState_ != GS_INGAME) // if we are not playing, dont get new positions
    {
        return;
    }

    joydirection_ = eventData["Position"].GetInt();

    // for hats that can do both x + y directions, make it do x direction instead, otherwise
    // the continuous action studders.
    if ( joydirection_ == 9 || joydirection_ == 12 ) joydirection_ = myjoystick_->button(DA_HAT_LEFT);
    else if ( joydirection_ == 3 || joydirection_ == 6) joydirection_ = myjoystick_->button(DA_HAT_RIGHT);

}

// Get the joystick button presses. For controllers without the hat, the direction
// values will go thru here, as does fire control, and pause, exit.
void DroneAnarchy::HandleButtonDown(StringHash eventType, VariantMap& eventData)
{
    int jsButton = eventData["Button"].GetInt();

    if ( myjoystick_ == NULL ) return;

    if ( jsButton == myjoystick_->button(CONTROLLER_BUTTON_DPAD_UP)
            || jsButton == myjoystick_->button(CONTROLLER_BUTTON_DPAD_LEFT)
            || jsButton == myjoystick_->button(CONTROLLER_BUTTON_DPAD_DOWN)
            || jsButton == myjoystick_->button(CONTROLLER_BUTTON_DPAD_RIGHT) )
    {
        joydirection_ = jsButton;  // feed the direction and bolt
        return;
    }

    // select/back exits always
    if( jsButton == myjoystick_->button(CONTROLLER_BUTTON_BACK) )
    {
        onQuit_ = true;
    }

    // start in game pauses, unpauses
    if( jsButton == myjoystick_->button(CONTROLLER_BUTTON_START) && (gameState_ == GS_INGAME || gameState_ == GS_PAUSED) )
    {
        PauseGame();
    }

    //  fire only in game
    if( jsButton == myjoystick_->button(CONTROLLER_BUTTON_X) && ( gameState_ == GS_INGAME))
    {
        Fire();
    }
}

// To make the continuous action work with the DPAD buttons, we have
// to detect a DPAD button up event and send a 0 for the direction,
// to enable a stop in direction.
void DroneAnarchy::HandleButtonUp(StringHash eventType, VariantMap& eventData)
{
    int jsButton = eventData["Button"].GetInt();

    if ( myjoystick_ == NULL ) return;

    // when one of these buttons goes up, set the direction to 0 so the
    // continuous control stops.
    if ( jsButton == myjoystick_->button(CONTROLLER_BUTTON_DPAD_UP)
            || jsButton == myjoystick_->button(CONTROLLER_BUTTON_DPAD_LEFT)
            || jsButton == myjoystick_->button(CONTROLLER_BUTTON_DPAD_DOWN)
            || jsButton == myjoystick_->button(CONTROLLER_BUTTON_DPAD_RIGHT) )
    {
        joydirection_ = 0;  // stop the spinning
    }
}


DEFINE_APPLICATION_MAIN(DroneAnarchy)


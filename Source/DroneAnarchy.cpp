
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

#include <Urho3D/AngelScript/ScriptInstance.h>
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
#include <Urho3D/Graphics/RenderPath.h>

#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Scene/ValueAnimation.h>

#include <Urho3D/Core/CoreEvents.h>

#include <Urho3D/UI/UI.h>
#include <Urho3D/Core/Variant.h>

#include "EventsAndDefs.h"
#include "InputController.h"
#include "DroneAnarchy.h"

#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/AngelScript/ScriptFile.h>




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

    context_->RegisterSubsystem(new Script(context_));



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

    CreateDebugHud();
    LoadDisplayInterface();

    //This is to prevent the pause that occurs in loading a resource for the first time
    LoadBackgroundResources();

    //Load Attribute animation files from disk
    LoadAttributeAnimations();

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

void DroneAnarchy::HandlePlayerHealthUpdate(StringHash eventType, VariantMap &eventData)
{
    using namespace PlayerHealthUpdate;
    //Update Health
    float playerHealthFraction = eventData[P_CURRENTHEALTHFRACTION].GetFloat();

    int range = 512 - ( 512 * playerHealthFraction);
    healthFillSprite_->SetImageRect(IntRect(range, 0, 512 + range, 64));
    UpdateHealthTexture(playerHealthFraction);
}

void DroneAnarchy::HandlePlayerHit(StringHash eventType, VariantMap& eventData)
{
    //Show Warning
    radarScreenBase_->SetAttributeAnimation("Color", damageAnimation_, WM_ONCE);
    PlaySoundFX(cameraNode_,"Resources/Sounds/boom5.ogg");
}

void DroneAnarchy::HandlePlayerDestroyed(StringHash eventType, VariantMap &eventData)
{
    using namespace PlayerDestroyed;

    playerDestroyed_ = true;

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


void DroneAnarchy::HandleCountFinished(StringHash eventType, VariantMap& eventData)
{
    CreatePlayer();

    cameraNode_->GetChild("DirectionalLight")->SetEnabled(false);

    scene_->SetUpdateEnabled(true);
    gameState_ = GS_INGAME;

    targetSprite_->SetVisible(true);
    enemyCountText_->SetText("0");
    playerScoreText_->SetText("0");

    RenderPath* rPath = GetSubsystem<Renderer>()->GetViewport(0)->GetRenderPath();
    rPath->SetEnabled("Blur",false);
}



void DroneAnarchy::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Resources/Objects/Scene.xml");

    scene_ = new Scene(context_);
    scene_->LoadXML(file->GetRoot());
    scene_->SetUpdateEnabled(false);

    droneRootNode_ = scene_->CreateChild("DroneRootNode");

}


void DroneAnarchy::CreatePlayer()
{
    playerNode_ = scene_->CreateChild("PlayerNode");

    Node* cameraNode = playerNode_->CreateChild("CameraNode");
    cameraNode->CreateComponent<Camera>();
    cameraNode->Translate(Vector3(0,1.7,0));

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    ScriptInstance* sInstance = playerNode_->CreateComponent<ScriptInstance>();
    sInstance->CreateObject(cache->GetResource<ScriptFile>("Resources/Scripts/GameObjects.as"),"PlayerObject");


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
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    SharedPtr<Viewport> viewPort(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));

    renderer->SetViewport(0, viewPort);

    SharedPtr<RenderPath> rPath (viewPort->GetRenderPath());
    rPath->Append(cache->GetResource<XMLFile>("PostProcess/Blur.xml"));
    rPath->SetEnabled("Blur",true);
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



void DroneAnarchy::LoadAttributeAnimations()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    textAnimation_ = cache->GetResource<ValueAnimation>("Resources/AttributeAnimations/GameStartCounterAnimation.xml");
    damageAnimation_ = cache->GetResource<ValueAnimation>("Resources/AttributeAnimations/DamageWarningAnimation.xml");
}


void DroneAnarchy::StartCounterToGame()
{
    statusText_->SetAttributeAnimation("Text", textAnimation_,WM_ONCE);
}



void DroneAnarchy::InitiateGameOver()
{
    scene_->SetUpdateEnabled( false);
    gameState_ = GS_OUTGAME;

    CleanupScene();

    RenderPath* rPath = GetSubsystem<Renderer>()->GetViewport(0)->GetRenderPath();
    rPath->SetEnabled("Blur",true);

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

    //Cleanup any bullet still remaining in the scene
    PODVector< Node * >  scriptedNodes;
    scene_->GetChildrenWithComponent<ScriptInstance>(scriptedNodes);

    for(int i = 0; i < scriptedNodes.Size(); i++)
    {
        scriptedNodes[i]->Remove();
    }

    //Hide the enemy counter and player score texts
    enemyCountText_->SetText(String::EMPTY);
    playerScoreText_->SetText(String::EMPTY);

}



void DroneAnarchy::SpawnDrone()
{

    Node* droneNode = droneRootNode_->CreateChild();

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Resources/Objects/LowLevelDrone.xml");
    droneNode->LoadXML(file->GetRoot());


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


void DroneAnarchy::LoadDisplayInterface()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UIElement* displayRoot = GetSubsystem<UI>()->GetRoot();

    XMLFile* file = cache->GetResource<XMLFile>("Resources/UI/ScreenDisplay.xml");
    displayRoot->LoadXML(file->GetRoot());

    //Load the various UI Elements
    healthFillSprite_ = dynamic_cast<Sprite*>(displayRoot->GetChild(String("HealthFill"), true)) ;
    radarScreenBase_ = dynamic_cast<Sprite*>(displayRoot->GetChild(String("RadarScreenBase")));

    targetSprite_ = dynamic_cast<Sprite*>(displayRoot->GetChild(String("Target")));

    enemyCountText_ = dynamic_cast<Text*>(displayRoot->GetChild(String("EnemyCounter")));
    playerScoreText_ = dynamic_cast<Text*>(displayRoot->GetChild(String("PlayerScore")));


    statusText_ = dynamic_cast<Text*>(displayRoot->GetChild(String("StatusText")));
    playerScoreMessageText_ = dynamic_cast<Text*>(displayRoot->GetChild(String("ScoreMessage")));
    optionsInfoText_ = dynamic_cast<Text*>(displayRoot->GetChild(String("OptionInfo")));

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

    source->SetDistanceAttenuation(2, 120, 0.1);
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
    SubscribeToEvent(E_PLAYERHEALTHUPDATE, HANDLER(DroneAnarchy, HandlePlayerHealthUpdate));
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


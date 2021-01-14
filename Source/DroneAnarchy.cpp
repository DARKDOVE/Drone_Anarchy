//
// Copyright (c) 2014 - 2021 Drone Anarchy.
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
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Engine/Application.h>

#include <Urho3D/AngelScript/ScriptInstance.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Math/Color.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>

#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Resource/Image.h>

#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Audio/AudioEvents.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/AngelScript/ScriptFile.h>

#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/Sound.h>

#include "LevelManager.h"
#include "EventsAndDefs.h"
#include "DroneAnarchy.h"


#ifdef __EMSCRIPTEN__

#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

static DroneAnarchy *webInstance;

#endif

DroneAnarchy::DroneAnarchy(Urho3D::Context *context) : Application(context), useMouseMode_(MM_ABSOLUTE)
, showingIntroScene_(true)
, hasPointerLock_(false)
{

    context_->RegisterSubsystem(new Script(context_));
    context_->RegisterFactory<LevelManager>();

#ifdef __EMSCRIPTEN__
    webInstance = this;
#endif

}


void DroneAnarchy::Setup()
{
    //seed the random number function
    srand(time(NULL) % 1000);

    engineParameters_[EP_RESOURCE_PREFIX_PATHS] = GetSubsystem<FileSystem>()->GetCurrentDir() + "Assets";

    engineParameters_[EP_RESOURCE_PATHS] = "CoreData;GameData;GameLogic";


#ifdef __EMSCRIPTEN__
    engineParameters_[EP_FULL_SCREEN] = false;
    engineParameters_[EP_HEADLESS] = false;
#else
    engineParameters_[EP_FULL_SCREEN] = true;
#endif

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

#ifndef __EMSCRIPTEN__
    hasPointerLock_ = true;
#endif

    auto* cache = GetSubsystem<ResourceCache>();
    cache->BackgroundLoadResource<Sound>("Sounds/through_space_(modified).ogg");
    cache->BackgroundLoadResource<Sound>("Sounds/cyber_dance.ogg");

    SetRandomSeed(rand());

    SetupAudioGain();
    
    SetWindowTitleAndIcon();

    CreateDebugHud();

    CreateIntroScene();

    CreateLevel();

    SubscribeToEvents();

    // Set the mouse mode to use in the sample
    InitMouseMode(MM_RELATIVE);

}

void DroneAnarchy::Stop()
{
}

void DroneAnarchy::SetupAudioGain()
{
    auto *audio = GetSubsystem<Audio>();

    audio->SetMasterGain(SOUND_MASTER, 0.95);
    audio->SetMasterGain(SOUND_MUSIC, 0.23);
    audio->SetMasterGain(SOUND_EFFECT, 0.5);

}

void DroneAnarchy::HandleKeyDown(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    //no event handling if no pointer lock
    if( !hasPointerLock_ )
    {
        return;
    }

    using namespace Urho3D::KeyDown;
    int key = eventData["key"].GetInt();

    if(key == KEY_F2)
    {
        GetSubsystem<DebugHud>()->ToggleAll();
    }
    else if( showingIntroScene_ && KEY_ESCAPE)
    {
        engine_->Exit();
    }
    else
    {
        eventData["ID"] = EVT_KEYDOWN;
        levelManager_->HandleLevelEvent(eventData);
    }

}

void DroneAnarchy::HandleMouseMove(StringHash eventType, VariantMap &eventData)
{
    //no event handling if no pointer lock
    if( !hasPointerLock_ )
    {
        return;
    }

    eventData["ID"] = EVT_MOUSEMOVE;
    levelManager_->HandleLevelEvent(eventData);
}


void DroneAnarchy::HandleMouseClick(StringHash eventType, VariantMap &eventData)
{
    //no event handling if no pointer lock
    if( !hasPointerLock_ )
    {
        return;
    }

    //if showing intro scene for non web platfomr then that means we are just starting
#ifndef __EMSCRIPTEN__

    if( showingIntroScene_ )
    {
        showingIntroScene_ = false;
        introScene_->GetComponent<SoundSource>()->Stop();
        introScene_->SetUpdateEnabled( false );
        introUI_->SetVisible(false);
        levelManager_->StartOrResumeLevel();
        return;
    }

#endif
    
}


void DroneAnarchy::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
    

    //if intro scene is shoing then handle update for intro scene scenarios
    if(showingIntroScene_){
        HandleIntroSceneUpdate( eventData );
        return;

    }

    //no event handling if no pointer lock
    if( !hasPointerLock_ )
    {
        return;
    }
    

    int statusId = GetGlobalVar("STATUS_ID").GetInt();

    //Quickly reset the status variable to normal
    SetGlobalVar("STATUS_ID",LSTATUS_NORMAL);

    if(statusId == LSTATUS_QUIT)
    {
        engine_->Exit();
    }
    else
    {
        eventData["ID"] = EVT_UPDATE;
        levelManager_->HandleLevelEvent(eventData);

    }
}


void DroneAnarchy::HandleIntroSceneUpdate(VariantMap &eventData)
{
    using namespace Update;

    float timeStep = eventData[P_TIMESTEP].GetFloat();
    introDroneNode_->Yaw(timeStep * 200);
}

void DroneAnarchy::HandleSoundFinished(StringHash eventType, VariantMap &eventData)
{
    eventData["ID"] = EVT_SOUNDFINISH;
    levelManager_->HandleLevelEvent(eventData);
}

void DroneAnarchy::HandleJoystickButtonDown(StringHash eventType, VariantMap &eventData)
{
    //no event handling if no pointer lock
    if( !hasPointerLock_ )
    {
        return;
    }
    
    eventData["ID"] = EVT_JOYSTICK_BUTTONDOWN;
    levelManager_->HandleLevelEvent(eventData);
}

void DroneAnarchy::HandleJoystickButtonUp(StringHash eventType, VariantMap &eventData)
{
    //no event handling if no pointer lock
    if( !hasPointerLock_ )
    {
        return;
    }
    
    eventData["ID"] = EVT_JOYSTICK_BUTTONUP;
    levelManager_->HandleLevelEvent(eventData);
}

void DroneAnarchy::HandleHatMove(StringHash eventType, VariantMap &eventData)
{
    //no event handling if no pointer lock
    if( !hasPointerLock_ )
    {
        return;
    }
    
    eventData["ID"] = EVT_JOYSTICK_HATMOVE;
    levelManager_->HandleLevelEvent(eventData);
}


void DroneAnarchy::CreateLevel()
{

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Objects/Scene.xml");

    levelScene_ = new Scene(context_);
    levelScene_->LoadXML(file->GetRoot());

    levelManager_ = levelScene_->CreateComponent<LevelManager>();
    levelManager_->InitialiseAndActivate();

}

void DroneAnarchy::CreateIntroScene()
{

    CreateIntroUI();

    auto *cache = GetSubsystem<ResourceCache>();

    introScene_ = new Scene(context_);

    introScene_->CreateComponent<Octree>();
    // Create a Zone component for ambient lighting & fog control
    Node* zoneNode = introScene_->CreateChild("Zone");
    Zone* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox( BoundingBox(-1000.0f, 1000.0f) );
    zone->SetAmbientColor( Color(0.2f, 0.2f, 0.2f) );
    zone->SetFogColor( Color(0.5f, 0.5f, 1.0f) );
    zone->SetFogStart( 5.0f );
    zone->SetFogEnd( 300.0f );
    
    //Create a plane
    Node* planeNode = introScene_->CreateChild("Plane");
    StaticModel* plane = planeNode->CreateComponent<StaticModel>();
    planeNode->Translate(Vector3(-2, -2.5, 1.5));
    planeNode->Pitch(-90);
    planeNode->Yaw(36);
    

    plane->SetModel( cache->GetResource<Model>("Models/floor.mdl") );
    plane->SetMaterial( cache->GetResource<Material>( "Materials/intro_wall.xml") );

    //create model drone
    introDroneNode_ = introScene_->CreateChild("DroneNode");
    introDroneNode_->SetScale(5.0f);
    introDroneNode_->Translate(Vector3(0,0.5,-5));

    auto* droneBody = introDroneNode_->CreateComponent<AnimatedModel>();
    droneBody->SetModel(cache->GetResource<Model>( "Models/drone_body.mdl") );
    droneBody->SetMaterial( cache->GetResource<Material>("Materials/drone_body.xml"));
    droneBody->SetCastShadows(true);
    
    auto* droneArm = introDroneNode_->CreateComponent<AnimatedModel>();
    droneArm->SetModel( cache->GetResource<Model>( "Models/drone_arm.mdl") );
    droneArm->SetMaterial( cache->GetResource<Material>( "Materials/drone_arm.xml") );
    droneArm->SetCastShadows(true);
    
    auto* animController = introDroneNode_->CreateComponent<AnimationController>();
    animController->PlayExclusive("Models/open_arm.ani", 0, false);


    Node* lightNode = introScene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(1, -3, 2));
    Light* light = lightNode->CreateComponent<Light>();
    light->SetLightType( LIGHT_DIRECTIONAL );
    light->SetCastShadows(true);
    
    Node* camNode = introScene_->CreateChild("Camera Node");
    introCamera_ = camNode->CreateComponent<Camera>();
    camNode->Translate(Vector3(0,0,-10));

    //setup viewport for intro display
    auto* renderer = GetSubsystem<Renderer>();
    
    introViewport_ = new Viewport(context_, introScene_, introCamera_);
    renderer->SetViewport(0, introViewport_);

    auto *soundSource = introScene_->CreateComponent<SoundSource>();
    soundSource->SetSoundType(SOUND_MUSIC);

    auto* music = cache->GetResource<Sound>("Sounds/through_space_(modified).ogg");
    // Set the song to loop
    music->SetLooped(true);

    soundSource->Play(music);


}

void DroneAnarchy::CreateIntroUI()
{

    auto *cache = GetSubsystem<ResourceCache>();

    //first create the UI
    auto *ui = GetSubsystem<UI>();
    introUI_ = ui->GetRoot()->CreateChild<UIElement>();
    UpdateIntroUIDimension();

    auto *titleText = introUI_->CreateChild<Text>();
    titleText->SetText("Drone Anarchy");
    titleText->SetFont( cache->GetResource<Font>("Fonts/pdark.ttf") );
    titleText->SetAlignment( HA_CENTER, VA_TOP );
    titleText->SetFontSize( 40 );
    titleText->SetTextEffect(TextEffect::TE_SHADOW);
    titleText->SetColor( Color(0.239, 0.913, 1) );
    titleText->SetPosition( 0, 40);

    auto *instructionText = introUI_->CreateChild<Text>();
    String instructions = "> Move mouse to rotate\n> Click to Shoot\n> KEY P to toggle Pause\n";

    if(GetPlatform() != "Web")
    {
        instructions += "> ESC To Quit\n";
    }

    instructions += "\n\n***CLICK TO ENTER GAME***";

    instructionText->SetText(instructions);

    instructionText->SetAlignment( HA_CENTER, VA_CENTER );
    instructionText->SetFont( cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"));
    instructionText->SetFontSize(20);
    instructionText->SetTextEffect(TextEffect::TE_SHADOW);
    instructionText->SetColor( Color(0.239, 0.913, 1) );
    instructionText->SetPosition( 0, 150);


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
    Image* icon = cache->GetResource<Image>("Textures/drone_anarchy_icon.png");
    graphics->SetWindowIcon(icon);
    graphics->SetWindowTitle("Drone Anarchy");
}


void DroneAnarchy::SubscribeToEvents()
{
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(DroneAnarchy, HandleKeyDown));
    SubscribeToEvent(E_MOUSEMOVE,URHO3D_HANDLER(DroneAnarchy,HandleMouseMove));
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(DroneAnarchy,HandleMouseClick));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(DroneAnarchy, HandleUpdate));
    SubscribeToEvent(E_SOUNDFINISHED, URHO3D_HANDLER(DroneAnarchy, HandleSoundFinished));

    Input* input = GetSubsystem<Input>();
    if(input->GetNumJoysticks() >  0)
    {
        SubscribeToEvent(E_JOYSTICKBUTTONDOWN, URHO3D_HANDLER(DroneAnarchy,HandleJoystickButtonDown));
        SubscribeToEvent(E_JOYSTICKBUTTONUP, URHO3D_HANDLER(DroneAnarchy, HandleJoystickButtonUp));
        SubscribeToEvent(E_JOYSTICKHATMOVE, URHO3D_HANDLER(DroneAnarchy, HandleHatMove));
    }

}

void DroneAnarchy::InitMouseMode(MouseMode mode)
{
    useMouseMode_ = mode;

    Input* input = GetSubsystem<Input>();

    if (GetPlatform() != "Web")
    {
        if (useMouseMode_ == MM_FREE)
            input->SetMouseVisible(true);

        if (useMouseMode_ != MM_ABSOLUTE)
        {
            input->SetMouseMode(useMouseMode_);
        }
    }
    else
    {
        input->SetMouseVisible(true);
        SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(DroneAnarchy, HandleMouseModeRequest));
        SubscribeToEvent(E_MOUSEMODECHANGED, URHO3D_HANDLER(DroneAnarchy, HandleMouseModeChange));
    }
}

void DroneAnarchy::UpdateIntroUIDimension()
{

    IntVector2 rect = GetSubsystem<Graphics>()->GetSize();

    if( !introUI_ )
    {
        return;
    }

    introUI_->SetSize( rect );
}


// If the user clicks the canvas, attempt to switch to relative mouse mode on web platform
void DroneAnarchy::HandleMouseModeRequest(StringHash /*eventType*/, VariantMap& eventData)
{
    Input* input = GetSubsystem<Input>();
    if (useMouseMode_ == MM_ABSOLUTE)
        input->SetMouseVisible(false);
    else if (useMouseMode_ == MM_FREE)
        input->SetMouseVisible(true);
    input->SetMouseMode(useMouseMode_);
}

void DroneAnarchy::HandleMouseModeChange(StringHash /*eventType*/, VariantMap& eventData)
{
    Input* input = GetSubsystem<Input>();
    bool mouseLocked = eventData[MouseModeChanged::P_MOUSELOCKED].GetBool();
    input->SetMouseVisible(!mouseLocked);
}


#ifdef __EMSCRIPTEN__

void DroneAnarchy::HandleWebResized()
{
    UpdateIntroUIDimension();

    //if intro scene is showing then return without doing anything
    if(showingIntroScene_)
    {
        return;
    }

    IntVector2 rect = GetSubsystem<Graphics>()->GetSize();


    VariantMap eventData;
    eventData["ID"] = EVT_WEB_WINDOW_RESIZED;
    eventData["CurrentWebWindowSize"] = rect;
    
    levelManager_->HandleLevelEvent(eventData);
}


void DroneAnarchy::PonterLockAcquired()
{
    hasPointerLock_ = true;
    showingIntroScene_ = false;
    introScene_->GetComponent<SoundSource>()->Stop();
    introScene_->SetUpdateEnabled( false );
    introUI_->SetVisible(false);
    levelManager_->StartOrResumeLevel();
}

void DroneAnarchy::PointerLockLost()
{
    hasPointerLock_ = false;
    showingIntroScene_ = true;

    auto *cache = GetSubsystem<ResourceCache>();

    auto* music = cache->GetResource<Sound>("Sounds/through_space_(modified).ogg");
    // Set the song to loop
    music->SetLooped(true);

    introScene_->GetComponent<SoundSource>()->Play(music);
    introScene_->SetUpdateEnabled( true );
    introUI_->SetVisible(true);
    auto* renderer = GetSubsystem<Renderer>();
    renderer->SetViewport(0, introViewport_);
    levelManager_->Deactivate();
}


static void SetPonterLockAcquired()
{
    if (webInstance)
    {
        webInstance->PonterLockAcquired();
    }
}

static void SetPointerLockLost()
{
    if (webInstance)
    {
        webInstance->PointerLockLost();
    }
}


static void WebWindowResized()
{
    if (webInstance)
    {
        webInstance->HandleWebResized();
    }
}

using namespace emscripten;

EMSCRIPTEN_BINDINGS(WebPlayer) {
    function("SetPonterLockAcquired", &SetPonterLockAcquired);
    function("SetPointerLockLost", &SetPointerLockLost);
    function("WebWindowResized", &WebWindowResized);
}


#endif


URHO3D_DEFINE_APPLICATION_MAIN(DroneAnarchy)


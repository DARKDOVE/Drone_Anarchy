
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
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>

#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Resource/Image.h>

#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Audio/AudioEvents.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/AngelScript/ScriptFile.h>

#include "LevelManager.h"
#include "EventsAndDefs.h"
#include "DroneAnarchy.h"

DroneAnarchy::DroneAnarchy(Urho3D::Context *context) : Application(context)
{
    onQuit_ = false;

    context_->RegisterSubsystem(new Script(context_));
    context->RegisterFactory<LevelManager>();

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

    CreateDebugHud();

    CreateLevel();

    SubscribeToEvents();


}

void DroneAnarchy::Stop()
{
}

void DroneAnarchy::HandleKeyDown(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    using namespace Urho3D::KeyDown;
    int key = eventData["key"].GetInt();

    if(key == KEY_ESC)
        onQuit_ = true;
    else if(key == KEY_F2)
        GetSubsystem<DebugHud>()->ToggleAll();
    else
    {
        eventData["ID"] = EVT_KEYDOWN;
        levelManager_->HandleLevelEvent(eventData);
    }

}

void DroneAnarchy::HandleMouseMove(StringHash eventType, VariantMap &eventData)
{
    eventData["ID"] = EVT_MOUSEMOVE;
    levelManager_->HandleLevelEvent(eventData);
}


void DroneAnarchy::HandleMouseClick(StringHash eventType, VariantMap &eventData)
{
    eventData["ID"] = EVT_MOUSECLICK;
    levelManager_->HandleLevelEvent(eventData);
}


void DroneAnarchy::HandleUpdate(StringHash eventType, VariantMap &eventData)
{

    if(onQuit_)
    {
        engine_->Exit();
    }
    else
    {
        eventData["ID"] = EVT_UPDATE;
        levelManager_->HandleLevelEvent(eventData);

    }
}

void DroneAnarchy::HandleSoundFinished(StringHash eventType, VariantMap &eventData)
{
    eventData["ID"] = EVT_SOUNDFINISH;
    levelManager_->HandleLevelEvent(eventData);
}

void DroneAnarchy::HandleJoystickButtonDown(StringHash eventType, VariantMap &eventData)
{
    eventData["ID"] = EVT_JOYSTICK_BUTTONDOWN;
    levelManager_->HandleLevelEvent(eventData);
}

void DroneAnarchy::HandleJoystickButtonUp(StringHash eventType, VariantMap &eventData)
{
    eventData["ID"] = EVT_JOYSTICK_BUTTONUP;
    levelManager_->HandleLevelEvent(eventData);
}

void DroneAnarchy::HandleHatMove(StringHash eventType, VariantMap &eventData)
{
    eventData["ID"] = EVT_JOYSTICK_HATMOVE;
    levelManager_->HandleLevelEvent(eventData);
}


void DroneAnarchy::CreateLevel()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Resources/Objects/Scene.xml");

    scene_ = new Scene(context_);
    scene_->LoadXML(file->GetRoot());

    levelManager_ = scene_->CreateComponent<LevelManager>();
    levelManager_->InitialiseAndActivate();


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


URHO3D_DEFINE_APPLICATION_MAIN(DroneAnarchy)


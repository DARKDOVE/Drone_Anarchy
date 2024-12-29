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

#include <Urho3D/Scene/Node.h>
#include <Urho3D/AngelScript/ScriptFile.h>
#include <Urho3D/Resource/ResourceCache.h>

#include "LevelManager.h"

LevelManager::LevelManager(Context *context): LogicComponent(context), hasScriptObject(false)
{

}

void LevelManager::Initialise()
{
    instance_ = GetNode()->CreateComponent<ScriptInstance>();

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    instance_->CreateObject(cache->GetResource<ScriptFile>("Scripts/LevelManager.as"),"LevelOneManager");
    instance_->Execute("void Initialise()");
    hasScriptObject = true;
}

void LevelManager::InitialiseAndActivate()
{
    Initialise();
    Activate();
}

void LevelManager::Activate()
{
    if(!hasScriptObject)
        return;

    instance_->Execute("void Activate()");
}

void LevelManager::Deactivate()
{
    if(!hasScriptObject)
        return;

    instance_->Execute("void Deactivate()");
}

void LevelManager::HandleLevelEvent(VariantMap &eventData)
{
    if(!hasScriptObject)
        return;

    VariantVector parameters;
    parameters.Push(eventData);
    instance_->Execute("void HandleLevelEvent(VariantMap& eventData)",parameters);
}

void LevelManager::StartOrResumeLevel()
{
    if(!hasScriptObject)
        return;

    instance_->Execute("void StartOrResumeLevel()");
}

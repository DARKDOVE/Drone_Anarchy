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
    instance_->CreateObject(cache->GetResource<ScriptFile>("Resources/Scripts/LevelManager.as"),"LevelOneManager");
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

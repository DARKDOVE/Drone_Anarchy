#ifndef LEVELMANAGER_H
#define LEVELMANAGER_H

#include <Urho3D/Urho3D.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/AngelScript/ScriptInstance.h>

using namespace Urho3D;

class LevelManager : public LogicComponent
{
    URHO3D_OBJECT(LevelManager, LogicComponent)


public:
        LevelManager(Context* context);
        void Initialise();
        void InitialiseAndActivate();
        void Activate();
        void Deactivate();
        void HandleLevelEvent(VariantMap& eventData);

private:
        bool hasScriptObject;
        WeakPtr<ScriptInstance> instance_;


};

#endif // LEVELMANAGER_H


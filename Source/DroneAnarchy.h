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



#ifndef __DRONEANARCHY_H_
#define __DRONEANARCHY_H_

using namespace Urho3D;

class DroneAnarchy : public Application
{

    URHO3D_OBJECT(DroneAnarchy,Application);


public:
    DroneAnarchy(Context* context);

    virtual void Setup();
    virtual void Start();
    virtual void Stop();


    void HandleKeyDown(StringHash eventType,VariantMap& eventData);
    void HandleMouseMove(StringHash eventType, VariantMap& eventData);
    void HandleMouseClick(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleIntroSceneUpdate( VariantMap& eventData);
    void HandleSoundFinished(StringHash eventType, VariantMap& eventData);
    void HandleJoystickButtonDown(StringHash eventType, VariantMap& eventData);
    void HandleJoystickButtonUp(StringHash eventType, VariantMap& eventData);
    void HandleHatMove(StringHash eventType, VariantMap& eventData);

    /// Handle request for mouse mode on web platform.
    void HandleMouseModeRequest(StringHash eventType, VariantMap& eventData);
    /// Handle request for mouse mode change on web platform.
    void HandleMouseModeChange(StringHash eventType, VariantMap& eventData);
    /// Initialize mouse mode on non-web platform.
    void InitMouseMode(MouseMode mode);

#ifdef __EMSCRIPTEN__
    void PonterLockAcquired();
    void PointerLockLost();
    void HandleWebResized();
#endif

private:

    void SubscribeToEvents();
    void SetWindowTitleAndIcon();
    void CreateLevel();
    void CreateIntroScene();
    void CreateIntroUI();
    void CreateDebugHud();
    void SetupAudioGain();
    void UpdateIntroUIDimension();

    
    bool hasPointerLock_;

    SharedPtr<Scene> levelScene_;
    SharedPtr<Scene> introScene_;
    SharedPtr<Viewport> introViewport_;
    SharedPtr<Camera> introCamera_;
    SharedPtr<Node> introDroneNode_;
    SharedPtr<UIElement> introUI_;

    WeakPtr<LevelManager> levelManager_;

    /// Mouse mode option to use in the sample.
    MouseMode useMouseMode_;
    bool showingIntroScene_;
};


#endif // #ifndef __DRONEANARCHY_H_

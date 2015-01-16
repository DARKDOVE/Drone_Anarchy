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



#ifndef __DRONEANARCHY_H_
#define __DRONEANARCHY_H_

#include <Application.h>
#include <Engine.h>
#include <InputEvents.h>
#include <Renderer.h>
#include <ResourceCache.h>
#include <Sprite.h>
#include <Texture2D.h>
#include <UI.h>
#include <Scene.h>
#include <Node.h>
#include <InputEvents.h>
#include <Text.h>
#include <ValueAnimation.h>
#include <SoundSource.h>

//Comment out the line below to build using LogicComponent
#define USE_SCRIPT_OBJECT


enum GameState
{
    GS_INGAME = 101,
    GS_OUTGAME,
    GS_PAUSED
};


using namespace Urho3D;

class DroneAnarchy : public Application
{

    OBJECT(DroneAnarchy)

public:
    DroneAnarchy(Context* context);

    virtual void Setup();
    virtual void Start();
    virtual void Stop();


    void HandleKeyDown(StringHash eventType,VariantMap& eventData);
    void HandleMouseMove(StringHash eventType, VariantMap& eventData);
    void HandleMouseClick(StringHash eventType, VariantMap& eventData);
    void HandleFixedUpdate(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    void HandleDroneDestroyed(StringHash eventType, VariantMap& eventData);
    void HandlePlayerHit(StringHash eventType, VariantMap& eventData);
    void HandleDroneHit(StringHash eventType, VariantMap& eventData);
    void HandleCountFinished(StringHash eventType, VariantMap& eventData);



private:


    void StartGame();
    void PauseGame();
    void StartCounterToGame();
    void InitiateGameOver();
    void CleanupScene();
    void SubscribeToEvents();
    void SetWindowTitleAndIcon();
    //This is to prevent the pause that occurs in loading a resource for the first time
    void LoadBackgroundResources();

    void CreateInterface();
    void CreateHUD();
    void CreateEnemyCountUI();
    void CreatePlayerScoreUI();
    void CreateDisplayTexts();

    void CreateScene();
    void CreateDebugHud();
    void CreatePlayer();
    void CreateCameraAndLight();
    void CreateValueAnimation();
    void CreateAudioSystem();
    Sprite* CreateDroneSprite();

    void Fire();
    void PlaySoundFX(Node* soundNode, String soundName);
    void PlayBackgroundMusic(String musicName);

    void SpawnDrone();
    void SpawnBullet(bool first);
    void SpawnExplosion(Vector3 position);

    void UpdateDroneSprites();
    void UpdateHealthTexture(float healthFraction);
    void UpdateScoreDisplay();

    void HandleKeyOnOutGame(int key);
    void HandleKeyOnInGame(int key);



    float spriteUpdateCounter_;
    float droneSpawnCounter_;
    float gamePhaseCounter_;

    int playerScore_;

    bool onQuit_;
    bool playerDestroyed_;

    GameState gameState_;

    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
    SharedPtr<Node> droneRootNode_;
    SharedPtr<Node> playerNode_;

    SharedPtr<Sprite> healthFillSprite_;
    SharedPtr<Sprite> radarScreenBase_;
    SharedPtr<Sprite> targetSprite_;

    SharedPtr<ValueAnimation> valAnim_;

    SharedPtr<Text> enemyCountText_;
    SharedPtr<Text> playerScoreText_;

    SharedPtr<Text> statusText_;
    SharedPtr<Text> playerScoreMessageText_;
    SharedPtr<Text> optionsInfoText_;
    SharedPtr<SoundSource> backgroundMusicSource_;

};


#endif // #ifndef __DRONEANARCHY_H_

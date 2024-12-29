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


//-----------------------------------------------PLAYER OBJECT--------------------------------------------------------------------------------


///Player Object

class PlayerObject : ScriptObject
{
	float maximumHealth_;
	float currentHealth_;
	float healthIncrement_;
	float healthIncrementTime_;
	float healthIncrementTimeCounter_;
	Weapon@ weapon_;
	
	PlayerObject()
	{
		maximumHealth_ = 100;
		currentHealth_ = 0;
		
		healthIncrement_ = 1.0f;
		healthIncrementTime_ = 10.0f;
		healthIncrementTimeCounter_ = 0;
	}
	
	void DelayedStart()
	{
		SubscribeToEvent("ActivateWeapon", "HandleActivateWeapon");
		SubscribeToEvent("PlayerRotation", "HandlePlayerRotation");
		Initialise();
		UpdateHealth(100);
	}
	
	void Initialise()
	{
		RigidBody@ playerBody  = node.CreateComponent("RigidBody");
		playerBody.SetCollisionLayerAndMask(PLAYER_COLLISION_LAYER, DRONE_COLLISION_LAYER);
		CollisionShape@ playerColShape = node.CreateComponent("CollisionShape");
		playerColShape.SetSphere(2);
		SetWeapon(OrdinaryWeapon(node.GetChild("CameraNode")));
		
		Node@ cameraNode = node.GetChild("CameraNode");
		
		Node@ lightNode = cameraNode.CreateChild("DirectionalLight");
		lightNode.direction = Vector3(0.6f, -1.0f, 0.8f);
		Light@ light = lightNode.CreateComponent("Light");
		light.lightType = LIGHT_DIRECTIONAL;
	}
	
	void HandleActivateWeapon()
	{
		weapon_.Fire();
	}
	
	void HandlePlayerRotation(StringHash eventType, VariantMap& eventData)
	{
		int dx = eventData["DX"].GetInt();
		int dy = eventData["DY"].GetInt();
		
		Node@ cameraNode = node.GetChild("CameraNode");
		
		float playerYaw = node.rotation.yaw + (dx * 0.25f);
		float camPitch = cameraNode.rotation.pitch + (dy * 0.25f);
		
		camPitch = Clamp(camPitch, -20.0f, 70.0f);
		
		cameraNode.rotation = Quaternion(camPitch, 0.0f, 0.0f);
		node.rotation = Quaternion(0.0f, playerYaw, 0.0f);
	}
	
	void OnHit(float damagePoint)
	{
		SendEvent("PlayerHit");
		UpdateHealth(-damagePoint);

		if(currentHealth_ == 0 )
		{
			//if health is equal to or below zero notify that the player
			//has been destroyed
			currentHealth_ = 0.0f;
			VariantMap eventData;
			
			Node@ cameraNode = node.GetChild("CameraNode");
			eventData["CamPosition"] = cameraNode.worldPosition;
			eventData["CamRotation"] = cameraNode.worldRotation;
			
			eventData["PlayerPosition"] = node.worldPosition;
			eventData["PlayerRotation"] = node.worldRotation;
			
			SendEvent("PlayerDestroyed",eventData);
			
			node.GetChild("CameraNode").GetChild("DirectionalLight").enabled = false;
		}
		
		healthIncrementTimeCounter_ = 0;
	}
	
	void FixedUpdate(float timeStep)
	{
		healthIncrementTimeCounter_ += timeStep;
		
		if(healthIncrementTimeCounter_ >= healthIncrementTime_)
		{
			healthIncrementTimeCounter_ = 0;

			UpdateHealth(healthIncrement_);	
		}
	}
	
	void UpdateHealth(float incrementValue)
	{
		if(currentHealth_ == maximumHealth_ && incrementValue > 0)
		{
			return;   //This is to avoid unnecessary sending of health update event when the health is full
		}
		
		currentHealth_ += incrementValue;
		
		if(currentHealth_ < 0)
		{
			currentHealth_ = 0;
		}
		else if(currentHealth_ > maximumHealth_)
		{
			currentHealth_ = maximumHealth_;
		}
		
		float healthFraction = currentHealth_ / maximumHealth_;
		VariantMap eventData;
		eventData["CurrentHealthFraction"] = healthFraction;
		SendEvent("PlayerHealthUpdate", eventData);
	}
	
	void SetWeapon(Weapon@ weapon)
	{
		weapon_ = weapon;
	}
}

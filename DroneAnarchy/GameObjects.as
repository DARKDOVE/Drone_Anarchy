///Player Object

class PlayerObject : ScriptObject
{
	float maximumHealth_;
	float currentHealth_;
	float healthDeductionRate_;
	
	
	PlayerObject()
	{
		maximumHealth_ = currentHealth_ = 100;
		healthDeductionRate_ = 2;
	}
	
	void OnHit()
	{
		currentHealth_ -= healthDeductionRate_;
		if(currentHealth_ < 0.0f )
		{
			currentHealth_ = 0.0f;
		}
		
		float healthFraction = currentHealth_ / maximumHealth_;
		VariantMap eventData;
		eventData["CurrentHealthFraction"] = healthFraction;
		SendEvent("PlayerHit", eventData);
	}
}

///Drone Object

class DroneObject : ScriptObject
{
	
	float currentHealthLevel_;
	float attackTime_;
	float attackTimer_;
	
	bool hasAttacked_;
	
	
	DroneObject()
	{
		currentHealthLevel_ = 6;
		attackTime_ = 37;
		attackTimer_ = 0;
		hasAttacked_ = false;
		
	}
	
	
	void DelayedStart()
	{
		SetupNodeAnimation();
		SubscribeToEvent(node, "NodeCollision", "HandleNodeCollision");
	}
	
	void SetupNodeAnimation()
	{
		ValueAnimation@ valAnim = ValueAnimation();
		
		valAnim.SetKeyFrame(0.0f, Variant(node.position));
		valAnim.SetKeyFrame(90.0f, Variant(node.rotation * Vector3(0,0,-35)));
		node.SetAttributeAnimation("Position", valAnim);
	}
	
	
	void FixedUpdate(float timestep)
	{
		
		if(currentHealthLevel_ <= 0)
		{
			//Add explosion Effects
			VariantMap eventData;
			eventData["DronePosition"] = node.worldPosition;
			SendEvent("DroneDestroyed", eventData);
			Destroy();
		}
		else if(!hasAttacked_)
		{
			attackTimer_ += timestep;
			if(attackTimer_ >= attackTime_)
			{
				Attack();
			}
		}
		
	}
	
	
	void HandleNodeCollision(StringHash eventType, VariantMap& eventData)
	{
		Node@ otherNode = eventData["OtherNode"].GetPtr();
		PlayerObject@ playerObj = cast<PlayerObject>(otherNode.scriptObject);
		
		if(playerObj !is null)
		{
			playerObj.OnHit();
			Destroy();
		}
	}
	
	void Attack()
	{
		node.animationEnabled = false;
			
		AnimationController@ animController = node.GetComponent("AnimationController");
		animController.PlayExclusive("DroneAnarchy/Resources/Models/close_arm.ani", 0, false);
		RigidBody@ objectBody = node.GetComponent("RigidBody");
		objectBody.kinematic = false;
				
		objectBody.linearVelocity = node.rotation * Vector3(0,-15, -35);
		hasAttacked_ = true;
	}
	
	void Destroy()
	{
		Sprite@ nodeSprite = node.vars["Sprite"].GetPtr();
		nodeSprite.Remove();
		node.Remove();
	}
	
	void OnHit()
	{
		currentHealthLevel_ -= 1;
	}
	
}


///Bullet Object Class
class BulletObject : ScriptObject
{
	float termTime_;
	float termTimeCounter_;
	
	
	BulletObject()
	{
		termTime_ = 1;
		termTimeCounter_ = 0;
	}
	
	
	void Start()
	{
		SubscribeToEvent(node, "NodeCollision", "HandleNodeCollision");
	}
	
	
	void FixedUpdate(float timestep)
	{
		termTimeCounter_ += timestep;
		
		if(termTimeCounter_ >= termTime_)
		{
			Destroy();
		}
	}
	

	void HandleNodeCollision(StringHash eventType, VariantMap& eventData)
	{
		Node@ otherNode = eventData["OtherNode"].GetPtr();
		DroneObject@ droneObj = cast<DroneObject>(otherNode.scriptObject);
		
		
		if(droneObj !is null)
		{
			droneObj.OnHit();
			SendEvent("DroneHit");
		}
		
		Destroy();
	}
	
	void Destroy()
	{
		node.Remove();
	}
}



///Explosion Object
class ExplosionObject : ScriptObject
{
	float duration_;
	
	
	ExplosionObject()
	{
		duration_ = 0.78f;
	}
	
	void FixedUpdate(float timestep)
	{
		duration_ -= timestep;
		if(duration_ < 0.0f)
		{
			node.Remove();
		}
	}
	
	
}

///Player Object

class PlayerObject : ScriptObject
{
	float maximumHealth;
	float currentHealth;
	float healthDeductionRate;
	
	
	PlayerObject()
	{
		maximumHealth = currentHealth = 100;
		healthDeductionRate = 2;
	}
	
	void OnHit()
	{
		currentHealth -= healthDeductionRate;
		if(currentHealth < 0.0f )
		{
			currentHealth = 0.0f;
		}
		
		float healthFraction = currentHealth / maximumHealth;
		VariantMap eventData;
		eventData["CurrentHealthFraction"] = healthFraction;
		SendEvent("PlayerHit", eventData);
	}
}

///Drone Object

class DroneObject : ScriptObject
{
	
	float currentHealthLevel;
	float attackTime;
	float attackTimer;
	
	bool hasAttacked;
	
	
	DroneObject()
	{
		currentHealthLevel = 6;
		attackTime = 37;
		attackTimer = 0;
		hasAttacked = false;
		
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
		
		if(currentHealthLevel <= 0)
		{
			//Add explosion Effects
			VariantMap eventData;
			eventData["DronePosition"] = node.worldPosition;
			SendEvent("DroneDestroyed", eventData);
			Destroy();
		}
		else if(!hasAttacked)
		{
			attackTimer += timestep;
			if(attackTimer >= attackTime)
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
		hasAttacked = true;
	}
	
	void Destroy()
	{
		Sprite@ nodeSprite = node.vars["Sprite"].GetPtr();
		nodeSprite.Remove();
		node.Remove();
	}
	
	void OnHit()
	{
		currentHealthLevel -= 1;
	}
	
}


///Bullet Object Class
class BulletObject : ScriptObject
{
	float termTime;
	float termTimeCounter;
	
	
	BulletObject()
	{
		termTime = 1;
		termTimeCounter = 0;
	}
	
	
	void Start()
	{
		SubscribeToEvent(node, "NodeCollision", "HandleNodeCollision");
	}
	
	
	void FixedUpdate(float timestep)
	{
		termTimeCounter += timestep;
		
		if(termTimeCounter >= termTime)
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
			SendEvent("DroneHit",VariantMap());
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
	float duration;
	
	
	ExplosionObject()
	{
		duration = 0.78f;
	}
	
	void FixedUpdate(float timestep)
	{
		duration -= timestep;
		if(duration < 0.0f)
		{
			node.Remove();
		}
	}
	
	
}

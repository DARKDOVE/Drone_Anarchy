//-----------------------------------------------DRONE OBJECT--------------------------------------------------------------------------------


///Drone Base Class
abstract class Drone : ScriptObject
{
	float currentHealthLevel_;
	DroneType droneType_;
	
	bool hasAttacked_;
	
	DroneType droneType
	{
		get const
		{
			return droneType_;
		}
	}
	
	void Start(){}
	
	void DelayedStart()
	{
		SubscribeToEvent(node, "NodeCollision", "HandleNodeCollision");
		Initialise();
	}
	
	void Initialise(){}
	
	void HandleNodeCollision(StringHash eventType, VariantMap& eventData){}
	
	void Attack(){}
	
	void FixedUpdate(float timestep)
	{
		
		if(currentHealthLevel_ <= 0)
		{
			//Add explosion Effects
			OnDestroyed();
			Destroy();
		}
		
	}
	
	void OnDestroyed(){}
	
	void Destroy()
	{
		Sprite@ nodeSprite = node.vars["Sprite"].GetPtr();
		nodeSprite.Remove();
		node.Remove();
	}
	
	void OnHit(float damagePoint)
	{
		currentHealthLevel_ -= damagePoint;
	}
	
}


///Low Level Drone Object
class LowLevelDrone : Drone
{
	int dronePoint_;
	int damagePoint_;
	
	LowLevelDrone()
	{
		droneType_ = DT_NORMAL_DRONE;
		currentHealthLevel_ = 6;
		hasAttacked_ = false;
		dronePoint_ = 2;
		damagePoint_ = 2;
		
	}
	
	void Start()
	{
		float nodeYaw = Random(360);
		Quaternion rot = Quaternion(0,nodeYaw, 0);
		
		node.SetTransform(rot * Vector3(0,4,40), rot);
		//node.rotation = Quaternion(0,nodeYaw, 0);
		//node.Translate(Vector3(0,7,40));
	}
	
	
	void DelayedStart()
	{
		
		Drone::DelayedStart();
	}

	void FixedUpdate(float timestep)
	{
		Drone::FixedUpdate(timestep);

		
		if(!hasAttacked_ && currentHealthLevel_ > 0)
		{
			Vector3 playerPos = Vector3(0,0,0); //used 0, 0, 0 as temprorary player position
			Vector3 dronePos = node.worldPosition;

			if((dronePos - playerPos).lengthSquared <= 50.0f)
			{
				Attack();			
			}
		}

		
	}

	void SetupNodeAnimation()
	{
		ValueAnimation@ valAnim = ValueAnimation();
		
		valAnim.SetKeyFrame(0.0f, Variant(node.position));
		valAnim.SetKeyFrame(20.0f, Variant(node.rotation * Vector3(0,4,-35)));
		node.SetAttributeAnimation("Position", valAnim);
	}
	
	
	void HandleNodeCollision(StringHash eventType, VariantMap& eventData)
	{
		Node@ otherNode = eventData["OtherNode"].GetPtr();
		PlayerObject@ playerObj = cast<PlayerObject>(otherNode.scriptObject);
		
		if(playerObj !is null)
		{
			playerObj.OnHit(damagePoint_);
			Destroy();
		}
	}
	
	void Attack()
	{
		node.animationEnabled = false;
			
		AnimationController@ animController = node.GetComponent("AnimationController");
		animController.PlayExclusive("Resources/Models/close_arm.ani", 0, false);
		RigidBody@ objectBody = node.GetComponent("RigidBody");
		objectBody.kinematic = false;
				
		objectBody.linearVelocity = node.rotation * Vector3(0,-25, -35);
		hasAttacked_ = true;
	}
	
	void Initialise()
	{
		AnimationController@ animController = node.GetComponent("AnimationController");
		animController.PlayExclusive("Resources/Models/open_arm.ani", 0, false);
		SetupNodeAnimation();
	}
	
	void OnDestroyed()
	{
		VariantMap eventData;
		eventData["DronePoint"] = dronePoint_;
		SendEvent("DroneDestroyed", eventData);
		
		SpawnExplosion();
	}
	
	void SpawnExplosion()
	{
		Node@ explosionNode = node.scene.CreateChild("ExplosionNode");
		explosionNode.worldPosition = node.worldPosition;	 
		explosionNode.CreateScriptObject(scriptFile, "SimpleExplosion");
	}
	
	
}

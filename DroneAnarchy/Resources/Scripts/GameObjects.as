//Drone Types Enum
enum DroneType
{
	DT_NORMAL_DRONE,
	DT_ALPHA_DRONE,
	DT_SUPREME_DRONE,
	DT_TOKEN_DRONE
}

//Bullet Type Enum
enum BulletObjectType
{
	BOT_LOW,
	BOT_NORMAL,
	BOT_POWERFUL
}

//Bullet Physics Mask
const int BULLET_COLLISION_LAYER = 1;
const int PLAYER_COLLISION_LAYER = 2;
const int DRONE_COLLISION_LAYER = 3;
const int FLOOR_COLLISION_LAYER = 5;
const int SCORE_ADDITION_RATE = 1;

///Player Object

class PlayerObject : ScriptObject
{
	float maximumHealth_;
	float currentHealth_;
	
	
	PlayerObject()
	{
		maximumHealth_ = currentHealth_ = 100;
	}
	
	void OnHit(float damagePoint)
	{
		currentHealth_ -= damagePoint;
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


///Drone Base Class
abstract class DroneObjectBase : ScriptObject
{
	float currentHealthLevel_;
	float attackTime_;
	float attackTimer_;
	DroneType droneType_;
	
	bool hasAttacked_;
	
	DroneType droneType
	{
		get const
		{
			return droneType_;
		}
	}
	
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
		else if(!hasAttacked_)
		{
			attackTimer_ += timestep;
			if(attackTimer_ >= attackTime_)
			{
				Attack();
			}
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
class LowLevelDrone : DroneObjectBase
{
	int dronePoint_;
	int damagePoint_;
	
	LowLevelDrone()
	{
		droneType_ = DT_NORMAL_DRONE;
		currentHealthLevel_ = 6;
		attackTime_ = 37;
		attackTimer_ = 0;
		hasAttacked_ = false;
		dronePoint_ = 2;
		damagePoint_ = 2;
		
	}
	
	
	void DelayedStart()
	{
		
		DroneObjectBase::DelayedStart();
		SetupNodeAnimation();
	}
	
	void SetupNodeAnimation()
	{
		ValueAnimation@ valAnim = ValueAnimation();
		
		valAnim.SetKeyFrame(0.0f, Variant(node.position));
		valAnim.SetKeyFrame(90.0f, Variant(node.rotation * Vector3(0,0,-35)));
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
				
		objectBody.linearVelocity = node.rotation * Vector3(0,-15, -35);
		hasAttacked_ = true;
	}
	
	void Initialise()
	{
		AnimatedModel@ droneBody = node.CreateComponent("AnimatedModel");
		droneBody.model = cache.GetResource("Model", "Resources/Models/drone_body.mdl");
		droneBody.material = cache.GetResource("Material", "Resources/Materials/drone_body.xml");
		
		AnimatedModel@ droneArm = node.CreateComponent("AnimatedModel");
		droneArm.model = cache.GetResource("Model", "Resources/Models/drone_arm.mdl");
		droneArm.material = cache.GetResource("Material", "Resources/Materials/drone_arm.xml");
		
		RigidBody@ droneRB = node.CreateComponent("RigidBody");
		droneRB.mass = 1.0f;
		droneRB.SetCollisionLayerAndMask(DRONE_COLLISION_LAYER, BULLET_COLLISION_LAYER | PLAYER_COLLISION_LAYER | FLOOR_COLLISION_LAYER);
		droneRB.kinematic = true;
		
		CollisionShape@ droneCS = node.CreateComponent("CollisionShape");
		droneCS.SetSphere(0.3f);
		
		AnimationController@ animController = node.CreateComponent("AnimationController");
		animController.PlayExclusive("Resources/Models/open_arm.ani", 0, false);
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


///Bullet Base Class
abstract class BulletObjectBase : ScriptObject
{
	float termTime_;
	float termTimeCounter_;
	BulletObjectType bulletObjectType_;
	float damagePoint_;
	
	
	void DelayedStart()
	{
		SubscribeToEvent(node, "NodeCollision", "HandleNodeCollision");
		Initialise();
	}
	
	void Initialise(){}
	
	
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
		DroneObjectBase@ droneObj = cast<DroneObjectBase>(otherNode.scriptObject);
		
		
		if(droneObj !is null)
		{
			droneObj.OnHit(damagePoint_);
		}
		
		Destroy();
	}
	
	void Destroy()
	{
		node.Remove();
	}
	
}

///Low Level Bullet Object
class LowLevelBullet : BulletObjectBase
{
	LowLevelBullet()
	{
		bulletObjectType_ = BOT_LOW;
		termTime_ = 1;
		termTimeCounter_ = 0;
		damagePoint_ = 1;
	}
	
	void Initialise()
	{
		BillboardSet@ bbSet = node.CreateComponent("BillboardSet");
		bbSet.numBillboards = 1;
		bbSet.material = cache.GetResource("Material", "Resources/Materials/bullet_particle.xml");
		
		ParticleEmitter@ pEmitter = node.CreateComponent("ParticleEmitter");
		pEmitter.effect = cache.GetResource("ParticleEffect", "Resources/Particles/bullet_particle.xml");
		pEmitter.enabled = true;
		
		
		RigidBody@ objBody = node.CreateComponent("RigidBody");
		objBody.mass = 1.0f;
		objBody.trigger = true;
		objBody.useGravity = false;
		objBody.ccdRadius = 0.05;
		objBody.ccdMotionThreshold = 0.15f;
		objBody.SetCollisionLayerAndMask(BULLET_COLLISION_LAYER, DRONE_COLLISION_LAYER | FLOOR_COLLISION_LAYER);
		
		CollisionShape@ objShape = node.CreateComponent("CollisionShape");
		objShape.SetSphere(0.3f);
		objBody.linearVelocity = node.rotation * Vector3(0,0,70);
	}

}

///Explosion Object Base
abstract class ExplosionObjectBase : ScriptObject
{
	float duration_;
	
	void DelayedStart()
	{
		Initialise();
	}
	
	void Initialise(){}
}

///Explosion Object
class SimpleExplosion : ExplosionObjectBase 
{
	
	SimpleExplosion()
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
	
	void Initialise()
	{
		ParticleEmitter@ pEmitter = node.CreateComponent("ParticleEmitter");
		pEmitter.effect = cache.GetResource("ParticleEffect", "Resources/Particles/explosion.xml");
		pEmitter.enabled = true;
		
		VariantMap eventData;
		eventData["SoundNode"] = node;
		eventData["SoundName"] = "Resources/Sounds/explosion.ogg";
		SendEvent("SoundGenerated", eventData);
	}
	
	
}



///Weapon Base Class
abstract class WeaponObjectBase
{
	Node@ refNode_;
	
	WeaponObjectBase(Node@ refNode)
	{
		refNode_ = refNode;
	}
	void Fire(){}
}


///Ordinary Weapon
class OrdinaryWeapon : WeaponObjectBase
{
	void Fire()
	{	
		SpawnBullet(true);
		SpawnBullet(false);
	}
	
	OrdinaryWeapon(Node@ refNode)
	{
		super(refNode);
	}

	void SpawnBullet(bool first)
	{
		Node@ bulletNode = scene_.CreateChild();
		bulletNode.worldPosition = refNode_.worldPosition;
		bulletNode.rotation = refNode_.worldRotation;
		
		float xOffSet = 0.3f * (first ? 1 : -1);
		bulletNode.Translate(Vector3(xOffSet,-0.2,0));
		
		bulletNode.CreateScriptObject(scriptFile, "LowLevelBullet");
	}
}


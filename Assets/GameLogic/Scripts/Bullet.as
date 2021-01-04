
//-----------------------------------------------BULLET OBJECT--------------------------------------------------------------------------------
///Bullet Base Class
abstract class Bullet : ScriptObject
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
		Drone@ droneObj = cast<Drone>(otherNode.scriptObject);
		
		
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
class LowLevelBullet : Bullet
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
		bbSet.material = cache.GetResource("Material", "Materials/bullet_particle.xml");
		
		ParticleEmitter@ pEmitter = node.CreateComponent("ParticleEmitter");
		pEmitter.effect = cache.GetResource("ParticleEffect", "Particles/bullet_particle.xml");
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




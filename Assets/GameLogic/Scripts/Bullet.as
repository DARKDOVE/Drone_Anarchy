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




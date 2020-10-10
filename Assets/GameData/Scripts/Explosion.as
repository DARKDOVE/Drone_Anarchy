//-----------------------------------------------EXPLOSION OBJECT--------------------------------------------------------------------------------


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
		pEmitter.effect = cache.GetResource("ParticleEffect", "Particles/explosion.xml");
		pEmitter.enabled = true;
		
		VariantMap eventData;
		eventData["SoundNode"] = node;
		eventData["SoundName"] = "Sounds/explosion.ogg";
		SendEvent("SoundGenerated", eventData);
	}
	
	
}







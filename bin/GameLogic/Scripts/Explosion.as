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







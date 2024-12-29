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


//-----------------------------------------------WEAPON OBJECT--------------------------------------------------------------------------------

///Weapon Base Class
abstract class Weapon
{
	Node@ refNode_;
	
	Weapon(Node@ refNode)
	{
		refNode_ = refNode;
	}
	void Fire(){}
}


///Ordinary Weapon
class OrdinaryWeapon : Weapon
{
	void Fire()
	{	
		SpawnBullet(true);
		SpawnBullet(false);
		VariantMap eventData;
		eventData["SoundNode"] = refNode_;
		eventData["SoundName"] = "Sounds/boom1.wav";
		SendEvent("SoundGenerated", eventData);
	}
	
	OrdinaryWeapon(Node@ refNode)
	{
		super(refNode);
	}

	void SpawnBullet(bool first)
	{
		Node@ bulletNode = refNode_.scene.CreateChild();
		bulletNode.worldPosition = refNode_.worldPosition;
		bulletNode.rotation = refNode_.worldRotation;
		
		float xOffSet = 0.3f * (first ? 1 : -1);
		bulletNode.Translate(Vector3(xOffSet,-0.2,0));
		
		bulletNode.CreateScriptObject("Scripts/GameObjects.as", "LowLevelBullet");
	}
}

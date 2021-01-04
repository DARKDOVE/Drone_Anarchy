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
		
		ScriptFile@ sFile = cache.GetResource("ScriptFile","Scripts/GameObjects.as");
		bulletNode.CreateScriptObject(sFile, "LowLevelBullet");
	}
}


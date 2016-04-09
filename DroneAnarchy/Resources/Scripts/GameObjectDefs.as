//Drone Types Enum
enum DroneType
{
	DT_NORMAL_DRONE,
	DT_ALPHA_DRONE,
	DT_SUPREME_DRONE,
	DT_TOKEN_DRONE
}

//Weapon Type
const int ORDINARY_WEAPON = 1;
const int CLASSIC_WEAPON = 2;
const int ENHANCED_WEAPON = 3;
const int ADVANCED_WEAPON = 4;



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

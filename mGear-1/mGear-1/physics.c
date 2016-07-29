#include <math.h>
#include "physics.h"
#include "engine.h"

Physics physics;

void InitPhysics(int8 gravity, int32 gravity_vel, int16 gravity_direction, int8 air)
{
	physics.gravity = gravity;
	physics.gravity_vel = gravity_vel;
	physics.gravity_direction = gravity_direction;
	physics.air = air;
	physics.physics_active = 1;
}

int8 AddDynamicSprite(int16 id)
{
	int16 i;

	if (physics.num_sprites < MAX_DYNAMIC_SPRITES)
	{
		//Check if it's already there
		for (i = 0; i < physics.num_sprites; i++)
		{
			if (physics.sprite_list[i] == id)
			{
				LogApp("Sprite %d already dynamic", id);
				return NULL;
			}
		}

		physics.sprite_list[physics.num_sprites] = id;
		physics.num_sprites++;

		st.Current_Map.sprites[id].body.velxy.x = 0;
		st.Current_Map.sprites[id].body.velxy.y = 0;
		st.Current_Map.sprites[id].body.total_vel = 0;
		st.Current_Map.sprites[id].body.acceleration = 0;
		st.Current_Map.sprites[id].body.energy = 0;
	}

	return 1;
}

int8 MainPhysics()
{
	int32 i, j;

	if (physics.physics_active)
	{
		for (i = 0; i < physics.num_sprites; i++)
		{
			if (physics.gravity)
			{
				j = physics.sprite_list[i];

				if (CheckCollisionSector(st.Current_Map.sprites[j].position.x, st.Current_Map.sprites[j].position.y, st.Current_Map.sprites[j].body.size.x,
					st.Current_Map.sprites[j].body.size.y, st.Current_Map.sprites[j].body.ang))
				{
					if (st.Current_Map.sprites[j].body.velxy.y > 0)
						st.Current_Map.sprites[j].body.velxy.y = 0;
				}
				else
				{
					st.Current_Map.sprites[j].body.velxy.x += mCos(physics.gravity_direction) * physics.gravity_vel;
					st.Current_Map.sprites[j].body.velxy.y += mSin(physics.gravity_direction) * physics.gravity_vel;
				}
			}

			if (CheckCollisionSectorWall(st.Current_Map.sprites[j].position.x, st.Current_Map.sprites[j].position.y, st.Current_Map.sprites[j].body.size.x,
				st.Current_Map.sprites[j].body.size.y, st.Current_Map.sprites[j].body.ang))
			{
				if (st.Current_Map.sprites[j].body.velxy.x > 0)
					st.Current_Map.sprites[j].body.velxy.x = 0;
			}
		}
	}
}
#include <math.h>
#include "physics.h"
#include "engine.h"

Physics physics;

void InitPhysics(int8 gravity, int32 gravity_vel, int16 gravity_direction, int8 air)
{
	int16 tmp = 0, i = 0, k = 4;

	physics.gravity = gravity;
	physics.gravity_vel = gravity_vel;
	physics.gravity_direction = gravity_direction;
	physics.air = air;
	physics.physics_active = 1;

	//Create the quadtree nodes and subnodes
	//By default there are 21 nodes
	/*
	physics.nodes[0].min.x = 0;
	physics.nodes[0].min.y = 0;
	physics.nodes[0].max.x = GAME_SCREEN_WIDTH;
	physics.nodes[0].max.y = GAME_SCREEN_HEIGHT;

	physics.nodes[0].num_sprites = 0;

	physics.nodes[0].sub_nodes[0] = 1;
	physics.nodes[0].sub_nodes[1] = 2;
	physics.nodes[0].sub_nodes[2] = 3;
	physics.nodes[0].sub_nodes[3] = 4;

	memset(physics.nodes[0].sprites, -1, MAX_ACTIVE_DYNAMIC_SPRITES*sizeof(int16));

	for (i = 1, k = 4; i < 5; i++, k += 4)
	{
		physics.nodes[i].min.x = 0 + ((i - 1) * (GAME_SCREEN_WIDTH / k));
		physics.nodes[i].min.y = 0 + ((i - 1) * (GAME_SCREEN_HEIGHT / k));
		physics.nodes[i].max.x = i * (GAME_SCREEN_WIDTH / k);
		physics.nodes[i].max.y = i * (GAME_SCREEN_HEIGHT / k);

		physics.nodes[i].num_sprites = 0;

		physics.nodes[i].sub_nodes[0] = 1 + k;
		physics.nodes[i].sub_nodes[1] = 2 + k;
		physics.nodes[i].sub_nodes[2] = 3 + k;
		physics.nodes[i].sub_nodes[3] = 4 + k;

		memset(physics.nodes[i].sprites, -1, MAX_ACTIVE_DYNAMIC_SPRITES*sizeof(int16));
	}

	for (i = 5, k = 16; i < 22; i++)
	{
		physics.nodes[i].min.x = 0 + ((i - 5) * (GAME_SCREEN_WIDTH / k));
		physics.nodes[i].min.y = 0 + ((i - 5) * (GAME_SCREEN_HEIGHT / k));
		physics.nodes[i].max.x = i * (GAME_SCREEN_WIDTH / k);
		physics.nodes[i].max.y = i * (GAME_SCREEN_HEIGHT / k);

		physics.nodes[i].num_sprites = 0;

		physics.nodes[i].sub_nodes[0] = physics.nodes[i].sub_nodes[1] = physics.nodes[i].sub_nodes[3] = physics.nodes[i].sub_nodes[4] = -1;

		memset(physics.nodes[i].sprites, -1, MAX_ACTIVE_DYNAMIC_SPRITES*sizeof(int16));
	}
	*/
	memset(&physics.sectorarea, 0, MAX_SECTORS * sizeof(uint16));
	memset(&physics.sectorarea_ids, -1, MAX_SECTORS * MAX_DYNAMIC_SPRITES * sizeof(int16));

	memset(&physics.spritesectornum, 0, MAX_ACTIVE_DYNAMIC_SPRITES*sizeof(int16));
	memset(&physics.spritesectors, 0, MAX_ACTIVE_DYNAMIC_SPRITES*MAX_SECTORS*sizeof(int8));

	physics.num_particles = physics.num_proj = 0;

	for (i = 0; i < MAX_PROJECTILES; i++)
	{
		physics.proj[i].owner = -1;
		physics.proj[i].stat = -1;
	}

	for (i = 0; i < MAX_PARTICLES; i++)
	{
		physics.particles[i].stat = -1;
	}

	physics.num_particles = 0;
	physics.num_proj = 0;
}

uint8 DefineProjectileProperties(int8 id, enum PROJ_TYPE type, int16 damage, uint32 radius, uint32 time, int16 trail, uint8 num_trail, int16 spawns)
{
	if (id >= MAX_GAME_PROJECTILES)
	{
		LogApp("Error: projectile ID %d above maximum limit", id);
		return NULL;
	}
	else
	{
		physics.game_proj[id].type = type;
		physics.game_proj[id].damage = damage;
		physics.game_proj[id].radius = radius;
		physics.game_proj[id].time = time;
		physics.game_proj[id].trail = trail;
		physics.game_proj[id].num_trail = num_trail;
		physics.game_proj[id].spawns = spawns;
		return 1;
	}
}

uint8 DefineProjectilePhysics(int8 id, uint8 bounce, uint8 num_bounces, uint8 rnd_bounce, uint8 vel_bounce, int16 mass, int32 sizex, int32 sizey, int32 speed)
{
	if (id >= MAX_GAME_PROJECTILES)
	{
		LogApp("Error: projectile ID %d above maximum limit", id);
		return NULL;
	}
	else
	{
		physics.game_proj[id].bounce = bounce;
		physics.game_proj[id].num_bounces = num_bounces;
		physics.game_proj[id].rnd_bounce = rnd_bounce;
		physics.game_proj[id].vel_bounce = vel_bounce;
		physics.game_proj[id].mass = mass;
		physics.game_proj[id].size.x = sizex;
		physics.game_proj[id].size.y = sizey;
		physics.game_proj[id].speed = speed;
		return 1;
	}
}

uint8 DefineProjectileVisual(int8 id, int16 frame, uint8 mgg, uint8 anim_id, uint8 animated, float speed_mul)
{
	if (id >= MAX_GAME_PROJECTILES)
	{
		LogApp("Error: projectile ID %d above maximum limit", id);
		return NULL;
	}
	else
	{
		physics.game_proj[id].frame_id = frame;
		physics.game_proj[id].mgg_id = mgg;
		physics.game_proj[id].anim_id = anim_id;
		physics.game_proj[id].animated = animated;
		physics.game_proj[id].speed_mul = speed_mul;
		return 1;
	}
}

uint8 ShootProjectile(int8 id, int16 ang, int16 owner, Pos pos)
{
	register int8 i = 0;

	if (id >= MAX_GAME_PROJECTILES)
	{
		LogApp("Error: projectile ID %d above maximum limit, unable to shoot it", id);
		return NULL;
	}
	else
	{
		for (i = 0; i < MAX_PROJECTILES; i++)
		{
			if (physics.proj[i].stat == -1)
			{
				physics.proj[i].stat = 1;
				physics.proj[i].ang = ang;
				physics.proj[i].vel.x = (float) physics.game_proj[id].speed * mCos(ang);
				physics.proj[i].vel.y = (float) physics.game_proj[id].speed * mSin(ang);
				physics.proj[i].animated = physics.game_proj[id].animated;
				physics.proj[i].anim_id = physics.game_proj[id].anim_id;
				physics.proj[i].bounce = physics.game_proj[id].bounce;
				physics.proj[i].damage = physics.game_proj[id].damage;
				physics.proj[i].frame_id = physics.game_proj[id].frame_id;
				physics.proj[i].mass = physics.game_proj[id].mass;
				physics.proj[i].mgg_id = physics.game_proj[id].mgg_id;
				physics.proj[i].num_bounces = physics.game_proj[id].num_bounces;
				physics.proj[i].pos = pos;
				physics.proj[i].owner = owner;
				physics.proj[i].radius = physics.game_proj[id].radius;
				physics.proj[i].rnd_bounce = physics.game_proj[id].rnd_bounce;
				physics.proj[i].size = physics.game_proj[id].size;
				physics.proj[i].speed_mul = physics.game_proj[id].speed_mul;
				physics.proj[i].time = physics.game_proj[id].time;
				physics.proj[i].type = physics.game_proj[id].type;
				physics.proj[i].vel_bounce = physics.game_proj[id].vel_bounce;
				physics.proj[i].spawns = physics.game_proj[id].spawns;
				physics.proj[i].trail = physics.game_proj[id].trail;
				physics.proj[i].num_trail = physics.game_proj[id].num_trail;
				physics.proj[i].trail_time = 0;

				return 1;
			}
		}
	}
}

uint8 DefineParticle(int8 id, int16 frame, uint8 mgg, uint8 anim_id, uint8 animated, float speed_mul, int16 speed, void (*func)(PARTICLES))
{
	if (id >= MAX_PARTICLES)
	{
		LogApp("Error: particle ID %d above maximum limit", id);
		return NULL;
	}
	else
	{
		physics.game_part[id].frame_id = frame;
		physics.game_part[id].anim_id = anim_id;
		physics.game_part[id].mgg_id = mgg;
		physics.game_part[id].animated = animated;
		physics.game_part[id].speed_mul = speed_mul;
		physics.game_part[id].speed = speed;
		physics.game_part[id].func = func;

		return 1;
	}
}

uint8 SpawnParticle(int8 id, Pos pos, int16 ang)
{
	register uint8 i;

	if (id >= MAX_PARTICLES)
	{
		LogApp("Error: particle ID %d above maximum limit", id);
		return NULL;
	}
	else
	{
		if (physics.num_particles < MAX_PARTICLES)
		{
			for (i = 0; i < MAX_PARTICLES; i++)
			{
				if (physics.particles[i].stat == -1)
				{
					physics.particles[i].stat = 1;
					physics.particles[i].id = id;
					physics.particles[i].animated = physics.game_part[id].animated;
					physics.particles[i].ang = ang;
					physics.particles[i].pos = pos;
					physics.particles[i].size = physics.game_part[id].size;
					physics.particles[i].anim_id = physics.game_part[id].anim_id;
					physics.particles[i].func = physics.game_part[id].func;
					physics.particles[i].frame_id = physics.game_part[id].frame_id;
					physics.particles[i].mgg_id = physics.game_part[id].mgg_id;
					physics.particles[i].speed = physics.game_part[id].speed;
					physics.particles[i].speed_mul = physics.game_part[id].speed_mul;
					
					return 1;
				}
			}
		}
	}
}

int16 UpdateSector(Pos pos, Pos size)
{
	int32 ydist, ydist2, first = 0, x1, x2, base_y;
	int16 sector_id = -1, i;

	for (i = 0; i < st.Current_Map.num_sector; i++)
	{
		if (st.Current_Map.sector[i].sloped)
		{
			if (st.Current_Map.sector[i].vertex[0].y > st.Current_Map.sector[i].vertex[1].y)
				base_y = st.Current_Map.sector[i].vertex[0].y;
			else
				base_y = st.Current_Map.sector[i].vertex[1].y;
		}
		else
			base_y = st.Current_Map.sector[i].base_y;

		if (st.Current_Map.sector[i].vertex[0].x > st.Current_Map.sector[i].vertex[1].x)
		{
			x2 = st.Current_Map.sector[i].vertex[0].x;
			x1 = st.Current_Map.sector[i].vertex[1].x;
		}
		else
		{
			x2 = st.Current_Map.sector[i].vertex[1].x;
			x1 = st.Current_Map.sector[i].vertex[0].x;
		}

		if (pos.x <= x2 && pos.x >= x1 &&
			pos.y <= base_y)
		{
			if (st.Current_Map.sector[i].sloped)
			{
				if (st.Current_Map.sector[i].vertex[0].y > st.Current_Map.sector[i].vertex[1].y)
					ydist2 = st.Current_Map.sector[i].vertex[0].y;
				else
					ydist2 = st.Current_Map.sector[i].vertex[1].y;
			}
			else
				ydist2 = st.Current_Map.sector[i].base_y;

			if (first == 0)
			{
				ydist = ydist2;
				sector_id = i;
				first = 1;
			}
			else
			{
				if (ydist2 < ydist)
				{
					ydist = ydist2;
					sector_id = i;
				}
			}
		}
	}
	
	return sector_id;
}

void UpdateSectorRange(int16 id, int16 phy_id)
{
	register int16 i;

	int32 x1, x2, base_y;

	Pos pos, size;

	pos = st.Current_Map.sprites[id].position;
	size = st.Current_Map.sprites[id].body.size;

	for (i = 0; i < st.Current_Map.num_sector; i++)
	{
		if (st.Current_Map.sector[i].sloped)
		{
			if (st.Current_Map.sector[i].vertex[0].y > st.Current_Map.sector[i].vertex[1].y)
				base_y = st.Current_Map.sector[i].vertex[0].y;
			else
				base_y = st.Current_Map.sector[i].vertex[1].y;
		}
		else
			base_y = st.Current_Map.sector[i].base_y;

		if (st.Current_Map.sector[i].vertex[0].x > st.Current_Map.sector[i].vertex[1].x)
		{
			x2 = st.Current_Map.sector[i].vertex[0].x;
			x1 = st.Current_Map.sector[i].vertex[1].x;
		}
		else
		{
			x2 = st.Current_Map.sector[i].vertex[1].x;
			x1 = st.Current_Map.sector[i].vertex[0].x;
		}

		if (pos.x <= x2 && pos.x >= x1 &&
			pos.y - (size.y / 2) < base_y)
		{
			physics.sectorarea[i]++;
			physics.sectorarea_ids[i][physics.sectorarea[i]] = id;
			physics.spritesectornum[phy_id]++;
			physics.spritesectors[phy_id][physics.spritesectornum[phy_id]-1] = i;
		}
	}
}

void ActivateDynamicSprite (int16 id)
{

	physics.active_sprites[physics.num_active_sprites] = id;

	physics.num_active_sprites++;

	//id = physics.sprite_list[id];

	/*
	if (st.Current_Map.sprites[id].body.size.x > st.Current_Map.sprites[id].body.size.y)
		radius = st.Current_Map.sprites[id].body.size.x;
	else
		radius = st.Current_Map.sprites[id].body.size.y;

	if (radius > GAME_SCREEN_WIDTH / NODES || radius > GAME_SCREEN_HEIGHT / NODES)
	{
		if (radius > GAME_SCREEN_WIDTH / 4 || radius > GAME_SCREEN_HEIGHT / 4)
		{
			physics.nodes[0].sprites[physics.nodes[0].num_sprites] = physics.num_active_sprites - 1;
			physics.nodes[0].num_sprites++;
		}
		else
		{
			for (i = 1; i < 5; i++)
			{
				if (st.Current_Map.sprites[id].position.x>physics.nodes[i].min.x && st.Current_Map.sprites[id].position.x<physics.nodes[i].max.x &&
					st.Current_Map.sprites[id].position.y>physics.nodes[i].min.y && st.Current_Map.sprites[id].position.y < physics.nodes[i].max.y)
				{
					physics.nodes[i].sprites[physics.nodes[i].num_sprites] = physics.num_active_sprites - 1;
					physics.nodes[i].num_sprites++;
				}
				else
				if (st.Current_Map.sprites[id].position.x + radius>physics.nodes[i].min.x && st.Current_Map.sprites[id].position.x + radius<physics.nodes[i].max.x &&
					st.Current_Map.sprites[id].position.y + radius>physics.nodes[i].min.y && st.Current_Map.sprites[id].position.y + radius < physics.nodes[i].max.y)
				{
					physics.nodes[i].sprites[physics.nodes[i].num_sprites] = physics.num_active_sprites - 1;
					physics.nodes[i].num_sprites++;
				}
			}
		}
	}
	else
	{
		for (i = 5; i < 22; i++)
		{
			if (st.Current_Map.sprites[id].position.x>physics.nodes[i].min.x && st.Current_Map.sprites[id].position.x<physics.nodes[i].max.x &&
				st.Current_Map.sprites[id].position.y>physics.nodes[i].min.y && st.Current_Map.sprites[id].position.y < physics.nodes[i].max.y)
			{
				physics.nodes[i].sprites[physics.nodes[i].num_sprites] = physics.num_active_sprites - 1;
				physics.nodes[i].num_sprites++;
			}
			else
			if (st.Current_Map.sprites[id].position.x + radius>physics.nodes[i].min.x && st.Current_Map.sprites[id].position.x + radius<physics.nodes[i].max.x &&
				st.Current_Map.sprites[id].position.y + radius>physics.nodes[i].min.y && st.Current_Map.sprites[id].position.y + radius < physics.nodes[i].max.y)
			{
				physics.nodes[i].sprites[physics.nodes[i].num_sprites] = physics.num_active_sprites - 1;
				physics.nodes[i].num_sprites++;
			}
		}
	}
	*/
}

void DeactivateDynamicSprite(int16 id)
{
	register int16 i, j;
	uint8 found = 0;

	if (id == physics.num_active_sprites - 1)
	{
		physics.active_sprites[id] = -1;
		physics.num_active_sprites--;
	}
	else
	{
		for (i = id; i < physics.num_active_sprites; i++)
		{
			if (i != physics.num_active_sprites - 1)
			{
				physics.active_sprites[i] = physics.active_sprites[i + 1];
				physics.active_sprites[i + 1] = -1;
			}
			else
				physics.active_sprites[i] = -1;
		}

		physics.num_active_sprites--;
	}
	/*
	for (i = 0; i < NODES + 1; i++)
	{
		if (i == 5 && found)
			break;

		if (physics.nodes[i].num_sprites > 0)
		{
			for (j = 0; j < physics.nodes[i].num_sprites; j++)
			{
				if (physics.nodes[i].sprites[j] == id)
				{
					found = 1;

					if (j != physics.nodes[i].num_sprites - 1)
					{
						physics.nodes[i].sprites[j] = physics.nodes[i].sprites[j + 1];
						physics.nodes[i].sprites[j + 1] = -1;
					}
					else
						physics.nodes[i].sprites[j] = -1;
				}
			}

			physics.nodes[i].num_sprites--;

			if (i == 0 && found)
				break;
		}
	}
	*/
}

int8 AddDynamicSprite(int16 id)
{
	register int16 i;

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
		//st.Current_Map.sprites[id].body.total_vel = 0;
		st.Current_Map.sprites[id].body.acceleration = 0;
		st.Current_Map.sprites[id].body.energy = 0;
		st.Current_Map.sprites[id].body.sector_id = -1;

		st.Current_Map.sprites[id].flags += 16;

		st.Current_Map.sprites[id].body.sector_id = UpdateSector(st.Current_Map.sprites[id].position,st.Current_Map.sprites[id].body.size);
	}

	return 1;
}

void RemoveDynamicSprite(int16 id)
{
	register int16 i;

	if (id != physics.num_sprites - 1)
	{
		for (i = id; i < physics.num_sprites; i++)
		{
			if (i != physics.num_sprites - 1)
			{
				physics.sprite_list[i] = physics.sprite_list[i + 1];
				physics.sprite_list[i + 1] = -1;
			}
			else
				physics.sprite_list[i] = -1;
		}
	}
	else
		physics.sprite_list[id] = -1;

	physics.num_sprites--;
}

int8 OnTheGround(int16 id, int32 *y)
{
	if (st.Current_Map.sprites[id].body.sector_id != -1)
	{
		if (CheckCollisionSector(st.Current_Map.sprites[id].position.x, st.Current_Map.sprites[id].position.y, st.Current_Map.sprites[id].body.size.x,
			st.Current_Map.sprites[id].body.size.y, st.Current_Map.sprites[id].body.ang, &y, st.Current_Map.sprites[id].body.sector_id))
			return 1;
		else
			return NULL;
	}
	else
		return NULL;
}

int8 HitSprite(int16 id)
{
	register int16 i, j, k, l;

	int16 as, id3, id4, side;

	for (i = 0; i < physics.num_active_sprites; i++)
	{
		if (physics.sprite_list[physics.active_sprites[i]] == id)
		{
			as = i;
			break;
		}
	}

	for (i = 0; i < physics.spritesectornum[as]; i++)
	{
		j = physics.spritesectors[as][i];
		for (k = 0; k < physics.sectorarea[j]; k++)
		{
			if (k == as)
				continue;

			id3 = physics.sprite_list[physics.active_sprites[as]];
			id4 = physics.sprite_list[physics.active_sprites[k]];

			if (st.Current_Map.sprites[id3].flags & 8 && st.Current_Map.sprites[id4].flags & 8 && (side = CheckCollision(st.Current_Map.sprites[id3].position,
				st.Current_Map.sprites[id3].body.size, st.Current_Map.sprites[id3].angle, st.Current_Map.sprites[id4].position,
				st.Current_Map.sprites[id4].body.size, st.Current_Map.sprites[id4].angle)) > 0)
			{
				return 1;
			}
		}
	}

	return NULL;
}

int8 HitscanID(Pos pos, int16 ang, int16 spr_id, Pos *rpos)
{
	register int32 i = spr_id;
	Pos pos2, size;
	float m;

	size.x = size.y = 10;

	pos2 = pos;

	if (st.Current_Map.sprites[i].flags & 8)
	{
		m = mTan(ang);

		pos2.x = st.Current_Map.sprites[i].position.x;

		pos2.y = (float)m*(pos2.x - pos.x) + pos.y;

		if (CheckCollision(pos2, size, ang, st.Current_Map.sprites[i].position, st.Current_Map.sprites[i].body.size, st.Current_Map.sprites[i].angle))
		{
			*rpos = pos2;
			return 1;
		}
	}

	return 0;
}

int8 Hitscan(Pos pos, int16 ang, int16  *spr_id, int16 *sector_id, Pos *rpos)
{
	register int32 i, j;
	Pos pos2, size;
	float m;

	size.x = size.y = 10;

	*sector_id = -1;

	pos2 = pos;

	for (j = 0; j < physics.num_active_sprites; j++)
	{
		i = physics.sprite_list[physics.active_sprites[j]];

		if (st.Current_Map.sprites[i].flags & 8)
		{
			m = mTan(ang);

			pos2.x = st.Current_Map.sprites[i].position.x;

			pos2.y = (float)m*(pos2.x - pos.x) + pos.y;

			if (CheckCollision(pos2, size, ang, st.Current_Map.sprites[i].position, st.Current_Map.sprites[i].body.size, st.Current_Map.sprites[i].angle))
			{
				*spr_id = i;
				*rpos = pos2;
				return 1;
			}
		}
	}

	*spr_id = -1;

	for (i = 0; i < st.Current_Map.num_sector; i++)
	{
		if (st.Current_Map.sector[i].sloped && abs(st.Current_Map.sector[i].vertex[1].x - st.Current_Map.sector[i].vertex[0].x) < 512)
		{
			m = mTan(ang);

			pos2.x = st.Current_Map.sector[i].vertex[0].x;

			pos2.y = (float)m*(pos2.x - pos.x) + pos.y;

			if (CheckCollisionSectorWallID(pos2.x, pos2.y, 10, 10, ang, i))
			{
				*sector_id = i;
				*rpos = pos2;
				return 1;
			}
		}
		else
		if (st.Current_Map.sector[i].sloped  && abs(st.Current_Map.sector[i].vertex[1].x - st.Current_Map.sector[i].vertex[0].x) > 512)
		{
			m = mTan(ang);

			pos2.x = st.Current_Map.sector[i].vertex[0].x;

			pos2.y = (float)m*(pos2.x - pos.x) + pos.y;

			if (CheckCollisionSector(pos2.x, pos2.y, 10, 10, ang, NULL, i))
			{
				*sector_id = i;
				*rpos = pos2;
				return 1;
			}
		}
		else
		if (!st.Current_Map.sector[i].sloped)
		{
			m = mTan(ang);

			pos2.x = st.Current_Map.sector[i].vertex[0].x;

			pos2.y = (float)m*(pos2.x - pos.x) + pos.y;

			if (CheckCollisionSector(pos2.x, pos2.y, 10, 10, ang, NULL, i))
			{
				*sector_id = i;
				*rpos = pos2;
				return 1;
			}
		}
	}

	*sector_id = -1;

	return 0;
}

int8 HitProj(int16 id, uint8 *proj_id)
{
	register uint8 i;

	int16 as, id3, id4, side;

	for (i = 0; i < physics.num_active_sprites; i++)
	{
		if (physics.sprite_list[physics.active_sprites[i]] == id)
		{
			as = i;
			break;
		}
	}

	for (i = 0; i < physics.num_proj; i++)
	{
		if (physics.proj[i].stat > -1 && physics.proj[i].type & PROJ_BULLET)
		{
			if (HitscanID(physics.proj[i].pos, physics.proj[i].ang, id, NULL))
			{
				*proj_id = physics.proj[i].id;
				return 1;
			}
		}
		else
		if (physics.proj[i].stat > -1 && physics.proj[i].type & PROJ_REGULAR && 
			CheckCollision(physics.proj[i].pos, physics.proj[i].size, physics.proj[i].ang, 
			st.Current_Map.sprites[id].position, st.Current_Map.sprites[id].body.size, st.Current_Map.sprites[id].angle))
		{
			*proj_id = physics.proj[i].id;
			return 1;
		}
	}

	return 0;
}

int8 HitProjID(int16 id, uint8 proj_id)
{
	register uint8 i;

	int16 as, id3, id4, side;

	for (i = 0; i < physics.num_active_sprites; i++)
	{
		if (physics.sprite_list[physics.active_sprites[i]] == id)
		{
			as = i;
			break;
		}
	}

	for (i = 0; i < physics.num_proj; i++)
	{
		if (physics.proj[i].stat > -1 && physics.proj[i].type & PROJ_BULLET && physics.proj[i].id==proj_id)
		{
			if (HitscanID(physics.proj[i].pos, physics.proj[i].ang, id, NULL))
				return 1;
		}
		else
		if (physics.proj[i].stat > -1 && physics.proj[i].id==proj_id && physics.proj[i].type & PROJ_REGULAR &&
			CheckCollision(physics.proj[i].pos, physics.proj[i].size, physics.proj[i].ang,
			st.Current_Map.sprites[id].position, st.Current_Map.sprites[id].body.size, st.Current_Map.sprites[id].angle))
			return 1;
	}

	return 0;
}

int8 MainPhysics()
{
	register int16 i, j; 
	int16 k, l, m, pen;
	int16 id, id2, side, sector_id;
	Pos pos, pos2, size, size2;
	int8 collided = 0;
	int32 sety;
	static int32 time;

	memset(&physics.sectorarea, 0, st.Current_Map.num_sector * sizeof(uint16));
	memset(&physics.sectorarea_ids, -1, st.Current_Map.num_sector * physics.num_sprites * sizeof(int16));

	memset(&physics.spritesectornum, 0, physics.num_active_sprites*sizeof(int16));
	memset(&physics.spritesectors, 0, physics.num_active_sprites*st.Current_Map.num_sector*sizeof(int8));

	memset(&physics.proj_list, -1, MAX_PROJECTILES*sizeof(int16));

	memset(&physics.part_list, -1, MAX_PARTICLES*sizeof(int16));

	for (i = 0, j = 0, physics.num_particles = 0; i < MAX_PARTICLES; i++)
	{
		if (physics.particles[i].stat > -1)
		{
			//Add it to the list

			physics.part_list[j] = i;
			j++;
			physics.num_particles++;
		}
	}

	for (i = 0, j = 0, physics.num_proj = 0; i < MAX_PROJECTILES; i++)
	{
		if (physics.proj[i].stat > -1)
		{
			//Add it to the list

			physics.proj_list[j] = i;
			j++;
			physics.num_proj++;
		}
	}

	//Dynamic sprite activation/deactivation and removal

	for (i = 0; i < physics.num_sprites; i++)
	{
		j = physics.sprite_list[i];

		for (k = 0; k < physics.num_active_sprites; k++)
			if (physics.active_sprites[k] == i)
				break;

		if (st.Current_Map.sprites[j].flags & 32)
		{
			if (!CheckBounds(st.Current_Map.sprites[j].position.x, st.Current_Map.sprites[j].position.y, st.Current_Map.sprites[j].body.size.x,
				st.Current_Map.sprites[j].body.size.y, st.Current_Map.sprites[j].position.z))
			{
				DeactivateDynamicSprite(k);
				RemoveDynamicSprite(i);
			}
		}
		else
		if (st.Current_Map.sprites[j].flags & 16)
		{
			if (!CheckBounds(st.Current_Map.sprites[j].position.x, st.Current_Map.sprites[j].position.y, st.Current_Map.sprites[j].body.size.x,
				st.Current_Map.sprites[j].body.size.y, st.Current_Map.sprites[j].position.z))
			{
				ActivateDynamicSprite(i);
				st.Current_Map.sprites[j].flags -= 16;
			}
		}
		else
		{
			if (CheckBounds(st.Current_Map.sprites[j].position.x, st.Current_Map.sprites[j].position.y, st.Current_Map.sprites[j].body.size.x,
				st.Current_Map.sprites[j].body.size.y, st.Current_Map.sprites[j].position.z))
			{
				DeactivateDynamicSprite(k);
				st.Current_Map.sprites[j].flags += 16;
			}
		}
	}
	
	for (i = 0; i < physics.num_active_sprites; i++)
	{
		j = physics.sprite_list[physics.active_sprites[i]];

		UpdateSectorRange(j, i);
	}

	//Particle system

	for (i = 0; i < physics.num_particles; i++)
	{
		j = physics.part_list[i];

		sector_id = UpdateSector(physics.particles[j].pos, physics.particles[j].size);

		(*physics.particles[j].func)(physics.particles[j]);
	}

	//Projectile system

	for (i = 0; i < physics.num_proj; i++)
	{
		j = physics.proj_list[i];

		sector_id = UpdateSector(physics.proj[j].pos, physics.proj[j].size);

		if (physics.proj[j].type & PROJ_BULLET)
		{
			if (Hitscan(physics.proj[j].pos, physics.proj[j].ang, &k, &l,&pos2))
			{
				if (k != -1 || l!=-1)
				{
					if (physics.proj[j].type & PROJ_SPAWNS_SPRITE)
						SpawnSprite(physics.proj[j].spawns, pos2, st.Game_Sprites[physics.proj[j].spawns].body.size, 0);

					if (k!=-1)
						st.Current_Map.sprites[k].body.damage_owner = physics.proj[j].owner;

					physics.proj[j].stat = -1; //Kill the projectile

					continue;
				}
			}
		}
		else
		if (physics.proj[j].type & PROJ_REGULAR)
		{
			if (physics.proj[j].type & PROJ_TRAIL)
			{
				if (physics.proj[j].trail > -1 && physics.proj[j].num_trail > 0)
				{
					if (physics.proj[j].trail_time >= TICSPERSECOND / physics.proj[j].num_trail)
					{
						//SpawnParticle(physics.proj[j].trail, physics.proj[j].pos, 0);

						physics.proj[j].trail_time = 0;
					}
					else
						physics.proj[j].trail_time++;
				}
			}

			if (CheckCollisionSectorWall(physics.proj[j].pos.x, physics.proj[j].pos.y, physics.proj[j].size.x, physics.proj[j].size.y, physics.proj[j].ang)!=-1 || 
				CheckCollisionSector(physics.proj[j].pos.x, physics.proj[j].pos.y, physics.proj[j].size.x, physics.proj[j].size.y, physics.proj[j].ang, NULL, sector_id))
			{
				if (physics.proj[j].type & PROJ_SPAWNS_SPRITE)
					SpawnSprite(physics.proj[j].spawns, physics.proj[j].pos, st.Game_Sprites[physics.proj[j].spawns].body.size, 0);

				physics.proj[j].stat = -1; //Kill the projectile

				continue;
			}

			for (k = 0; k < physics.num_active_sprites; k++)
			{
				l = physics.sprite_list[physics.active_sprites[k]];

				if (l != physics.proj[j].owner)
				{
					if (st.Current_Map.sprites[l].flags & 8 && CheckCollision(physics.proj[j].pos, physics.proj[j].size, physics.proj[j].ang,
						st.Current_Map.sprites[l].position, st.Current_Map.sprites[l].body.size, st.Current_Map.sprites[l].angle))
					{
						if (physics.proj[j].type & PROJ_SPAWNS_SPRITE)
							SpawnSprite(physics.proj[j].spawns, physics.proj[j].pos, st.Game_Sprites[physics.proj[j].spawns].body.size, 0);

						st.Current_Map.sprites[l].body.damage_owner = physics.proj[j].owner;

						physics.proj[j].stat = -1; //Kill the projectile

						break;
					}
				}
			}

			physics.proj[j].pos.x += physics.proj[j].vel.x;
			physics.proj[j].pos.y += physics.proj[j].vel.y;
		}
	}

	//Collision detection

		for (i = 0; i < physics.num_active_sprites; i++)
		{
			
			collided = 0;

			for (j = 0; j < physics.spritesectornum[i]; j++)
			{
				//for (l = 0; l < physics.spritesectors[i][j]; l++)
				//{
					//for (m = 0; m < physics.sectorarea[j]; m++)
					//{
					m = physics.spritesectors[i][j];
					if (physics.sectorarea[m] > 0)
					{
						for (k = 0; k < physics.sectorarea[m]; k++)
						{
							if (k == i)
								continue;

							id = physics.sprite_list[physics.active_sprites[i]];
							id2 = physics.sprite_list[physics.active_sprites[k]];

							if (st.Current_Map.sprites[id].flags & 8 && st.Current_Map.sprites[id2].flags & 8 && (side = CheckCollision(st.Current_Map.sprites[id].position,
								st.Current_Map.sprites[id].body.size, st.Current_Map.sprites[id].angle, st.Current_Map.sprites[id2].position,
								st.Current_Map.sprites[id2].body.size, st.Current_Map.sprites[id2].angle)) > NULL)
							{
								//Check relative position for velocity cancelation

								pos = st.Current_Map.sprites[id].position;
								pos2 = st.Current_Map.sprites[id2].position;
								size = st.Current_Map.sprites[id].body.size;
								size2 = st.Current_Map.sprites[id2].body.size;

								if (pos.y < pos2.y && st.Current_Map.sprites[id].body.velxy.y!=0)
								{
									//st.Current_Map.sprites[id].body.velxy.y = 0;
									st.Current_Map.sprites[id].position.y = pos2.y - (size2.y / 2) - (size.y / 2);

									collided = 1;
								}
								else
								if (pos.y > pos2.y && st.Current_Map.sprites[id].body.velxy.y != 0)
								{
									//st.Current_Map.sprites[id].body.velxy.y = 0;
									st.Current_Map.sprites[id].position.y = pos2.y + (size2.y / 2) + (size.y / 2);

									collided = 1;
								}
								else
								if (pos.x < pos2.x && st.Current_Map.sprites[id].body.velxy.x != 0)
								{
									//st.Current_Map.sprites[id].body.velxy.x = 0;
									st.Current_Map.sprites[id].position.x = pos2.x - (size2.x / 2) - (size.x/2);

									//collided = 1;
								}
								else
								if (pos.x > pos2.x && st.Current_Map.sprites[id].body.velxy.x != 0)
								{
									//st.Current_Map.sprites[id].body.velxy.x = 0;

									st.Current_Map.sprites[id].position.x = pos2.x + (size2.x / 2) + (size.x / 2);

									//collided = 1;
								}
							}
						}
					}
					//}
				//}
			}
			
			//if (collided)
				//continue;
			/*
			if (st.Current_Map.sprites[j].body.total_vel != 0 && st.Current_Map.sprites[j].body.total_vel< MAX_SPEED  && 
				abs(st.Current_Map.sprites[j].body.velxy.x) < MAX_SPEED &&  abs(st.Current_Map.sprites[j].body.velxy.y) < MAX_SPEED)
			{
				st.Current_Map.sprites[j].body.velxy.x = (float)st.Current_Map.sprites[j].body.total_vel * mCos(st.Current_Map.sprites[j].body.ang);
				st.Current_Map.sprites[j].body.velxy.y = (float)st.Current_Map.sprites[j].body.total_vel * mSin(st.Current_Map.sprites[j].body.ang);
			}
			*/
			j = physics.sprite_list[i];

			if (st.Current_Map.sprites[j].body.velxy.x != 0 || st.Current_Map.sprites[j].body.velxy.y != 0)
			{
				st.Current_Map.sprites[j].body.sector_id = UpdateSector(st.Current_Map.sprites[j].position, st.Current_Map.sprites[j].body.size);
			}
				if (st.Current_Map.sprites[j].body.sector_id != -1)
				{
					if (CheckCollisionSector(st.Current_Map.sprites[j].position.x, st.Current_Map.sprites[j].position.y, st.Current_Map.sprites[j].body.size.x,
						st.Current_Map.sprites[j].body.size.y, st.Current_Map.sprites[j].body.ang, &sety, st.Current_Map.sprites[j].body.sector_id))
					{
						if (st.Current_Map.sprites[j].body.velxy.y > 0)
							st.Current_Map.sprites[j].body.velxy.y = 0;

						st.Current_Map.sprites[j].position.y = sety - (st.Current_Map.sprites[j].body.size.y / 2);
					}
					else
					{
						if (physics.gravity && collided != 1 && abs(st.Current_Map.sprites[j].body.velxy.x) < MAX_SPEED &&  abs(st.Current_Map.sprites[j].body.velxy.y) < MAX_SPEED)
						{
							st.Current_Map.sprites[j].body.velxy.x += (float) mCos(physics.gravity_direction) * physics.gravity_vel;
							st.Current_Map.sprites[j].body.velxy.y += (float) mSin(physics.gravity_direction) * physics.gravity_vel;
						}
					}
				}
				else
				{
					if (physics.gravity && collided != 1 && abs(st.Current_Map.sprites[j].body.velxy.x) < MAX_SPEED &&  abs(st.Current_Map.sprites[j].body.velxy.y) < MAX_SPEED)
					{
						st.Current_Map.sprites[j].body.velxy.x += (float)mCos(physics.gravity_direction) * physics.gravity_vel;
						st.Current_Map.sprites[j].body.velxy.y += (float)mSin(physics.gravity_direction) * physics.gravity_vel;
					}
				}
			//}

			if (CheckCollisionSectorWall(st.Current_Map.sprites[j].position.x, st.Current_Map.sprites[j].position.y, st.Current_Map.sprites[j].body.size.x,
				st.Current_Map.sprites[j].body.size.y, st.Current_Map.sprites[j].body.ang)!=-1)
			{
				if (st.Current_Map.sprites[j].body.velxy.x > 0)
					st.Current_Map.sprites[j].body.velxy.x = 0;
			}

			st.Current_Map.sprites[j].position.x += st.Current_Map.sprites[j].body.velxy.x;
			st.Current_Map.sprites[j].position.y += st.Current_Map.sprites[j].body.velxy.y;
		}



	//}
	
}

void DrawMisc()
{
	register uint32 i, j;

	for (i = 0; i < physics.num_proj; i++)
	{
		j = physics.proj_list[i];

		if (physics.proj[j].type & PROJ_REGULAR)
		{
			DrawSprite(physics.proj[j].pos.x, physics.proj[j].pos.y, physics.proj[j].size.x, physics.proj[j].size.y, physics.proj[j].ang,
				255, 255, 255, mgg_game[physics.proj[j].mgg_id].frames[physics.proj[j].frame_id], 255, physics.proj[j].pos.z, 0, 0, 0, 0, 0);
		}
	}
}

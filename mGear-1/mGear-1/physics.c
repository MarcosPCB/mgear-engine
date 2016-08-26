#include <math.h>
#include "physics.h"
#include "engine.h"

Physics physics;

void InitPhysics(int8 gravity, int32 gravity_vel, int16 gravity_direction, int8 air)
{
	int8 tmp = 0, i = 0, k = 4;

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
}

int16 UpdateSector(int16 id)
{
	int32 ydist, ydist2, first = 0, x1, x2, base_y;
	int16 sector_id = -1, i;
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

		st.Current_Map.sprites[id].body.sector_id = UpdateSector(id);
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

int8 MainPhysics()
{
	int32 i, j, k, l, m, sety, pen;
	int16 id, id2, side;
	Pos pos, pos2, size, size2;
	int8 collided = 0;

	memset(&physics.sectorarea, 0, st.Current_Map.num_sector * sizeof(uint16));
	memset(&physics.sectorarea_ids, -1, st.Current_Map.num_sector * physics.num_sprites * sizeof(int16));

	memset(&physics.spritesectornum, 0, physics.num_active_sprites*sizeof(int16));
	memset(&physics.spritesectors, 0, physics.num_active_sprites*st.Current_Map.num_sector*sizeof(int8));

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
								st.Current_Map.sprites[id2].body.size, st.Current_Map.sprites[id2].angle)) > 0)
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

			if (st.Current_Map.sprites[j].body.total_vel != 0 && st.Current_Map.sprites[j].body.total_vel< MAX_SPEED  && 
				abs(st.Current_Map.sprites[j].body.velxy.x) < MAX_SPEED &&  abs(st.Current_Map.sprites[j].body.velxy.y) < MAX_SPEED)
			{
				st.Current_Map.sprites[j].body.velxy.x = (float)st.Current_Map.sprites[j].body.total_vel * mCos(st.Current_Map.sprites[j].body.ang);
				st.Current_Map.sprites[j].body.velxy.y = (float)st.Current_Map.sprites[j].body.total_vel * mSin(st.Current_Map.sprites[j].body.ang);
			}

			j = physics.sprite_list[i];

			if (st.Current_Map.sprites[j].body.velxy.x != 0 || st.Current_Map.sprites[j].body.velxy.y != 0)
			{
				st.Current_Map.sprites[j].body.sector_id = UpdateSector(j);
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
				st.Current_Map.sprites[j].body.size.y, st.Current_Map.sprites[j].body.ang))
			{
				if (st.Current_Map.sprites[j].body.velxy.x > 0)
					st.Current_Map.sprites[j].body.velxy.x = 0;
			}

			st.Current_Map.sprites[j].position.x += st.Current_Map.sprites[j].body.velxy.x;
			st.Current_Map.sprites[j].position.y += st.Current_Map.sprites[j].body.velxy.y;
		}



	//}
	
}

#include "types.h"
#include "engine.h"

#define DEFAULT_GRAVITY 64
#define AIR_RESISTANCE_MAX 10

#define MAX_DYNAMIC_SPRITES 64

struct _Physics
{
	uint8 physics_active;
	uint8 gravity;
	int32 gravity_vel;
	int16 gravity_direction;
	uint8 air;
	int16 sprite_list[MAX_DYNAMIC_SPRITES];
	uint16 num_sprites;
};

typedef struct _Physics Physics;
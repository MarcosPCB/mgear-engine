#include "types.h"

#define GRAVITY 25.0
#define AIR_RESISTANCE_MAX 10.0 

enum Material
{
	METAL,
	WOOD,
	PLASTIC,
	CONCRETE,
	ORGANIC,
	MATERIAL_END
};

struct _BODY
{
	float mass;
	Pos size;
	float max_elasticy;
	Material material;
	uint8 conductor : 2;
	uint8 flamable : 2;
	uint8 explosive : 2;
	Pos position;
	float total_vel;
	Pos velxy;
	float acceleration;
	float energy;
	float temperature;
};

typedef struct _BODY Body;
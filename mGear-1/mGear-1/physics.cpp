#include <math.h>
#include "physics.h"

#ifdef TEST
void Physics(_SPRITES actor)
{
	double layer_y=st.Current_Map.sector[actor.current_sector].Layer[actor.current_layer].position.y;
	double layer_x=st.Current_Map.sector[actor.current_sector].Layer[actor.current_layer].position.x;

	if(actor.body.position.y<layer_y)
		actor.body.total_vel+=GRAVITY;
	else
		
}
#endif
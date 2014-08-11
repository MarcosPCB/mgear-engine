#include <math.h>
#include "physics.h"
#include "engine.h"

#ifdef TEST
void Physics(_SPRITES actor)
{
	Pos Layer, Layer_size;
	float deformation;

	Layer.y=st.Current_Map.sector[actor.current_sector].Layer[actor.current_layer].position.y;
	Layer.x=st.Current_Map.sector[actor.current_sector].Layer[actor.current_layer].position.x;

	Layer.y=st.Current_Map.sector[actor.current_sector].Layer[actor.current_layer].size.y;
	Layer.x=st.Current_Map.sector[actor.current_sector].Layer[actor.current_layer].size.x;

	if(actor.body.position.y<Layer.y)
		actor.body.total_vel+=GRAVITY;
	else
	if(CheckColisionHitbox(actor.body.position,actor.body.size,Layer,Layer_size))
	{
		actor.body.energy=(actor.body.mass*pow(actor.body.total_vel,2))/2;
		deformation=actor.body.energy/actor.body.max_elasticy;

		if(deformation>=actor.body.max_elasticy)
		{
			//Breaks
			actor.health=0;
			//Further coding
		}
		else
		{
			actor.body.energy=(actor.body.max_elasticy*pow(deformation,2))/2;
			actor.body.total_vel=sqrt14((actor.body.energy*2)/actor.body.mass);
			actor.body.ang+=180;
			actor.body.velxy.x=actor.body.total_vel*cos(actor.body.ang);
			actor.body.velxy.y=actor.body.total_vel*sin(actor.body.ang);
		}
	}
}
#endif
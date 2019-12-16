#ifndef space_generator_H
#define space_generator_H


typedef struct {
	float color[3];
	float radius;
	float orbit_speed;
	Vec2 orbit_shape;
	float orbit_distance;
	Vec2 pos;
	float orbit_offset;
	int num_children;
	int parent;
	int index;
	int type;	//0: center of galaxy, 1: star, 2: planet, 3: moon
	float heat; //if type = 1, ie star
} SpaceBody;

typedef struct {
	int *planet_count;
	int *moon_count;
	int star_num;
	int planet_num;
	int moon_num;
	uint bodies_num;
	SpaceBody *bodies;
} Galaxy;


typedef struct {
	int body_location_index;
	int type;
	int ownership;
	int *outputs;
	float output_per_tick;
	float stored;
	float max_storage;
} RGO; //resource gathering operation

/*
	types list
	0: iron mine
	1: carbon condenser
	2: drone hatchery
*/


typedef struct {
	int body_location_index;
	int type;
	int ownership;
	int *inputs;
	int *outputs;
	float *input_rates;
	float *output_rates;
	float *stored;
	float *max_storage;
} Factory;

/*
	types list
	0: toaster manufactory
		iron, fuel -> toasters
	1: fuel refinery
		condensed carbon -> fuel
	2: slave training camp
		drones -> slaves
*/

typedef struct {
	int body_location_index;
	int type;
	int ownership;
	int *inputs;
	float *buy_prices;
} Sink;

/*
	types list
	0: cafe la toast
		toast, slaves -> -
	1: joe's diesel imporium
		fuel -> -
*/

typedef struct {
	RGO *rgos;
	Factory *factories;
	Sink *sinks;
} Economy;



void PopulateSpace(Galaxy *g, float radius, int num_stars, int planet_rolls, float planet_chance, int moon_rolls, float moon_chance, float size_degeneration);
void PopulateEconomy(Economy *e);



#endif


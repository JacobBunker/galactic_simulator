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

void PopulateSpace(Galaxy *g, float radius, int num_stars, int planet_rolls, float planet_chance, int moon_rolls, float moon_chance, float size_degeneration);



#endif


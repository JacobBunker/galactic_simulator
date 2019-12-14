#ifndef econ_simulator_H
#define econ_simulator_H



typedef struct {
	GLfloat x;
	GLfloat y;
} Vec2;

typedef struct {
	int id;
	Vec2 pos;
	double traits[20];
} Player;

typedef struct {
	Vec2 pos;
	float h_height;
	float h_width;
} Square;

typedef struct {
	Square shape;
	float color[3];
	float activation;
} Tile;

typedef struct {
	Square shape;
	int state;
	float unpressed_color[3];
	float pressed_color[3];
} Button;

typedef struct Rock{
	float color[3];
	float radius;
	Vec2 orbit_speed;
	Vec2 orbit_distance;
	Vec2 pos;
	float orbit_offset;
	int num_children;
	struct Rock **children;
} SpaceRock;

typedef struct {
	Player *players;
	Tile *tiles;

	SpaceRock *star;
	SpaceRock **star_list;
	uint star_count;
	GLfloat *allCircleVertices;
	int vertices_per_circle;
	GLuint *c_idxs;
	int c_idxs_size;
	int c_idxs_len;
	GLfloat *c_colors;

	GLfloat *vertex_data;
	int vertex_data_size;

	GLfloat *star_positions;
	int star_positions_size;

	float activation_max;
	float activation_min;

	float timeStep;
	float time;
	int render;
	Button on_button;

} SimulationState;

#endif


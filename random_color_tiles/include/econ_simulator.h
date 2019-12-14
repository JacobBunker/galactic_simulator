#ifndef econ_simulator_H
#define econ_simulator_H



typedef struct {
	float x;
	float y;
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
} Tile;

typedef struct {
	Square shape;
	int state;
	float unpressed_color[3];
	float pressed_color[3];
} Button;

typedef struct {
	Player *players;
	Tile *tiles;

	float timeStep;
	float time;
	int render;
	Button on_button;

} SimulationState;

#endif


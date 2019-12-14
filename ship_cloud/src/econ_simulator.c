#include "helper.h"
#include "space_generator.h"
#include "econ_simulator.h"


#define pi 3.1415926
#define CLEAR_BUFFER 1

#define WINDOW_X_SIZE 1600 //800
#define WINDOW_Y_SIZE 800 //800

//#define OBJECT_SPACE_X 1000.0
//#define OBJECT_SPACE_Y 1000.0

#define TILE_X	32
#define TILE_Y	32

#define CIRCLE_SIDES 4

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

float OBJECT_SPACE_X = 1000.0 * (WINDOW_X_SIZE / 800.0);
float OBJECT_SPACE_Y = 1000.0 * (WINDOW_Y_SIZE / 800.0);


SimulationState render_sim;

char *name;
char buffer[64];


int Check_In_Square(Square s, GLdouble m_x, GLdouble m_y) {
	if(m_x < s.pos.x + s.h_width) {
		if(m_x > s.pos.x - s.h_width) {
			if(m_y < s.pos.y + s.h_height) {
				if(m_y > s.pos.y - s.h_height) {
					return 1;
				}
			}
		}
	}
	return 0;
}

float distance(float x1, float y1, float x2, float y2)
{
    float square_difference_x = (x2 - x1) * (x2 - x1);
    float square_difference_y = (y2 - y1) * (y2 - y1);
    float sum = square_difference_x + square_difference_y;
    float d = sqrt(sum);
    return d;
}



Vec2 GetBodyPositionAtTime(SimulationState *sim, float time, int body_index) {
	Vec2 local = cv2((sim->g.bodies[body_index].orbit_shape.x * sin((time + sim->g.bodies[body_index].orbit_offset) * sim->g.bodies[body_index].orbit_speed)),
					 (sim->g.bodies[body_index].orbit_shape.y * cos((time + sim->g.bodies[body_index].orbit_offset) * sim->g.bodies[body_index].orbit_speed)));
	if(sim->g.bodies[body_index].parent == -1) {
		return local;
	} else {
		Vec2 parent = GetBodyPositionAtTime(sim, time, sim->g.bodies[body_index].parent);
		local.x += parent.x;
		local.y += parent.y;
		return local;
	}
}

void SolveShipPathSimple(SimulationState *sim, int ship_index, int start_body_index, int target_body_index) {
	Vec2 starting_pos = GetBodyPositionAtTime(sim, sim->orbit_time, start_body_index);
	Vec2 target_pos = cv2(0.0, 0.0);
	int growing = 1;
	float tolerance = 0.001;
	float dist;
	float t_step = 0.05;
	float radius = 0.0;
	float t = 0.0;
	int passes = 0;
	while(passes < 1000) {
		radius = t * sim->ships[ship_index].velocity;
		target_pos = GetBodyPositionAtTime(sim, sim->orbit_time + t, target_body_index);
		dist = distance(starting_pos.x, starting_pos.y, target_pos.x, target_pos.y);
		if(fabs(radius - dist) < tolerance) {
			printf("found path with time %f with overshoot %f in %d passes\n", t, (radius - dist), passes);
			break;
		} else {
			if(radius > dist && growing) {
				t_step = (-t_step) * 0.9;
				growing = 0;
			} else if(radius < dist && !growing) {
				t_step = (-t_step) * 0.9;
				growing = 1;
			}
			t += t_step;
		}
		passes++;
	}

	//update the ship
	sim->ships[ship_index].start_time = sim->orbit_time;
	sim->ships[ship_index].journey_time = t;
	sim->ships[ship_index].trajectory = cv2(target_pos.x - starting_pos.x, target_pos.y - starting_pos.y);
	sim->ships[ship_index].source_pos = starting_pos;
	sim->ships[ship_index].destination_pos = target_pos;
}



void GenerateStarShapes(SimulationState *sim) {

	GLint verticesPerCircle = CIRCLE_SIDES + 2;
	GLfloat double_pi = 2.0f * M_PI;

	sim->vertices_per_circle = verticesPerCircle;
	sim->allCircleVertices = malloc(sizeof(GLfloat) * verticesPerCircle * 4);
	sim->c_colors =	malloc(sizeof(GLfloat) * verticesPerCircle * 4);
	sim->star_index = malloc(sizeof(GLuint) * verticesPerCircle);
	sim->star_index_size = sizeof(GLuint)  * verticesPerCircle;
	sim->star_index_len = verticesPerCircle;

	sim->allCircleVertices[0] = 0.0f;
	sim->allCircleVertices[1] = 0.0f;
	sim->allCircleVertices[2] = 0.0f; 
	sim->allCircleVertices[3] = 1.0f; 

	sim->c_colors[0] = ((float)(rand() % 1000)) * 0.001;
	sim->c_colors[1] = ((float)(rand() % 1000)) * 0.001;
	sim->c_colors[2] = ((float)(rand() % 1000)) * 0.001;
	sim->c_colors[3] = 1.0f;

	sim->star_index[0] = 0;

	for(int ii = 1; ii < verticesPerCircle; ++ii) {
		sim->allCircleVertices[(ii * 4) + 0] = 0.0 + (sim->g.bodies[0].radius * cos(ii * double_pi / CIRCLE_SIDES));
		sim->allCircleVertices[(ii * 4) + 1] = 0.0 + (sim->g.bodies[0].radius * sin(ii * double_pi / CIRCLE_SIDES));
		sim->allCircleVertices[(ii * 4) + 2] = 0.0f;
		sim->allCircleVertices[(ii * 4) + 3] = 1.0f;

		sim->c_colors[(ii * 4) + 0] = sim->c_colors[0];
		sim->c_colors[(ii * 4) + 1] = sim->c_colors[1];
		sim->c_colors[(ii * 4) + 2] = sim->c_colors[2];
		sim->c_colors[(ii * 4) + 3] = sim->c_colors[3];

		sim->star_index[ii] = ii;
	}

	printf("creating star_vertex_data\n");
	size_t numBytes = sizeof(GLfloat) * verticesPerCircle * 4;
	printf("numBytes: %ld\n", numBytes);

	sim->star_vertex_data = malloc(2 * numBytes);
	printf("vertex_data malloc'd\n");

	memcpy(&(sim->star_vertex_data[0]), &(sim->allCircleVertices[0]), numBytes);
	printf("allCircleVertices copied\n");

	memcpy(&(sim->star_vertex_data[verticesPerCircle * 4]), &(sim->c_colors[0]), numBytes);
	printf("c_colors copied\n");

	sim->star_vertex_data_size = 2 * numBytes;

	for(int i = 0; i < sim->star_index_len*2; ++i) {
		printf("i: %d star_index: %d  %.1f %.1f %.1f %.1f\n", i, sim->star_index[i%sim->star_index_len], sim->star_vertex_data[(i * 4) + 0],
										sim->star_vertex_data[(i * 4) + 1],
										sim->star_vertex_data[(i * 4) + 2],
										sim->star_vertex_data[(i * 4) + 3]);
	}

}


void InitSim(SimulationState *sim) {
	sim->timeStep = 1.0 / (60.0 * 1.0);
	sim->orbit_step = 1.0 / (6000.0 * 1.0);
	sim->tick_counter = 0;
	sim->render = 1;
	sim->on_button.state = 0;
	sim->on_button.shape.pos = cv2(-OBJECT_SPACE_X + 50.0, -OBJECT_SPACE_Y + 50.0);
	sim->on_button.shape.h_height = 50.0;
	sim->on_button.shape.h_width = 50.0;
	SetColor(sim->on_button.unpressed_color, 0.0, 1.0, 0.0);
	SetColor(sim->on_button.pressed_color, 	 0.0, 0.7, 0.0);

	sim->tiles = malloc(sizeof(Tile) * TILE_X * TILE_Y);

	float x_spacing = (((OBJECT_SPACE_X * 2) - 0) / TILE_X);
	float y_spacing = (((OBJECT_SPACE_Y * 2) - 0) / TILE_Y);

	sim->tile_size_x = x_spacing;
	sim->tile_size_y = y_spacing;

	sim->tile_vertex_data_size = 	sizeof(GLfloat) * 4 * 4;
	sim->tile_positions_size = 		sizeof(GLfloat) * 4 * TILE_X * TILE_Y;
	sim->tile_data_size = 			sizeof(GLfloat) * TILE_X * TILE_Y;
	sim->tile_index_size = 			sizeof(GLuint) * 4;
	sim->tile_index_len = 			4;

	sim->tile_vertex_data = 	malloc(sim->tile_vertex_data_size);
	sim->tile_positions = 		malloc(sim->tile_positions_size);
	sim->tile_data = 			malloc(sim->tile_data_size);
	sim->tile_index =			malloc(sim->tile_index_size);

	sim->tile_vertex_data[(0*4) + 0] = -((x_spacing / 2.0) - 2);
	sim->tile_vertex_data[(0*4) + 1] = -((y_spacing / 2.0) - 2);
	sim->tile_vertex_data[(0*4) + 2] = 0.0f;
	sim->tile_vertex_data[(0*4) + 3] = 1.0f;
	sim->tile_index[0] = 0;

	sim->tile_vertex_data[(1*4) + 0] =  ((x_spacing / 2.0) - 2);
	sim->tile_vertex_data[(1*4) + 1] = -((y_spacing / 2.0) - 2);
	sim->tile_vertex_data[(1*4) + 2] = 0.0f;
	sim->tile_vertex_data[(1*4) + 3] = 1.0f;
	sim->tile_index[1] = 1;

	sim->tile_vertex_data[(2*4) + 0] = -((x_spacing / 2.0) - 2);
	sim->tile_vertex_data[(2*4) + 1] =  ((y_spacing / 2.0) - 2);
	sim->tile_vertex_data[(2*4) + 2] = 0.0f;
	sim->tile_vertex_data[(2*4) + 3] = 1.0f;
	sim->tile_index[2] = 2;

	sim->tile_vertex_data[(3*4) + 0] =  ((x_spacing / 2.0) - 2);
	sim->tile_vertex_data[(3*4) + 1] =  ((y_spacing / 2.0) - 2);
	sim->tile_vertex_data[(3*4) + 2] = 0.0f;
	sim->tile_vertex_data[(3*4) + 3] = 1.0f;
	sim->tile_index[3] = 3;

	for(int i = 0; i < sim->tile_index_len; ++i) {
		printf("i: %d %d -> %.1f %.1f %.1f %.1f\n", i, sim->tile_index[i],
											 sim->tile_vertex_data[(i*4)+0],
											 sim->tile_vertex_data[(i*4)+1],
											 sim->tile_vertex_data[(i*4)+2],
											 sim->tile_vertex_data[(i*4)+3]);
	}

	for(int x = 0; x < TILE_X; ++x) {
		for(int y = 0; y < TILE_Y; ++y) {
			sim->tile_positions[(x*TILE_Y*4) + (y*4) + 0] = ((x_spacing / 2.0) + x * x_spacing) - OBJECT_SPACE_X;
			sim->tile_positions[(x*TILE_Y*4) + (y*4) + 1] = ((y_spacing / 2.0) + y * y_spacing) - OBJECT_SPACE_Y;
			sim->tile_positions[(x*TILE_Y*4) + (y*4) + 2] = 0.0f;
			sim->tile_positions[(x*TILE_Y*4) + (y*4) + 3] = 0.0f;
			printf("tile_positions x %d y %d -> %.2f %.2f %.2f %.2f\n", x, y, sim->tile_positions[(x*TILE_Y*4) + (y*4) + 0],
				sim->tile_positions[(x*TILE_Y*4) + (y*4) + 1],
				sim->tile_positions[(x*TILE_Y*4) + (y*4) + 2],
				sim->tile_positions[(x*TILE_Y*4) + (y*4) + 3]);

			sim->tile_data[(x*TILE_Y)+y] = 0.0f;

			sim->tiles[(x*TILE_Y)+y].shape.pos.x = ((x_spacing / 2.0) + x * x_spacing) - OBJECT_SPACE_X;
			sim->tiles[(x*TILE_Y)+y].shape.pos.y = ((y_spacing / 2.0) + y * y_spacing) - OBJECT_SPACE_Y;
			sim->tiles[(x*TILE_Y)+y].shape.h_width  = (x_spacing / 2.0) - 2;
			sim->tiles[(x*TILE_Y)+y].shape.h_height = (y_spacing / 2.0) - 2;
			sim->tiles[(x*TILE_Y)+y].color[0] = 0.0;//((float)(rand() % 100)) * 0.01;
			sim->tiles[(x*TILE_Y)+y].color[1] = 0.0;//((float)(rand() % 100)) * 0.01;
			sim->tiles[(x*TILE_Y)+y].color[2] = 0.0;//((float)(rand() % 100)) * 0.01;
		}
	}

	int star_range[2] = {200, 300};
	int planet_rolls = 10;
	float planet_chance = 0.2;
	int moon_rolls = 30;
	float moon_chance = 0.1;
	float size_degeneration_range[2] = {0.05, 0.7};
	float initial_radius = 10.0;

	int num_stars = (rand() % (star_range[1] - star_range[0])) + star_range[0];
	PopulateSpace(&(sim->g), initial_radius, num_stars, planet_rolls, planet_chance, moon_rolls, moon_chance, size_degeneration_range[1]);
	GenerateStarShapes(sim);

	sim->star_positions_size = sizeof(GLfloat) * 4 * sim->g.bodies_num;
	sim->star_positions = malloc(sim->star_positions_size);
	for(int i = 0; i < sim->g.bodies_num; ++i) {
		sim->star_positions[(i*4)+0] = sim->g.bodies[i].pos.x;
		sim->star_positions[(i*4)+1] = sim->g.bodies[i].pos.y;
		sim->star_positions[(i*4)+2] = 0.0f;
		sim->star_positions[(i*4)+3] = 0.0f;
	}
	sim->star_colors_size = sizeof(GLfloat) * 4 * sim->g.bodies_num;
	sim->star_colors = malloc(sim->star_colors_size);
	for(int i = 0; i < sim->g.bodies_num; ++i) {
		sim->star_colors[(i*4)+0] = sim->g.bodies[i].color[0];
		sim->star_colors[(i*4)+1] = sim->g.bodies[i].color[1];
		sim->star_colors[(i*4)+2] = sim->g.bodies[i].color[2];
		sim->star_colors[(i*4)+3] = 1.0f;
	}

	sim->ship_count = 100000;
	sim->ships = 					malloc(sizeof(Ship) * sim->ship_count);
	sim->ship_positions_data_size = sizeof(float) * 4 * sim->ship_count;
	sim->ship_positions_data = 		malloc(sim->ship_positions_data_size);
	sim->ship_colors_size = 		sizeof(float) * 4 * sim->ship_count;
	sim->ship_colors = 				malloc(sim->ship_colors_size);
	sim->ship_index_size = 			sizeof(GLuint) * sim->ship_count;
	sim->ship_index = 				malloc(sim->ship_index_size);

	for(int i = 0; i < sim->ship_count; ++i) {
		sim->ships[i].index = i;
		sim->ships[i].source = rand() % sim->g.bodies_num;
		sim->ships[i].destination = rand() % sim->g.bodies_num;
		sim->ships[i].velocity = ((float) (rand() % 1000));
		sim->ships[i].status = 2;
		sim->ships[i].journey_time = ((float) (rand() % 1000)) * 0.001;

		sim->ship_positions_data[(i*4)+0] = 0.0;
		sim->ship_positions_data[(i*4)+1] = 0.0;
		sim->ship_positions_data[(i*4)+2] = 0.0;
		sim->ship_positions_data[(i*4)+3] = 1.0;

		sim->ship_colors[(i*4) + 0] = ((sim->ships[i].velocity + 100) / (1100));
		sim->ship_colors[(i*4) + 1] = ((sim->ships[i].velocity + 10) / (10010));
		sim->ship_colors[(i*4) + 2] = ((sim->ships[i].velocity + 10) / (10010));
		sim->ship_colors[(i*4) + 3] = 1.0f;
		sim->ship_index[i] = i;
	}
}

void FreeSim(SimulationState *sim) {
	free(sim->tiles);
}


void init(SimulationState *sim) {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	glEnable( GL_BLEND );
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glColor3f(0.0, 0.0, 1.0);

	glm_ortho(-OBJECT_SPACE_X, OBJECT_SPACE_X, -OBJECT_SPACE_Y, OBJECT_SPACE_Y, -10.0, 10.0, MVP);

	InitializeShipProgram();
	InitializeStarProgram();
	InitializeTileProgram();
	InitializeShipVAO(sim);
	InitializeStarVAO(sim);
	InitializeTileVAO(sim);

	glMatrixMode(GL_PROJECTION);	
	glLoadIdentity();
	glOrtho(-OBJECT_SPACE_X, OBJECT_SPACE_X, -OBJECT_SPACE_Y, OBJECT_SPACE_Y, -10.0, 10.0);

}
  
void timer(int value) {
    glutTimerFunc(16, timer, 0);
    glutPostRedisplay();
}

void DrawSquare(float color[3], Vec2 pos, float h_height, float h_width) {
	glColor4f(color[0], color[1], color[2], 0.3f);
	glVertex3f(pos.x + h_width, pos.y + h_height, 1.0);
	glVertex3f(pos.x - h_width, pos.y + h_height, 1.0);
	glVertex3f(pos.x - h_width, pos.y - h_height, 1.0);
	glVertex3f(pos.x + h_width, pos.y - h_height, 1.0);
}

void DrawStars(SimulationState *sim) {

	/*printf("copying star positions to buffer...\n");
	for(int i = 0; i < sim->g.bodies_num; ++i) {
		printf("star at index %d has pos %.2f %.2f %.2f %.2f\n", i, sim->star_positions[(i*4)+0],
																	sim->star_positions[(i*4)+1],
																	sim->star_positions[(i*4)+2],
																	sim->star_positions[(i*4)+3]);
	} */

	glBindBuffer(GL_ARRAY_BUFFER, starOffsetBufferObject);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sim->star_positions_size, &(sim->star_positions[0]));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(starProgram);
	glBindVertexArray(star_VAO);

	glDrawElementsInstanced(GL_TRIANGLE_FAN, sim->vertices_per_circle, GL_UNSIGNED_INT, BUFFER_OFFSET(0), sim->g.bodies_num);

	glBindVertexArray(0);
	glUseProgram(0);
}

void DrawTiles(SimulationState *sim) {

	glBindBuffer(GL_ARRAY_BUFFER, tileActivationBufferObject);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sim->tile_data_size, &(sim->tile_data[0]));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(tileProgram);
	glUniform2f(tileMinMaxUniform, sim->activation_min, sim->activation_max);
	glBindVertexArray(tile_VAO);
	glDrawElementsInstanced(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, BUFFER_OFFSET(0), TILE_X*TILE_Y);
	glBindVertexArray(0);
	glUseProgram(0);
	//	temp->color[0] = (temp->activation - render_sim.activation_min) / (render_sim.activation_max - render_sim.activation_min);

}

void DrawShips(SimulationState *sim) {
	glBindBuffer(GL_ARRAY_BUFFER, shipVertexBufferObject);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sim->ship_positions_data_size, &(sim->ship_positions_data[0]));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(shipProgram);
	glBindVertexArray(ship_VAO);

	glDrawElements(GL_POINTS, sim->ship_count, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

	glBindVertexArray(0);
	glUseProgram(0);
}

int RecursiveDrawBodyLines(SimulationState *sim, float time, int current, float parentX, float parentY, int level) {
	int leap = 1;
	glVertex3f(parentX, parentY, 1.0f);
	glVertex3f(sim->g.bodies[current].pos.x, sim->g.bodies[current].pos.y, 1.0f);
	for(int i = 0; i < sim->g.bodies[current].num_children; ++i) {
		leap += RecursiveDrawBodyLines(sim, time, current + leap, sim->g.bodies[current].pos.x, sim->g.bodies[current].pos.y, level+1);
	}

	return leap;
}

void StepShips(SimulationState *sim) {
	float progress;
	for(int i = 0; i < sim->ship_count; ++i) {
		if(sim->ships[i].status == 1) {
			progress = (sim->orbit_time - sim->ships[i].start_time) / sim->ships[i].journey_time;
			sim->ship_positions_data[(i*4)+0] = sim->ships[i].source_pos.x + (sim->ships[i].trajectory.x * progress * 1.00);
			sim->ship_positions_data[(i*4)+1] = sim->ships[i].source_pos.y + (sim->ships[i].trajectory.y * progress * 1.00);
			sim->ship_positions_data[(i*4)+2] = 0.0;
			sim->ship_positions_data[(i*4)+3] = 1.0;
			//glVertex3f(sim->ships[i].source_pos.x + (sim->ships[i].trajectory.x * progress * 1.00), sim->ships[i].source_pos.y + (sim->ships[i].trajectory.y * progress * 1.00), 0.0f);
		} else {
			sim->ship_positions_data[(i*4)+3] = 0.0;
		}
	}
}

void GetBodyTileActivation(SimulationState *sim, float pos_x, float pos_y) {
	int x = 0;
	int y = 0;
	int search = 1;

	if(pos_x < -OBJECT_SPACE_X || pos_x > OBJECT_SPACE_X) {
		return; //point outside of tile grid
	}

	if(pos_y < -OBJECT_SPACE_Y || pos_y > OBJECT_SPACE_Y) {
		return; //point outside of tile grid
	}

	while(search) {
		if(pos_x < -OBJECT_SPACE_X + sim->tile_size_x) {
			search = 0;
		} else {
			pos_x = pos_x - sim->tile_size_x;
			x++;
		}
	}

	search = 1;

	while(search) {
		if(pos_y < -OBJECT_SPACE_Y + sim->tile_size_y) {
			search = 0;
		} else {
			pos_y = pos_y - sim->tile_size_y;
			y++;
		}
	}

	sim->tiles[(x*TILE_Y)+y].activation += 1;
}

int RecursiveStepBody(SimulationState *sim, float time, int current, float parentX, float parentY, int level) {
	int leap = 1;
	sim->g.bodies[current].pos.x = parentX + (sim->g.bodies[current].orbit_shape.x * sin((time + sim->g.bodies[current].orbit_offset) * sim->g.bodies[current].orbit_speed));
	sim->g.bodies[current].pos.y = parentY + (sim->g.bodies[current].orbit_shape.y * cos((time + sim->g.bodies[current].orbit_offset) * sim->g.bodies[current].orbit_speed));
	for(int i = 0; i < sim->g.bodies[current].num_children; ++i) {
		leap += RecursiveStepBody(sim, time, current + leap, sim->g.bodies[current].pos.x, sim->g.bodies[current].pos.y, level+1);
	}

	return leap;
}


void ProcessTileActivation(SimulationState *sim) {
	sim->activation_min = 1000000.0;
	sim->activation_max = 0;
	for(int x = 0; x < TILE_X; ++x) {
		for(int y = 0; y < TILE_Y; ++y) {
			sim->tile_data[(x*TILE_Y)+y] = sim->tiles[(x*TILE_Y)+y].activation;
			if(sim->tiles[(x*TILE_Y)+y].activation < sim->activation_min) {
				sim->activation_min = sim->tiles[(x*TILE_Y)+y].activation;
			}
			if(sim->tiles[(x*TILE_Y)+y].activation > sim->activation_max) {
				sim->activation_max = sim->tiles[(x*TILE_Y)+y].activation;
			}
			sim->tiles[(x*TILE_Y)+y].activation = 0;
		}
	}
	if(sim->activation_min == sim->activation_max) {
		sim->activation_min = 0;
		sim->activation_max = 1000000.0;
	} 
}

void ProcessShipActivity(SimulationState *sim) {
	for(int i = 0; i < sim->ship_count; ++i) {
		if(sim->ships[i].status == 0) {
			//ship is prepped, lets get it moving!
			sim->ships[i].status = 1;
			sim->ships[i].destination = rand() % sim->g.bodies_num;
			printf("ship %d is being charted from body %d to body %d\n", i, sim->ships[i].source, sim->ships[i].destination);
			SolveShipPathSimple(sim, i, sim->ships[i].source, sim->ships[i].destination);
		} else if (sim->ships[i].status == 1) {
			//ship is moving, update it!
			if(sim->orbit_time - sim->ships[i].start_time >= sim->ships[i].journey_time) {
				//ship has reached the end of it's journey
				//printf("ship %d has reached body %d\n", i, sim->ships[i].destination);
				sim->ships[i].status = 2;
				sim->ships[i].start_time = sim->orbit_time;
				sim->ships[i].journey_time = sim->ships[i].journey_time * 0.1; //let resupply be 1/10th the previous journey time
				sim->ships[i].source = sim->ships[i].destination;
			}
		} else if (sim->ships[i].status == 2) {
			//ship is resupplying, check if it's ready to launch
			if(sim->orbit_time - sim->ships[i].start_time >= sim->ships[i].journey_time) {
				sim->ships[i].status = 0;
			}
		}
	}
}


void ResetSim(SimulationState *sim) {
	sim->time = 0.0;
}

void StepSim(SimulationState *sim) {
	RecursiveStepBody(sim, sim->orbit_time, 0, 0.0, 0.0, 0);
	for(int i = 0; i < sim->g.bodies_num; ++i) {
			sim->star_positions[(i*4)+0] = sim->g.bodies[i].pos.x;
			sim->star_positions[(i*4)+1] = sim->g.bodies[i].pos.y;
			sim->star_positions[(i*4)+2] = 0.0f;
			sim->star_positions[(i*4)+3] = sim->g.bodies[i].radius / 10.0;
			/*printf("body %d pos: %.2f %.2f %.2f %.2f\n", i, sim->star_positions[(i*4)+0],
															sim->star_positions[(i*4)+1],
															sim->star_positions[(i*4)+2],
															sim->star_positions[(i*4)+3]); */
	}

	for(int i = 0; i < sim->g.bodies_num; ++i) {
		GetBodyTileActivation(sim, sim->star_positions[(i*4)+0], sim->star_positions[(i*4)+1]);
	}

	ProcessTileActivation(sim);
	ProcessShipActivity(sim);
	StepShips(sim);

	sim->orbit_time += sim->orbit_step;
}

int on = 1;

void display(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	if(CLEAR_BUFFER) {
		glClear(GL_COLOR_BUFFER_BIT);
	}


	//glBegin(GL_LINES);   
	if(on) {
		StepSim(&render_sim);
		render_sim.time += render_sim.timeStep;
		//sleep(sim.timeStep);
		snprintf(buffer, sizeof buffer, "%f", render_sim.orbit_time);
		glutSetWindowTitle(buffer);
		glColor3f(0.6, 0.3, 0.0);

		//DrawTiles(&render_sim);

		DrawShips(&render_sim);

		//glBegin(GL_LINES);
		//RecursiveDrawBodyLines(&render_sim, render_sim.time, 0, 0.0, 0.0, 0);
		//glEnd();

		DrawStars(&render_sim); 

		glBegin(GL_QUADS);
		Button *b_temp = &render_sim.on_button;

		if(b_temp->state == 0) {
			DrawSquare(b_temp->unpressed_color, b_temp->shape.pos, b_temp->shape.h_height, b_temp->shape.h_width);
		} else {
			DrawSquare(b_temp->pressed_color, b_temp->shape.pos, b_temp->shape.h_height, b_temp->shape.h_width);
		}

		glEnd(); 
		glutSwapBuffers();

	} else if(render_sim.render == 1){
		char *name = "SIMULATION ENDED";
		glutSetWindowTitle(name);
		ResetSim(&render_sim);
	}  
}

void keyboard(unsigned char key, int x, int y)
{
	/* this is the keyboard event handler
	   the x and y parameters are the mouse 
	   coordintes when the key was struck */
	switch (key)
	{
	case 'u':
	case 'U':
		render_sim.on_button.state = !(render_sim.on_button.state);
		break;
	}

	display(); /* repaint the window */
}

void mouse(int button, int state, int x, int y) {

	//x = (WINDOW_X_SIZE - x - 1);
	//y = (WINDOW_Y_SIZE - y - 1);

	GLdouble b_x, b_y, b_z;
	GLdouble * model = malloc(sizeof(GLdouble) * 16);
	GLdouble * proj = malloc(sizeof(GLdouble) * 16);
	GLint * view = malloc(sizeof(GLint) * 4);

	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, view);
	gluUnProject(x, y, 0.0, model, proj, view, &b_x, &b_y, &b_z);

	b_y = -b_y;

	switch (state) {
		case GLUT_UP:
			printf("b_x: %f, b_y: %f\n", b_x, b_y);
			if(Check_In_Square(render_sim.on_button.shape, b_x, b_y)) {
				render_sim.on_button.state = !(render_sim.on_button.state );
			}
			break;
		case GLUT_DOWN:
			break;
	}

	free(model);
	free(proj);
	free(view);
}

void reshape (int w, int h)
{
	printf("window reshaped: %d %d\n", w, h);
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}


int main(int argc, char** argv) {
	srand(time(0));
	InitSim(&render_sim);

	printf("SIM INITIATED\n");

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(WINDOW_X_SIZE, WINDOW_Y_SIZE);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Simulator");

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		/* Problem: glewInit failed, something is seriously wrong. */
		printf("Error: %s\n", glewGetErrorString(err));
		return 1;
	} else {
		if (GLEW_VERSION_1_3)
		{
		  /* Yay! OpenGL 1.3 is supported! */
			printf("Yay! OpenGL 1.3 is supported!\n");
		}
		if (GLEW_VERSION_2_1)
		{
		  /* Yay! OpenGL 1.3 is supported! */
			printf("Yay! OpenGL 2.0 is supported!\n");
		}
		if (GLEW_VERSION_3_1)
		{
		  /* Yay! OpenGL 1.3 is supported! */
			printf("Yay! OpenGL 3.1 is supported!\n");
		}
		else {
			return 1;
		}
	}
	printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));
	init(&render_sim);
	glutTimerFunc(0, timer, 0);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);  /* set keyboard handler */
	glutMouseFunc(mouse);
	glutMainLoop();

	return 0;
}

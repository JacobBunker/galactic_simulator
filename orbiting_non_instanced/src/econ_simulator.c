#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <bsd/stdlib.h>
#include <time.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <unistd.h>

#include <cglm/cglm.h>

#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>

#include <pthread.h>

#include "econ_simulator.h"

#define pi 3.1415926
#define NUM_NODES 20000
#define CLEAR_BUFFER 1
#define DRAW_POINTS 20000
#define DRAW_LINES 0

#define WINDOW_X_SIZE 800
#define WINDOW_Y_SIZE 800

#define OBJECT_SPACE_X 1000.0
#define OBJECT_SPACE_Y 1000.0

#define TILE_X	32
#define TILE_Y	32

#define CIRCLE_SIDES 6

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

GLuint PRIMITIVE_RESTART = 987654321;


SimulationState render_sim;

char *name;
char buffer[64];
Vec2 spawns[NUM_NODES];

Vec2 cpv(float a, float b) {
	Vec2 out;
	out.x = a;
	out.y = b;
	return out;
}

void SetColor(float *color, float a, float b, float c) {
	color[0] = a;
	color[1] = b;
	color[2] = c;
}

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

int count = 0;

void GenStarListHelper(SimulationState *sim, SpaceRock *star) {
	sim->star_list[count] = star;
	count++;
	for(int i = 0; i < star->num_children; ++i) {
		GenStarListHelper(sim, star->children[i]);
	}
}

void GenerateStarList(SimulationState *sim, SpaceRock *star) {
	count = 0;
	GenStarListHelper(sim, star);
}

void GenerateStarShapes(SimulationState *sim) {

	GLint verticesPerCircle = CIRCLE_SIDES + 2;
	GLfloat double_pi = 2.0f * M_PI;

	sim->vertices_per_circle = verticesPerCircle;
	sim->allCircleVertices = 	malloc(sizeof(GLfloat) * verticesPerCircle * 4   * sim->star_count);
	sim->c_colors = 			malloc(sizeof(GLfloat) * verticesPerCircle * 4   * sim->star_count);
	sim->c_idxs = 				malloc(sizeof(GLuint)  * verticesPerCircle * sim->star_count);
	sim->c_idxs_size = sizeof(GLuint)  * verticesPerCircle * sim->star_count;
	sim->c_idxs_len = verticesPerCircle * sim->star_count;

	for(int i = 0; i < sim->star_count; ++i) {
		sim->allCircleVertices[(i * verticesPerCircle * 4) + 0] = 0.0f;
		sim->allCircleVertices[(i * verticesPerCircle * 4) + 1] = 0.0f;
		sim->allCircleVertices[(i * verticesPerCircle * 4) + 2] = 0.0f; 
		sim->allCircleVertices[(i * verticesPerCircle * 4) + 3] = 1.0f; 

		sim->c_colors[(i * verticesPerCircle * 4) + 0] = ((float)(rand() % 1000)) * 0.001;
		sim->c_colors[(i * verticesPerCircle * 4) + 1] = ((float)(rand() % 1000)) * 0.001;;
		sim->c_colors[(i * verticesPerCircle * 4) + 2] = ((float)(rand() % 1000)) * 0.001;;
		sim->c_colors[(i * verticesPerCircle * 4) + 3] = 1.0f;

		
		sim->c_idxs[(i * verticesPerCircle)] = (i * verticesPerCircle) + 0;

		for(int ii = 1; ii < verticesPerCircle; ++ii) {
			sim->allCircleVertices[(i * verticesPerCircle * 4) + (ii * 4) + 0] = 0.0 + (sim->star_list[i]->radius * cos(ii * double_pi / CIRCLE_SIDES));
			sim->allCircleVertices[(i * verticesPerCircle * 4) + (ii * 4) + 1] = 0.0 + (sim->star_list[i]->radius * sin(ii * double_pi / CIRCLE_SIDES));
			sim->allCircleVertices[(i * verticesPerCircle * 4) + (ii * 4) + 2] = 0.0f;
			sim->allCircleVertices[(i * verticesPerCircle * 4) + (ii * 4) + 3] = 1.0f;

			sim->c_colors[(i * verticesPerCircle * 4) + (ii * 4) + 0] = sim->c_colors[(i * verticesPerCircle * 4) + 0];
			sim->c_colors[(i * verticesPerCircle * 4) + (ii * 4) + 1] = sim->c_colors[(i * verticesPerCircle * 4) + 1];
			sim->c_colors[(i * verticesPerCircle * 4) + (ii * 4) + 2] = sim->c_colors[(i * verticesPerCircle * 4) + 2];
			sim->c_colors[(i * verticesPerCircle * 4) + (ii * 4) + 3] = sim->c_colors[(i * verticesPerCircle * 4) + 3];

			sim->c_idxs[(i * verticesPerCircle) + ii] = (i * verticesPerCircle) + ii;
		}
	}

	printf("creating vertex_data\n");
	size_t numBytes = sizeof(GLfloat) * verticesPerCircle * 4   * sim->star_count;
	printf("numBytes: %ld\n", numBytes);

	sim->vertex_data = malloc(2 * numBytes);
	printf("vertex_data malloc'd\n");

	memcpy(&(sim->vertex_data[0]), &(sim->allCircleVertices[0]), numBytes);
	printf("allCircleVertices copied\n");

	memcpy(&(sim->vertex_data[verticesPerCircle * 4  * sim->star_count]), &(sim->c_colors[0]), numBytes);
	printf("c_colors copied\n");

	sim->vertex_data_size = 2 * numBytes;

	for(int i = 0; i < sim->c_idxs_len*2; ++i) {
		printf("i: %d c_idxs: %d  %.1f %.1f %.1f %.1f\n", i, sim->c_idxs[i], sim->vertex_data[(i * 4) + 0],
										sim->vertex_data[(i * 4) + 1],
										sim->vertex_data[(i * 4) + 2],
										sim->vertex_data[(i * 4) + 3]);
	}

}


int PopulateSpaceRock(SpaceRock *star, int num_children, int children_distance_factor, int moon_rolls, float moon_chance, float size_degeneration_range[2]) {
	star->children = malloc(sizeof(SpaceRock *) * star->num_children);
	star->num_children = num_children;
	int moons = 0;
	int out = 0;
	for(int i = 0; i < num_children; ++i) {
		star->children[i] = malloc(sizeof(SpaceRock));
		star->children[i]->radius = star->radius * size_degeneration_range[1];
		star->children[i]->orbit_speed = cpv((float)((random() % 200) - 100)*0.01, (float)((random() % 200) - 100)*0.01);
		star->children[i]->orbit_distance = cpv((((float)(random() % 1000)) * pow(0.1, children_distance_factor)+star->radius),
											  (((float)(random() % 1000)) * pow(0.1, children_distance_factor)+star->radius));
		star->children[i]->orbit_offset = ((float)(random() % 10000) * 0.01);
		SetColor(star->children[i]->color, ((float)(rand() % 100)) * 0.01, ((float)(rand() % 100)) * 0.01, ((float)(rand() % 100)) * 0.01);
		moons = 0;
		for(int roll = 0; roll < moon_rolls; ++roll) {
			if(((float)(rand() % 1000) * 0.001) <= moon_chance) {
				moons++;
			}
		}
		if(moons > 0) {
			//printf("child %d on level %d generated %d moons\n", i, children_distance_factor, moons);
		}
		out += PopulateSpaceRock(star->children[i], moons, children_distance_factor + 1, moon_rolls - 5, moon_chance, size_degeneration_range);
	}
	return 1 + out;
}

void InitSim(SimulationState *sim) {
	sim->timeStep = 1.0 / (60.0 * 1.0);
	sim->render = 1;
	sim->on_button.state = 0;
	sim->on_button.shape.pos = cpv(-950.0, -950.0);
	sim->on_button.shape.h_height = 50.0;
	sim->on_button.shape.h_width = 50.0;
	SetColor(sim->on_button.unpressed_color, 0.0, 1.0, 0.0);
	SetColor(sim->on_button.pressed_color, 	 0.0, 0.7, 0.0);

	sim->players = malloc(sizeof(Player) * NUM_NODES);
	sim->tiles = malloc(sizeof(Tile) * TILE_X * TILE_Y);

	float x_spacing = (((OBJECT_SPACE_X * 2) - 0) / TILE_X);
	float y_spacing = (((OBJECT_SPACE_Y * 2) - 0) / TILE_Y);

	for(int x = 0; x < TILE_X; ++x) {
		for(int y = 0; y < TILE_Y; ++y) {
			sim->tiles[(x*TILE_Y)+y].shape.pos.x = ((x_spacing / 2.0) + x * x_spacing) - OBJECT_SPACE_X;
			sim->tiles[(x*TILE_Y)+y].shape.pos.y = ((y_spacing / 2.0) + y * y_spacing) - OBJECT_SPACE_Y;
			sim->tiles[(x*TILE_Y)+y].shape.h_width  = (x_spacing / 2.0) - 2;
			sim->tiles[(x*TILE_Y)+y].shape.h_height = (y_spacing / 2.0) - 2;
			sim->tiles[(x*TILE_Y)+y].color[0] = 0.0;//((float)(rand() % 100)) * 0.01;
			sim->tiles[(x*TILE_Y)+y].color[1] = 0.0;//((float)(rand() % 100)) * 0.01;
			sim->tiles[(x*TILE_Y)+y].color[2] = 0.0;//((float)(rand() % 100)) * 0.01;
			/*printf("color: %f %f %f\n", sim->tiles[(x*TILE_Y)+y].color[0],
				sim->tiles[(x*TILE_Y)+y].color[1],
				sim->tiles[(x*TILE_Y)+y].color[2]); */
		}
	}

	int planetoid_range[2] = {10, 20};
	int moon_rolls = 10;
	float moon_chance = 0.3;
	float size_degeneration_range[2] = {0.05, 0.7};

	int num_planetoids = (rand() % (planetoid_range[1] - planetoid_range[0])) + planetoid_range[0];

	sim->star = malloc(sizeof(SpaceRock));
	sim->star->radius = 10.0;
	sim->star->orbit_speed = cpv(0.0, 0.0);
	sim->star->orbit_distance = cpv(0.0, 0.0);
	sim->star->orbit_offset = 0.0;
	SetColor(sim->star->color, 1.0f, 1.0f, 1.0f);
	sim->star->num_children = num_planetoids;
	printf("star generated %d planetoids\n", num_planetoids);


	sim->star_count = PopulateSpaceRock(sim->star, num_planetoids, 0, moon_rolls, moon_chance, size_degeneration_range);
	sim->star_list = malloc(sizeof(SpaceRock *) * sim->star_count);
	GenerateStarList(sim, sim->star);
	GenerateStarShapes(sim);
	sim->star_positions_size = sizeof(GLfloat) * 4 * sim->vertices_per_circle * sim->star_count;
	sim->star_positions = malloc(sim->star_positions_size);
	for(int i = 0; i < sim->star_count; ++i) {
		for(int ii = 0; ii < sim->vertices_per_circle; ++ii) {
			sim->star_positions[(i*sim->vertices_per_circle*4)+(ii*4)+0] = sim->star_list[i]->pos.x;
			sim->star_positions[(i*sim->vertices_per_circle*4)+(ii*4)+1] = sim->star_list[i]->pos.y;
			sim->star_positions[(i*sim->vertices_per_circle*4)+(ii*4)+2] = 0.0f;
			sim->star_positions[(i*sim->vertices_per_circle*4)+(ii*4)+3] = 0.0f;

		}
	}
	//UpdateStarShapes(sim);
}

void FreeSim(SimulationState *sim) {
	free(sim->players);
	free(sim->tiles);
}


GLuint theProgram;
GLuint offsetAttrib;
GLuint positionAttrib;
GLuint colorAttrib;
GLuint elapsedTimeUniform;
GLuint positionMove;

const GLchar v_shader[] = "#version 330\nlayout(location = 0) in vec4 position;layout(location = 1) in vec4 color;layout(location = 2) in vec4 offset;smooth out vec4 theColor;uniform mat4 projection;uniform vec2 move;void main() {vec4 totalOffset = position + offset;gl_Position = projection * totalOffset; theColor=color;}";
const GLchar f_shader[] = "#version 330\nsmooth in vec4 theColor; out vec4 outputColor;uniform float fragLoopDuration;uniform float time;void main(){outputColor = theColor;}";

void InitializeProgram() {
	GLint log_size, success;
	GLchar *error_msg;

	GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);

	GLchar const **source = malloc(sizeof(char *) * 1);
	source[0] = v_shader;	
	glShaderSource(VertexShader, 1, source, NULL);
	glCompileShader(VertexShader);


	success = 0;
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &success);

	if(success == GL_FALSE) {
		printf("VERTEX SHADER FAILED TO COMPILE!\n");
		log_size = 0;
		glGetShaderiv(VertexShader, GL_INFO_LOG_LENGTH, &log_size);
		error_msg = malloc(sizeof(GLchar) * log_size);
		glGetShaderInfoLog(VertexShader, log_size, &log_size, &error_msg[0]);
		printf("%s\n", error_msg);
		free(error_msg);
		glDeleteShader(VertexShader);
		return;
	}
	else {
		printf("VERTEX SHADER COMPILED SUCCESSFULLY!\n");
	}

	GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	source[0] = f_shader;	
	glShaderSource(FragmentShader, 1, source, NULL);
	glCompileShader(FragmentShader);

	success = 0;
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &success);

	if(success == GL_FALSE) {
		printf("FRAG SHADER FAILED TO COMPILE!\n");
		log_size = 0;
		glGetShaderiv(FragmentShader, GL_INFO_LOG_LENGTH, &log_size);
		error_msg = malloc(sizeof(GLchar) * log_size);
		glGetShaderInfoLog(FragmentShader, log_size, &log_size, &error_msg[0]);
		printf("%s\n", error_msg);
		free(error_msg);
		glDeleteShader(FragmentShader);
		return;
	}
	else {
		printf("FRAG SHADER COMPILED SUCCESSFULLY!\n");
	}

	//shaders compiled successfully
	theProgram = glCreateProgram();
	glAttachShader(theProgram, VertexShader);
	glAttachShader(theProgram, FragmentShader);
	glLinkProgram(theProgram);

	GLint isLinked = 0;
	glGetProgramiv(theProgram, GL_LINK_STATUS, (int *)&isLinked);
	if(isLinked == GL_FALSE) {
		printf("SHADER PROGRAM FAILED TO LINK!\n");

		log_size = 0;
		glGetProgramiv(theProgram, GL_INFO_LOG_LENGTH, &log_size);
		error_msg = malloc(sizeof(GLchar) * log_size);
		glGetProgramInfoLog(theProgram, log_size, &log_size, &error_msg[0]);

		printf("%s\n", error_msg);
		free(error_msg);

		glDeleteProgram(theProgram);
		glDeleteShader(FragmentShader);
		glDeleteShader(VertexShader);

		return;
	} else {
		printf("SHADER PROGRAM SUCCESSFULLY LINKED!\n");
	}

	glDetachShader(theProgram, VertexShader);
	glDetachShader(theProgram, FragmentShader);

	positionAttrib = glGetAttribLocation(theProgram, "position");
	colorAttrib = glGetAttribLocation(theProgram, "color");
	offsetAttrib = glGetAttribLocation(theProgram, "offset");


	elapsedTimeUniform = glGetUniformLocation(theProgram, "time");
	positionMove = glGetUniformLocation(theProgram, "move");
	GLuint projection = glGetUniformLocation(theProgram, "projection");
	GLuint loopDurationUnf = glGetUniformLocation(theProgram, "loopDuration");
	GLuint fragLoopDurUnf = glGetUniformLocation(theProgram, "fragLoopDuration");

	glUseProgram(theProgram);
	glUniform1f(loopDurationUnf, 5.0f);
	glUniform1f(fragLoopDurUnf, 10.0f);
	mat4 MVP;
	glm_ortho(-OBJECT_SPACE_X, OBJECT_SPACE_X, -OBJECT_SPACE_Y, OBJECT_SPACE_Y, -10.0, 10.0, MVP);
	glUniformMatrix4fv(projection, 1, GL_FALSE, &MVP[0][0]);
	glUseProgram(0);

}

GLuint offsetBufferObject;
GLuint vertexBufferObject;
GLuint indexBufferObject;
GLuint vao;

void InitializeVAO(SimulationState *sim)
{
	glGenBuffers(1, &vertexBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sim->vertex_data_size, sim->vertex_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &offsetBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, offsetBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sim->star_positions_size, sim->star_positions, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &indexBufferObject);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sim->c_idxs_size, sim->c_idxs, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	size_t colorDataOffset = sizeof(float) * 4 * sim->c_idxs_len;
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glEnableVertexAttribArray(positionAttrib);
	glVertexAttribPointer(positionAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(colorAttrib);
	glVertexAttribPointer(colorAttrib, 4, GL_FLOAT, GL_FALSE, 0, (void*)colorDataOffset);

	glBindBuffer(GL_ARRAY_BUFFER, offsetBufferObject);
	glEnableVertexAttribArray(offsetAttrib);
	glVertexAttribPointer(offsetAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);

	//glVertexAttribDivisor(positionAttrib, 1);
	//glVertexAttribDivisor(colorAttrib, 1);
	//glVertexAttribDivisor(offsetAttrib, sim->vertices_per_circle);

	glBindVertexArray(0);
}

void init(SimulationState *sim) {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	glEnable( GL_BLEND );
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glColor3f(0.0, 0.0, 1.0);

	InitializeProgram();
	InitializeVAO(sim);

	glMatrixMode(GL_PROJECTION);	
	glLoadIdentity();
	glOrtho(-OBJECT_SPACE_X, OBJECT_SPACE_X, -OBJECT_SPACE_Y, OBJECT_SPACE_Y, -10.0, 10.0);

}
  
void timer(int value) {
    glutTimerFunc(16, timer, 0 );
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

	glBindBuffer(GL_ARRAY_BUFFER, offsetBufferObject);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sim->star_positions_size, &(sim->star_positions[0]));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(theProgram);
	glBindVertexArray(vao);

	glUniform1f(elapsedTimeUniform, glutGet(GLUT_ELAPSED_TIME) / 1000.0f);

	for(int i = 0; i < sim->star_count; ++i) {
		//glUniform2f(positionMove, sim->star_list[i]->pos.x, sim->star_list[i]->pos.y);
		glDrawElements(GL_TRIANGLE_FAN, sim->vertices_per_circle, GL_UNSIGNED_INT, BUFFER_OFFSET(sizeof(GLuint) * sim->vertices_per_circle * i));
		//glDrawArrays(GL_TRIANGLE_FAN, (i * sim->vertices_per_circle), sim->vertices_per_circle);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

int CheckRocksInSquare(SpaceRock *star, Tile *t) {
	int sum = 0;
	if(Check_In_Square(t->shape, star->pos.x, star->pos.y)) {
		sum++;
	}
	for(int i = 0; i < star->num_children; ++i) {
		sum += CheckRocksInSquare(star->children[i], t);
	}
	return sum;
}

void StepSpaceRock(SpaceRock *star, float time, float parentX, float parentY) {
	star->pos.x = parentX + (star->orbit_distance.x * sin((time + star->orbit_offset) * star->orbit_speed.x));
	star->pos.y = parentY + (star->orbit_distance.y * cos((time + star->orbit_offset) * star->orbit_speed.y));

	for(int i = 0; i < star->num_children; ++i) {
		StepSpaceRock(star->children[i], time, star->pos.x, star->pos.y);
	}
}

void ResetSim(SimulationState *sim) {
	sim->time = 0.0;
}

void StepSim(SimulationState *sim, int graphics_on) {
	StepSpaceRock(sim->star, sim->time, 0.0, 0.0);

	for(int i = 0; i < sim->star_count; ++i) {
		for(int ii = 0; ii < sim->vertices_per_circle; ++ii) {
			sim->star_positions[(i*sim->vertices_per_circle*4)+(ii*4)+0] = sim->star_list[i]->pos.x;
			sim->star_positions[(i*sim->vertices_per_circle*4)+(ii*4)+1] = sim->star_list[i]->pos.y;
			sim->star_positions[(i*sim->vertices_per_circle*4)+(ii*4)+2] = 0.0f;
			sim->star_positions[(i*sim->vertices_per_circle*4)+(ii*4)+3] = 0.0f;
		}
	}

	sim->activation_min = 1000000.0;
	sim->activation_max = 0;
	for(int x = 0; x < TILE_X; ++x) {
		for(int y = 0; y < TILE_Y; ++y) {
			sim->tiles[(x*TILE_Y)+y].activation = CheckRocksInSquare(sim->star, &(sim->tiles[(x*TILE_Y)+y]));
			if(sim->tiles[(x*TILE_Y)+y].activation < sim->activation_min) {
				sim->activation_min = sim->tiles[(x*TILE_Y)+y].activation;
			}
			if(sim->tiles[(x*TILE_Y)+y].activation > sim->activation_max) {
				sim->activation_max = sim->tiles[(x*TILE_Y)+y].activation;
			}
		}
	}
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
		StepSim(&render_sim, DRAW_LINES);
		render_sim.time += render_sim.timeStep;

		//sleep(sim.timeStep);
		snprintf(buffer, sizeof buffer, "%f", render_sim.time);
		glutSetWindowTitle(buffer);
		glColor3f(0.6, 0.3, 0.0);
		DrawStars(&render_sim);

		glBegin(GL_QUADS);
		Button *b_temp = &render_sim.on_button;

		if(b_temp->state == 1) {
			for(int x = 0; x < TILE_X; ++x) {
				for(int y = 0; y < TILE_Y; ++y) {
					Tile *temp = &render_sim.tiles[(x*TILE_Y)+y];
					temp->color[0] = (temp->activation - render_sim.activation_min) / (render_sim.activation_max - render_sim.activation_min);
					if(temp->color[0] > 1.0) {
						temp->color[0] = 1.0;
					}
					DrawSquare(temp->color, temp->shape.pos, temp->shape.h_height, temp->shape.h_width);
				}
			}
		}

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

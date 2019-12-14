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
			sim->tiles[(x*TILE_Y)+y].color[0] = ((float)(rand() % 100)) * 0.01;
			sim->tiles[(x*TILE_Y)+y].color[1] = ((float)(rand() % 100)) * 0.01;
			sim->tiles[(x*TILE_Y)+y].color[2] = ((float)(rand() % 100)) * 0.01;
			printf("color: %f %f %f\n", sim->tiles[(x*TILE_Y)+y].color[0],
				sim->tiles[(x*TILE_Y)+y].color[1],
				sim->tiles[(x*TILE_Y)+y].color[2]);
		}
	}
}

void FreeSim(SimulationState *sim) {
	free(sim->players);
	free(sim->tiles);
}

void init(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glColor3f(0.0, 0.0, 1.0);
	glMatrixMode(GL_PROJECTION);	
	glLoadIdentity();
	glOrtho(-OBJECT_SPACE_X, OBJECT_SPACE_X, -OBJECT_SPACE_Y, OBJECT_SPACE_Y, -10.0, 10.0);

	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);

}
  
void timer( int value )
{
    glutTimerFunc(0.01, timer, 0 );
    glutPostRedisplay();
}

void DrawLine(float x1, float y1, float x2, float y2) {
	glColor4f(1.0f, 1.0f, 0.0f, 0.3f);
	glBegin(GL_LINES);   
	glVertex3f(x1,y1,0.0);     
	glVertex3f(x2,y2,0.0);
	glEnd();
}

void DrawSolidLine(float x1, float y1, float x2, float y2) {
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);   
	glVertex3f(x1,y1,0.0);     
	glVertex3f(x2,y2,0.0);
	glEnd();
}

void ResetSim(SimulationState *sim) {
	sim->time = 0.0;
}

void StepSim(SimulationState *sim, int graphics_on) {
}

void display(void)
{
	if(CLEAR_BUFFER) {
		glClear(GL_COLOR_BUFFER_BIT);
	}

	//glBegin(GL_LINES);   
	if(1) {
		StepSim(&render_sim, DRAW_LINES);
		render_sim.time += render_sim.timeStep;

		//sleep(sim.timeStep);
		snprintf(buffer, sizeof buffer, "%f", render_sim.time);
		glutSetWindowTitle(buffer);

		glBegin(GL_QUADS);
		Button *b_temp = &render_sim.on_button;


		for(int x = 0; x < TILE_X; ++x) {
			for(int y = 0; y < TILE_Y; ++y) {
				Tile *temp = &render_sim.tiles[(x*TILE_Y)+y];
				if(b_temp->state == 1) {
					temp->color[0] = 0.0;//((float)(rand() % 100)) * 0.01;
					temp->color[1] = ((float)(rand() % 100)) * 0.01;
					temp->color[2] = 0.0;//((float)(rand() % 100)) * 0.01;
				}

				glColor3f(temp->color[0],
						  temp->color[1],
						  temp->color[2]); 
				glVertex3f(temp->shape.pos.x + (temp->shape.h_width), temp->shape.pos.y + (temp->shape.h_height), 0.0);
				glVertex3f(temp->shape.pos.x - (temp->shape.h_width), temp->shape.pos.y + (temp->shape.h_height), 0.0);
				glVertex3f(temp->shape.pos.x - (temp->shape.h_width), temp->shape.pos.y - (temp->shape.h_height), 0.0);
				glVertex3f(temp->shape.pos.x + (temp->shape.h_width), temp->shape.pos.y - (temp->shape.h_height), 0.0);
			}
		}

		if(b_temp->state == 0) {
			glColor3f(b_temp->unpressed_color[0],
				  	  b_temp->unpressed_color[1],
				  	  b_temp->unpressed_color[2]); 
		} else {
			glColor3f(b_temp->pressed_color[0],
				  	  b_temp->pressed_color[1],
				  	  b_temp->pressed_color[2]); 
		}
		glVertex3f(b_temp->shape.pos.x + (b_temp->shape.h_width), b_temp->shape.pos.y + (b_temp->shape.h_height), 1.0);
		glVertex3f(b_temp->shape.pos.x - (b_temp->shape.h_width), b_temp->shape.pos.y + (b_temp->shape.h_height), 1.0);
		glVertex3f(b_temp->shape.pos.x - (b_temp->shape.h_width), b_temp->shape.pos.y - (b_temp->shape.h_height), 1.0);
		glVertex3f(b_temp->shape.pos.x + (b_temp->shape.h_width), b_temp->shape.pos.y - (b_temp->shape.h_height), 1.0);

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


int main(int argc, char** argv) {
	srand(time(0));
	InitSim(&render_sim);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WINDOW_X_SIZE, WINDOW_Y_SIZE);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Simulator");
	init();
	glutTimerFunc(0, timer, 0);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);  /* set keyboard handler */
	glutMouseFunc(mouse);
	glutMainLoop();

	return 0;
}

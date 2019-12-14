#ifndef helper_H
#define helper_H

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
#include <cglm/mat4.h>

#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>

#include <pthread.h>


typedef struct {
	GLfloat x;
	GLfloat y;
} Vec2;

Vec2 cv2(float x, float y);

void SetColor(float *color, float a, float b, float c);

float argmin(float a, float b);
float argmax(float a, float b);

void SetStarColor(float *color, float t);
void SetMoonColor(float *color, float t);



#endif


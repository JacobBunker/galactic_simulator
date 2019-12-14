#include "helper.h"

Vec2 cv2(float x, float y) {
	Vec2 out;
	out.x = x;
	out.y = y;
	return out;
}

void SetColor(float *color, float a, float b, float c) {
	color[0] = a;
	color[1] = b;
	color[2] = c;
}

float argmin(float a, float b) {
	if(a > b) {
		return b;
	} else {
		return a;
	}
}

float argmax(float a, float b) {
	if(a > b) {
		return a;
	} else {
		return b;
	}
}

//returns valid star color when given t value between 0 and 1
//where 0 is the coldest and 1 is the hottest star
void SetStarColor(float *color, float t) {
	/* star colors range from
	red to white to blue
	r:1		1		0
	g:0		1		0
	b:0		1		1

	t from 0 to 1

	r = 0 if t > 0.5, else r = argmin(2 - (t*2), 1) argmin(t * 2, 1)
	g = 1 - 2*abs((t-0.5)) if abs(t-0.5) < 0.5, else g = 0
	b = 0 if t < 0.5, else b = argmin(t * 2, 1)
	*/

	if(t > 0.5) {
		color[0] = 0.0f;
	} else {
		color[0] = argmin(2.0 - (t * 2.0), 1.0);
	}

	if(fabs(t - 0.5) < 0.5) {
		color[1] = 1.0 - (2.0 * fabs(t - 0.5));
	} else {
		color[1] = 0.0;
	}
	
	if(t < 0.5) {
		color[2] = 0.0f;
	} else {
		color[2] = argmin(t * 2.0, 1.0);
	}

	printf("t: %f -> r: %f\tg: %f\tb: %f\n", t, color[0], color[1], color[2]);
}

void SetMoonColor(float *color, float t) {
	color[0] = 0.125 + (0.628 * t);
	color[1] = 0.125 + (0.628 * t);
	color[2] = 0.125 + (0.628 * t);
}




/*
planets go from orange to grey to blue

204		128		0
102		128		0
0		128		102

0.8		0.5		0.0
0.4		0.5		0.0
0.0		0.5		0.4

*/
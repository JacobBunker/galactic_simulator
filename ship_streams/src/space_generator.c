#include <cglm/cglm.h>
#include <cglm/mat4.h>

#include "helper.h"
#include "space_generator.h"


void PopulateSpace(Galaxy *g, float radius, int num_stars, int planet_rolls, float planet_chance, int moon_rolls, float moon_chance, float size_degeneration) {
	g->star_num = num_stars;
	g->planet_count = malloc(sizeof(int) * g->star_num);
	g->planet_num = 0;
	for(int i = 0; i < g->star_num; ++i) {
		g->planet_count[i] = 0;
		for(int roll = 0; roll < planet_rolls; ++roll) {
			if(((float)(rand() % 1000) * 0.001) <= planet_chance) {
				g->planet_count[i]++;
				g->planet_num++;
			}
		}
	}
	g->moon_count = malloc(sizeof(int) * g->planet_num);
	g->moon_num = 0;
	for(int i = 0; i < g->planet_num; ++i) {
		for(int roll = 0; roll < moon_rolls; ++roll) {
			if(((float)(rand() % 1000) * 0.001) <= moon_chance) {
				g->moon_count[i]++;
				g->moon_num++;
			}
		}
	}

	g->bodies_num = (1 + g->star_num + g->planet_num + g->moon_num);
	printf("generating %d stars, %d planets, and %d moons for a total of %d bodies\n", g->star_num, g->planet_num, g->moon_num, g->bodies_num);
	printf("memory: %ld\n", (sizeof(SpaceBody) * g->bodies_num));
	g->bodies = malloc(sizeof(SpaceBody) * g->bodies_num);

	//create center of the galaxy
	g->bodies[0].num_children = g->star_num;
	g->bodies[0].index = 0;
	g->bodies[0].parent = -1; //the center of the galaxy has no parent
	g->bodies[0].radius = radius;
	g->bodies[0].orbit_speed = 0.0f;
	g->bodies[0].orbit_offset = 0.0f;
	g->bodies[0].orbit_distance = 0.0;
	g->bodies[0].orbit_shape = cv2(0.0, 0.0);

	g->bodies[0].pos = cv2(0.0f, 0.0f);
	g->bodies[0].type = 0;
	g->bodies[0].heat = 0;
	SetColor(g->bodies[0].color, 1.0f, 1.0f, 1.0f);

	int l1 = 0;
	int l2 = 0;
	int c_planet = 0;
	int curr_body = 1;
	for(int curr_star = 0; curr_star < g->star_num; ++curr_star) {
		//make star body at curr_body here
		g->bodies[curr_body].num_children = g->planet_count[curr_star];
		g->bodies[curr_body].index = curr_body;
		g->bodies[curr_body].parent = 0; // all stars are children of the galaxy center
		g->bodies[curr_body].radius = g->bodies[g->bodies[curr_body].parent].radius * size_degeneration;
		g->bodies[curr_body].orbit_speed = (float)((random() % 95) + 5)*0.001;
		printf("star orbit speed: %f\n", g->bodies[curr_body].orbit_speed);
		g->bodies[curr_body].orbit_offset = ((float)(random() % 100000) * 0.01);
		g->bodies[curr_body].orbit_distance = (float)((random() % 2800))*0.001;
		g->bodies[curr_body].orbit_shape = cv2(((float)(random() % 500) + 250) * g->bodies[curr_body].orbit_distance,
											   ((float)(random() % 500) + 250) * g->bodies[curr_body].orbit_distance);
		g->bodies[curr_body].pos = cv2(0.0f, 0.0f); //we will set this later
		g->bodies[curr_body].type = 1;
		g->bodies[curr_body].heat = ((float)(random() % 1000) * 0.001);
		SetStarColor(g->bodies[curr_body].color, g->bodies[curr_body].heat);
		l1 = curr_body;
		curr_body++;
		for(int curr_planet = 0; curr_planet < g->planet_count[curr_star]; ++curr_planet) {
			//make planet body at curr_body here
			g->bodies[curr_body].num_children = g->moon_count[c_planet];
			g->bodies[curr_body].index = curr_body;
			g->bodies[curr_body].parent = l1; //planet's parent is it's star
			g->bodies[curr_body].radius = g->bodies[g->bodies[curr_body].parent].radius * 0.5;
			g->bodies[curr_body].orbit_speed = (float)((random() % 20000) - 10000)*0.01;
			g->bodies[curr_body].orbit_offset = ((float)(random() % 10000) * 0.01);;
			g->bodies[curr_body].orbit_distance = (float)((random() % 100))*0.01;
			g->bodies[curr_body].orbit_shape = cv2((((float)(random() % 1000)) * pow(0.1, 1)+g->bodies[g->bodies[curr_body].parent].radius), //to pow two for planets
												   (((float)(random() % 1000)) * pow(0.1, 1)+g->bodies[g->bodies[curr_body].parent].radius));
			g->bodies[curr_body].pos = cv2(0.0f, 0.0f); //we will set this later
			g->bodies[curr_body].type = 2;
			g->bodies[curr_body].heat = ((float)(random() % 1000) * 0.001);
			SetColor(g->bodies[curr_body].color, ((float)(rand() % 80) + 20.0) * 0.01, ((float)(rand() % 80) + 20.0) * 0.01, ((float)(rand() % 80) + 20.0) * 0.01);
			//SetColor(g->bodies[curr_body].color, 0.0f, 0.0f, 0.0f);
			l2 = curr_body;
			curr_body++;
			for(int curr_moon = 0; curr_moon < g->moon_count[c_planet]; ++curr_moon) {
				//make moon body at curr_body here
				g->bodies[curr_body].num_children = 0; //moons have no children
				g->bodies[curr_body].index = curr_body;
				g->bodies[curr_body].parent = l2; //moon's parent is it's planet
				g->bodies[curr_body].radius = g->bodies[g->bodies[curr_body].parent].radius * 0.5;
				g->bodies[curr_body].orbit_speed = (float)((random() % 200) - 100)*0.01;
				g->bodies[curr_body].orbit_offset = ((float)(random() % 10000) * 0.01);
				g->bodies[curr_body].orbit_distance = (float)((random() % 100))*0.01;
				g->bodies[curr_body].orbit_shape = cv2((((float)(random() % 1000)) * pow(0.1, 2)+g->bodies[g->bodies[curr_body].parent].radius), //to pow three for moons
													   (((float)(random() % 1000)) * pow(0.1, 2)+g->bodies[g->bodies[curr_body].parent].radius));
				g->bodies[curr_body].pos = cv2(0.0f, 0.0f); //we will set this later
				g->bodies[curr_body].type = 3;
				g->bodies[curr_body].heat = ((float)(random() % 1000) * 0.001);
				SetMoonColor(g->bodies[curr_body].color, g->bodies[curr_body].heat);
				//SetColor(g->bodies[curr_body].color, 0.0f, 0.0f, 0.0f);
				curr_body++;
			}
			c_planet++;
		}
	}

	/*for(int i = 0; i < g->bodies_num; ++i) {
		printf("body %d is child of %d and has %d children\n", i, g->bodies[i].parent, g->bodies[i].num_children);
	}*/

}
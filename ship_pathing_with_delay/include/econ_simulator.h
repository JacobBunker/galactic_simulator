#ifndef econ_simulator_H
#define econ_simulator_H

void InitializeProgram(GLuint *prog, const GLchar *v_s, const GLchar *f_s) {
	GLint log_size, success;
	GLchar *error_msg;

	GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);

	GLchar const **source = malloc(sizeof(char *) * 1);
	source[0] = v_s;	
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
	source[0] = f_s;	
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
	*prog = glCreateProgram();
	glAttachShader(*prog, VertexShader);
	glAttachShader(*prog, FragmentShader);
	glLinkProgram(*prog);

	GLint isLinked = 0;
	glGetProgramiv(*prog, GL_LINK_STATUS, (int *)&isLinked);
	if(isLinked == GL_FALSE) {
		printf("SHADER PROGRAM FAILED TO LINK!\n");

		log_size = 0;
		glGetProgramiv(*prog, GL_INFO_LOG_LENGTH, &log_size);
		error_msg = malloc(sizeof(GLchar) * log_size);
		glGetProgramInfoLog(*prog, log_size, &log_size, &error_msg[0]);

		printf("%s\n", error_msg);
		free(error_msg);

		glDeleteProgram(*prog);
		glDeleteShader(FragmentShader);
		glDeleteShader(VertexShader);

		return;
	} else {
		printf("SHADER PROGRAM SUCCESSFULLY LINKED!\n");
	}

	glDetachShader(*prog, VertexShader);
	glDetachShader(*prog, FragmentShader);
}

typedef struct {
	int index;
	float velocity;
	Vec2 trajectory;
	float start_time;
	float journey_time;
	int status; //0 is ready to fly, 1 is flying, 2 is resupplying
	int source;
	int destination;
	Vec2 source_pos;
	Vec2 destination_pos;
} Ship;


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


typedef struct {
	Player *players;
	Ship *ships;
	int ship_count;
	Tile *tiles;
	Galaxy g;
	Economy e;

	GLfloat *ship_positions_data;
	GLfloat *ship_colors;
	GLuint *ship_index;
	int ship_positions_data_size;
	int ship_colors_size;
	int ship_index_size;

	GLfloat *allCircleVertices;
	int vertices_per_circle;
	GLfloat *c_colors;

	GLuint *star_index;
	GLfloat *star_vertex_data;
	GLfloat *star_positions;
	GLfloat *star_colors;
	int star_index_size;
	int star_index_len;
	int star_vertex_data_size;
	int star_positions_size;
	int star_colors_size;

	GLfloat *tile_vertices;
	int vertices_per_tile;

	GLuint *tile_index;
	GLfloat *tile_vertex_data;
	GLfloat *tile_positions;
	GLfloat *tile_data; //activations
	int tile_data_size;
	int tile_vertex_data_size;
	int tile_index_len;
	int tile_index_size;
	int tile_positions_size;

	float tile_size_x;
	float tile_size_y;

	float activation_max;
	float activation_min;

	float timeStep;
	float time;
	float orbit_step;
	float orbit_time;
	int render;
	int tick_counter;
	Button on_button;

} SimulationState;

GLuint ship_VAO;
GLuint shipColorBufferObject;
GLuint shipVertexBufferObject;
GLuint shipIndexBufferObject;

GLuint shipProgram;
GLuint shipPositionAttrib;
GLuint shipColorAttrib;

GLuint star_VAO;
GLuint starOffsetBufferObject;
GLuint starColorBufferObject;
GLuint starVertexBufferObject;
GLuint starIndexBufferObject;

GLuint starProgram;
GLuint starOffsetAttrib;
GLuint starPositionAttrib;
GLuint starColorAttrib;

GLuint tile_VAO;
GLuint tileActivationBufferObject;
GLuint tileOffsetBufferObject;
GLuint tileVertexBufferObject;
GLuint tileIndexBufferObject;

GLuint tileProgram;
GLuint tileMinMaxUniform;
GLuint tilePositionAttrib;
GLuint tileOffsetAttrib;
GLuint tileActivationAttrib;

GLuint shipProjection;
GLuint starProjection;
GLuint tileProjection;

const GLchar v_ship_shader[] = "#version 330\nlayout(location = 0) in vec4 position;layout(location = 1) in vec4 color;smooth out vec4 theColor;uniform mat4 projection;void main() {gl_Position = projection * position; theColor=color;}";
const GLchar f_ship_shader[] = "#version 330\nsmooth in vec4 theColor; out vec4 outputColor; void main(){outputColor = theColor;}";

//const GLchar v_star_shader[] = "#version 330\nlayout(location = 0) in vec4 position;layout(location = 1) in vec4 color;layout(location = 2) in vec4 offset;smooth out vec4 theColor;uniform mat4 projection;void main() {vec4 totalOffset = position;gl_Position = projection * totalOffset; theColor=color;}";
const GLchar v_star_shader[] = "#version 330\nlayout(location = 0) in vec4 position;layout(location = 1) in vec4 color;layout(location = 2) in vec4 offset;smooth out vec4 theColor;uniform mat4 projection;void main() {vec4 totalOffset = vec4((position.xy*offset[3]) + offset.xy, 0.0, 1.0);gl_Position = projection * totalOffset; theColor=color;}";
const GLchar f_star_shader[] = "#version 330\nsmooth in vec4 theColor; out vec4 outputColor; void main(){outputColor = theColor;}";

const GLchar v_tile_shader[] = "#version 330\nlayout(location = 0) in vec4 position; layout(location = 1) in float activation; layout(location = 2) in vec4 offset; smooth out vec4 theColor; uniform vec2 min_max; uniform mat4 projection;void main() {vec4 totalOffset = position + offset; gl_Position = projection * totalOffset; theColor=vec4(((activation - min_max.x) / (min_max.y - min_max.x)), 0.0, 0.0, 0.5);}";
const GLchar f_tile_shader[] = "#version 330\nsmooth in vec4 theColor; out vec4 outputColor; void main(){outputColor = theColor;}";

mat4 MVP = {{1.0f, 0.0f, 0.0f, 0.0f},                    \
		   {0.0f, 1.0f, 0.0f, 0.0f},                    \
	       {0.0f, 0.0f, 1.0f, 0.0f},                    \
		   {0.0f, 0.0f, 0.0f, 1.0f}};

void InitializeShipProgram() {
	InitializeProgram(&shipProgram, v_ship_shader, f_ship_shader);

	shipPositionAttrib = glGetAttribLocation(shipProgram, "position");
	shipColorAttrib = glGetAttribLocation(shipProgram, "color");
	shipProjection = glGetUniformLocation(shipProgram, "projection");

	glUseProgram(shipProgram);
	glUniformMatrix4fv(shipProjection, 1, GL_FALSE, &MVP[0][0]);
	glUseProgram(0);
}

void InitializeStarProgram() {
	InitializeProgram(&starProgram, v_star_shader, f_star_shader);

	starPositionAttrib = glGetAttribLocation(starProgram, "position");
	starColorAttrib = glGetAttribLocation(starProgram, "color");
	starOffsetAttrib = glGetAttribLocation(starProgram, "offset");
	starProjection = glGetUniformLocation(starProgram, "projection");

	glUseProgram(starProgram);
	glUniformMatrix4fv(starProjection, 1, GL_FALSE, &MVP[0][0]);
	glUseProgram(0);
}

void InitializeTileProgram() {

	InitializeProgram(&tileProgram, v_tile_shader, f_tile_shader);

	tilePositionAttrib = glGetAttribLocation(tileProgram, "position");
	tileOffsetAttrib = glGetAttribLocation(tileProgram, "offset");
	tileActivationAttrib = glGetAttribLocation(tileProgram, "activation");
	tileMinMaxUniform = glGetUniformLocation(tileProgram, "min_max");
	tileProjection = glGetUniformLocation(tileProgram, "projection");

	glUseProgram(tileProgram);
	glUniformMatrix4fv(tileProjection, 1, GL_FALSE, &MVP[0][0]);
	glUniform2f(tileMinMaxUniform, 0.0f, 1000000000.0f);
	glUseProgram(0);
}

void InitializeShipVAO(SimulationState *sim)
{
	glGenBuffers(1, &shipVertexBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, shipVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sim->ship_positions_data_size, sim->ship_positions_data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &shipColorBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, shipColorBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sim->ship_colors_size, sim->ship_colors, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &shipIndexBufferObject);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shipIndexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sim->ship_index_size, sim->ship_index, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &ship_VAO);
	glBindVertexArray(ship_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, shipVertexBufferObject);
	glEnableVertexAttribArray(shipPositionAttrib);
	glVertexAttribPointer(shipPositionAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, shipColorBufferObject);
	glEnableVertexAttribArray(shipColorAttrib);
	glVertexAttribPointer(shipColorAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shipIndexBufferObject);

	glBindVertexArray(0);
}

void InitializeStarVAO(SimulationState *sim)
{
	glGenBuffers(1, &starVertexBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, starVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sim->star_vertex_data_size, sim->star_vertex_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &starOffsetBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, starOffsetBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sim->star_positions_size, sim->star_positions, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &starColorBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, starColorBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sim->star_colors_size, sim->star_colors, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &starIndexBufferObject);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, starIndexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sim->star_index_size, sim->star_index, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &star_VAO);
	glBindVertexArray(star_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, starVertexBufferObject);
	glEnableVertexAttribArray(starPositionAttrib);
	glVertexAttribPointer(starPositionAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(starColorAttrib);
	//glVertexAttribPointer(starColorAttrib, 4, GL_FLOAT, GL_FALSE, 0, (void*)colorDataOffset);

	glBindBuffer(GL_ARRAY_BUFFER, starColorBufferObject);
	glEnableVertexAttribArray(starColorAttrib);
	glVertexAttribPointer(starColorAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(starColorAttrib, 1);  

	glBindBuffer(GL_ARRAY_BUFFER, starOffsetBufferObject);
	glEnableVertexAttribArray(starOffsetAttrib);
	glVertexAttribPointer(starOffsetAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(starOffsetAttrib, 1);  

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, starIndexBufferObject);

	glBindVertexArray(0);
}

void InitializeTileVAO(SimulationState *sim) {
	glGenBuffers(1, &tileVertexBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, tileVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sim->tile_vertex_data_size, sim->tile_vertex_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &tileOffsetBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, tileOffsetBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sim->tile_positions_size, sim->tile_positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &tileActivationBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, tileActivationBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sim->tile_data_size, sim->tile_data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &tileIndexBufferObject);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tileIndexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sim->tile_index_size, sim->tile_index, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &tile_VAO);
	glBindVertexArray(tile_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tileVertexBufferObject);
	glEnableVertexAttribArray(tilePositionAttrib);
	glVertexAttribPointer(tilePositionAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, tileActivationBufferObject);
	glEnableVertexAttribArray(tileActivationAttrib);
	glVertexAttribPointer(tileActivationAttrib, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(tileActivationAttrib, 1);  

	glBindBuffer(GL_ARRAY_BUFFER, tileOffsetBufferObject);
	glEnableVertexAttribArray(tileOffsetAttrib);
	glVertexAttribPointer(tileOffsetAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(tileOffsetAttrib, 1);  

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tileIndexBufferObject);

	glBindVertexArray(0);

}

const char * resources[] = {
    "iron",
    "condensed carbon",
    "drones",
    "toasters",
    "fuel",
    "slaves",
};

const char * RGO_types[] = {
    "iron mine",
    "carbon condenser",
    "drone hatchery",
};


const char * Factory_types[] = {
    "toaster manufactory",
    "fuel refinery",
    "slave training camp",
};

const char * Sink_types[] = {
    "cafe la toast",
    "joe's diesel imporium",
};

#endif


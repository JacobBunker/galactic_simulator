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
	float orbit_speed;
	Vec2 orbit_distance;
	Vec2 pos;
	float orbit_offset;
	int num_children;
	int count;
	struct Rock **children;
} SpaceRock;

typedef struct {
	Player *players;
	Tile *tiles;

	Galaxy g;

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
	float orbit_time;
	int render;
	int tick_counter;
	Button on_button;

} SimulationState;



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



//const GLchar v_star_shader[] = "#version 330\nlayout(location = 0) in vec4 position;layout(location = 1) in vec4 color;layout(location = 2) in vec4 offset;smooth out vec4 theColor;uniform mat4 projection;void main() {vec4 totalOffset = position;gl_Position = projection * totalOffset; theColor=color;}";
const GLchar v_star_shader[] = "#version 330\nlayout(location = 0) in vec4 position;layout(location = 1) in vec4 color;layout(location = 2) in vec4 offset;smooth out vec4 theColor;uniform mat4 projection;void main() {vec4 totalOffset = vec4((position.xy*offset[3]) + offset.xy, 0.0, 1.0);gl_Position = projection * totalOffset; theColor=color;}";
const GLchar f_star_shader[] = "#version 330\nsmooth in vec4 theColor; out vec4 outputColor; void main(){outputColor = theColor;}";

const GLchar v_tile_shader[] = "#version 330\nlayout(location = 0) in vec4 position; layout(location = 1) in float activation; layout(location = 2) in vec4 offset; smooth out vec4 theColor; uniform vec2 min_max; uniform mat4 projection;void main() {vec4 totalOffset = position + offset; gl_Position = projection * totalOffset; theColor=vec4(((activation - min_max.x) / (min_max.y - min_max.x)), 0.0, 0.0, 0.5);}";
const GLchar f_tile_shader[] = "#version 330\nsmooth in vec4 theColor; out vec4 outputColor; void main(){outputColor = theColor;}";

#endif


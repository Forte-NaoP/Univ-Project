#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include <random>

#define NUM_CIRCLES 10
//*******************************************************************
// global constants
static const char* window_name = "cgbase - circle";
static const char* vert_shader_path = "../bin/shaders/circ.vert";
static const char* frag_shader_path = "../bin/shaders/circ.frag";
static const uint	MIN_TESS = 3;		// minimum tessellation factor (down to a triangle)
static const uint	MAX_TESS = 256;		// maximum tessellation factor (up to 256 triangles)
uint				NUM_TESS = 36;		// initial tessellation factor of the circle as a polygon

//*******************************************************************
// window objects
GLFWwindow* window = nullptr;
ivec2		window_size = ivec2(1280, 720);	// initial window size
vec2 aspect_coeff = vec2(1.0f, 1.0f);
vec2 real_coeff = vec2(1.0f, 1.0f);
float aspect_ratio = 1280.0f / 720.0f;
//*******************************************************************
// OpenGL objects
GLuint	program = 0;	// ID holder for GPU program
GLuint	vertex_buffer = 0;	// ID holder for vertex buffer
GLuint	index_buffer = 0;	// ID holder for index buffer
GLuint  offset_buffer = 0;
//*******************************************************************
// global variables
int		frame = 0;				// index of rendering frames
float	solid_color[4 * NUM_CIRCLES];
float	radius[NUM_CIRCLES];
bool    bUseSolidColor = true;
bool	bUseIndexBuffer = true;
bool	bWireframe = false;			// this is the default
//*******************************************************************
// holder of vertices and indices
std::vector<vertex>	vertex_list;	// host-side vertices
std::vector<uint>	index_list;		// host-side indices
float vertex_offset[3 * NUM_CIRCLES];

//*******************************************************************
//
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> ran(0.0f, 1.0f);
std::uniform_real_distribution<float> ran_rad(0.1f, 0.2f);
std::uniform_real_distribution<float> ran_off(-1.0f, 1.0f);
std::uniform_real_distribution<float> ran_spd(-1.f, 1.f);
//*******************************************************************

typedef struct Circle {
	vec4 solid_color;
	vec3 offset;
	vec3 speed;
	float radius;
} Circle;

Circle circles[NUM_CIRCLES];

bool collision_check(Circle& c1, Circle& c2, float e) {
	float d = c1.radius + c2.radius + e;
	float dx = c1.offset.x - c2.offset.x;
	float dy = c1.offset.y - c2.offset.y;
	return !(dx * dx + dy * dy > d * d);
}

bool is_inside(Circle& c) {
	return !(c.offset.x >= aspect_coeff.x - c.radius ||
		c.offset.x <= -aspect_coeff.x + c.radius ||
		c.offset.y >= aspect_coeff.y - c.radius ||
		c.offset.y <= -aspect_coeff.y + c.radius);
}

void move_back(Circle& c) {
	c.offset -= c.speed;
}

void move_forward(Circle& c) {
	c.offset += c.speed;
}

void elastic(Circle& c1, Circle& c2) {
	// 충돌 직전의 위치로 이동
	move_back(c1);
	move_back(c2);

	vec3 n = (c2.offset - c1.offset).normalize(); // 충돌 방향 단위 벡터
	vec3 t = { -n.y, n.x, n.z }; // 접선 방향 단위 벡터 (충돌 방향 단위 벡터 90도 반시계 회전)

	// 충돌 방향 속력
	float mag_n1 = c1.speed.dot(n);
	float mag_n2 = c2.speed.dot(n);

	// 접선 방향 속력
	float mag_t1 = c1.speed.dot(t);
	float mag_t2 = c2.speed.dot(t);

	// 질량은 면적에 비례한다 가정
	float m1 = (float)pow(c1.radius, 2);
	float m2 = (float)pow(c2.radius, 2);

	// 충돌 후 충돌 방향 속력 계산
	float mag_n1_prime = (mag_n1 * (m1 - m2) + 2 * mag_n2 * m2) / (m1 + m2);
	float mag_n2_prime = (mag_n2 * (m2 - m1) + 2 * mag_n1 * m1) / (m1 + m2);

	// 접선 방향 속도는 변경되지 않음
	float mag_t1_prime = mag_t1;
	float mag_t2_prime = mag_t2;

	// 충돌 후 속도를 충돌 방향 및 접선 방향으로 재조합
	c1.speed = n * mag_n1_prime + t * mag_t1_prime;
	c2.speed = n * mag_n2_prime + t * mag_t2_prime;

	// 충돌 직후 위치로 보정
	move_forward(c1);
	move_forward(c2);
}

void reflect_wall() {
	for (int i = 0; i < NUM_CIRCLES; i++) {
		Circle& c = circles[i];
		if ((c.offset.x + c.radius + c.speed.x >= aspect_coeff.x) ||
			(c.offset.x - c.radius + c.speed.x <= -aspect_coeff.x)
			) {
			c.speed.x = -c.speed.x;
		}
		if ((c.offset.y + c.radius + c.speed.y >= aspect_coeff.y) ||
			(c.offset.y - c.radius + c.speed.y <= -aspect_coeff.y)
			) {
			c.speed.y = -c.speed.y;
		}
		move_forward(c);
	}
}

void update() {
	for (int i = 0; i < NUM_CIRCLES - 1; ++i) {
		for (int j = i + 1; j < NUM_CIRCLES; ++j) {
			Circle& c1 = circles[i];
			Circle& c2 = circles[j];
			if (collision_check(c1, c2, 0)) {
				elastic(c1, c2);
			}
		}
	}

	reflect_wall();

	for (int i = 0; i < NUM_CIRCLES; ++i) {
		int idx = i * 3;
		Circle& c = circles[i];
		vertex_offset[idx] = c.offset.x / real_coeff.x;
		vertex_offset[idx + 1] = c.offset.y / real_coeff.y;
		vertex_offset[idx + 2] = 0.0f;
		radius[i] = c.radius / real_coeff.x;
	}

	GLint uloc;
	uloc = glGetUniformLocation(program, "bUseSolidColor");	if (uloc > -1) glUniform1i(uloc, bUseSolidColor);
	uloc = glGetUniformLocation(program, "solid_color");	if (uloc > -1) glUniform4fv(uloc, NUM_CIRCLES, solid_color);
	uloc = glGetUniformLocation(program, "aspect_ratio");	if (uloc > -1) glUniform1f(uloc, window_size.x / float(window_size.y));
	uloc = glGetUniformLocation(program, "radius");			if (uloc > -1) glUniform1fv(uloc, NUM_CIRCLES, radius);
	uloc = glGetUniformLocation(program, "offset");			if (uloc > -1) glUniform3fv(uloc, NUM_CIRCLES, vertex_offset);
}

void render() {
	// clear screen (with background color) and clear depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// notify GL that we use our own program
	glUseProgram(program);

	// bind vertex attributes to your shader program
	const char* vertex_attrib[] = { "position", "normal", "texcoord" };
	size_t		attrib_size[] = { sizeof(vertex().pos), sizeof(vertex().norm), sizeof(vertex().tex) };
	for (size_t k = 0, kn = std::extent<decltype(vertex_attrib)>::value, byte_offset = 0; k < kn; k++, byte_offset += attrib_size[k - 1]) {
		GLuint loc = glGetAttribLocation(program, vertex_attrib[k]); if (loc >= kn) continue;
		glEnableVertexAttribArray(loc);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glVertexAttribPointer(loc, attrib_size[k] / sizeof(GLfloat), GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*)byte_offset);
	}

	//render vertices: trigger shader programs to process vertex data
	if (bUseIndexBuffer) {
		if (index_buffer) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glDrawElementsInstanced(GL_TRIANGLES, index_list.size(), GL_UNSIGNED_INT, nullptr, 21);
	}
	else {
		glDrawArrays(GL_TRIANGLES, 0, NUM_TESS * 3); // NUM_TESS = N
	}

	// swap front and back buffers, and display to screen
	glfwSwapBuffers(window);
}

void reshape(GLFWwindow* window, int width, int height) {
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width, height);
	float real_ratio = (window_size.x) / (float)window_size.y;
	if (real_ratio > aspect_ratio) real_coeff = vec2(1.0f, 1.0f);
	else if (real_ratio > 1) real_coeff = vec2(aspect_ratio / real_ratio, aspect_ratio / real_ratio);
	else real_coeff = vec2(aspect_ratio, aspect_ratio);

	glViewport(0, 0, width, height);
}

void print_help() {
	printf("[help]\n");
	printf("- press ESC or 'q' to terminate the program\n");
	printf("- press F1 or 'h' to see help\n");
	printf("- press 'd' to toggle between solid color and texture coordinates\n");
	printf("- press '+/-' to increase/decrease tessellation factor (min=%d, max=%d)\n", MIN_TESS, MAX_TESS);
	printf("- press 'i' to toggle between index buffering and simple vertex buffering\n");
	printf("- press 'w' to toggle wireframe\n");
	printf("\n");
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)	glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_W) {
			bWireframe = !bWireframe;
			glPolygonMode(GL_FRONT_AND_BACK, bWireframe ? GL_LINE : GL_FILL);
			printf("> using %s mode\n", bWireframe ? "wireframe" : "solid");
		}
		else if (key == GLFW_KEY_D) {
			bUseSolidColor = !bUseSolidColor;
			printf("> using %s\n", bUseSolidColor ? "solid color" : "texture coordinates as color");
		}
	}
}

void mouse(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
		printf("> Left mouse button pressed at (%d, %d)\n", int(pos.x), int(pos.y));
	}
}

void motion(GLFWwindow* window, double x, double y) {
}

void update_vertex_buffer(uint N) {
	// clear and create new buffers
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;

	// check exceptions
	if (vertex_list.empty()) { printf("[error] vertex_list is empty.\n"); return; }

	// create buffers
	if (bUseIndexBuffer) {
		index_list.clear();
		for (uint k = 0; k < N; k++) {
			index_list.push_back(0);	// the origin
			index_list.push_back(k + 1);
			index_list.push_back(k + 2);
		}

		// generation of vertex buffer: use vertex_list as it is
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertex_list.size(), &vertex_list[0], GL_STATIC_DRAW);

		// geneation of index buffer
		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * index_list.size(), &index_list[0], GL_STATIC_DRAW);

	}
	else {
		std::vector<vertex> triangle_vertices;
		for (uint k = 0; k < N; k++) {
			triangle_vertices.push_back(vertex_list.front());	// the origin
			triangle_vertices.push_back(vertex_list[k + 1]);
			triangle_vertices.push_back(vertex_list[k + 2]);
		}

		// generation of vertex buffer: use triangle_vertices instead of vertex_list
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * triangle_vertices.size(), &triangle_vertices[0], GL_STATIC_DRAW);
	}
}

void update_circle_vertices(uint N) {
	vertex_list.clear();

	// define the position of four corner vertices
	vertex_list.push_back({ vec3(0,0,0), vec3(0.0f,0.0f,-1.0f), vec2(0.5f) });	// the origin
	for (uint k = 0; k <= N; k++) {
		float t = PI * 2.0f / float(N) * float(k);
		float c = cos(t), s = sin(t);
		vertex_list.push_back({ vec3(c,s,0.0f) , vec3(0.0f,0.0f,-1.0f), vec2(c * 0.5f + 0.5f,s * 0.5f + 0.5f) });
	}
}


bool user_init() {
	// log hotkeys
	print_help();

	float real_window = (window_size.x / float(window_size.y));
	aspect_coeff = real_window > 1 ? vec2(real_window, 1) : vec2(1, 1 / real_window);

	// init GL states
	glLineWidth(1.0f);
	glClearColor(39 / 255.0f, 40 / 255.0f, 34 / 255.0f, 1.0f);	// set clear color
	glEnable(GL_CULL_FACE);								// turn on backface culling
	glEnable(GL_DEPTH_TEST);								// turn on depth tests

	for (int i = 0; i < NUM_CIRCLES; ++i) {
		Circle& c = circles[i];
		for (int j = 0; j < 4; ++j) {
			solid_color[i * 4 + j] = c.solid_color[j] = ran(gen);
		}
		c.radius = ran_rad(gen);
	}

	Circle& c = circles[0];
	while (true) {
		vec3 offset{ ran_off(gen) * aspect_coeff.x, ran_off(gen) * aspect_coeff.y, 0 };
		c.offset = offset;
		if (is_inside(c)) {
			break;
		}
	}

	for (int i = 1; i < NUM_CIRCLES; ++i) {
		Circle& c = circles[i];
		while (true) {
			vec3 offset{ ran_off(gen) * aspect_coeff.x, ran_off(gen) * aspect_coeff.y, 0 };
			c.offset = offset;
			int j;
			for (j = 0; j < i; ++j) {
				if (!is_inside(c) ||
					collision_check(circles[i], circles[j], 0.03f)
					) {
					break;
				}
			}
			if (j >= i) {
				break;
			}
		}
	}

	for (int i = 0; i < NUM_CIRCLES; ++i) {
		float r = circles[i].radius;
		circles[i].speed = vec3{ ran_spd(gen) * (0.02f - r * 0.03f), ran_spd(gen) * (0.02f - r * 0.03f), 0 };
	}

	// define the position of four corner vertices
	update_circle_vertices(NUM_TESS);

	// create vertex buffer; called again when index buffering mode is toggled
	update_vertex_buffer(NUM_TESS);

	return true;
}

void user_finalize() {
}

int main(int argc, char* argv[]) {
	// initialization
	if (!glfwInit()) { printf("[error] failed in glfwInit()\n"); return 1; }

	// create window and initialize OpenGL extensions
	if (!(window = cg_create_window(window_name, window_size.x, window_size.y))) { glfwTerminate(); return 1; }
	if (!cg_init_extensions(window)) { glfwTerminate(); return 1; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if (!(program = cg_create_program(vert_shader_path, frag_shader_path))) { glfwTerminate(); return 1; }	// create and compile shaders/program
	if (!user_init()) { printf("Failed to user_init()\n"); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback(window, reshape);	// callback for window resizing events
	glfwSetKeyCallback(window, keyboard);			// callback for keyboard events
	glfwSetMouseButtonCallback(window, mouse);	// callback for mouse click inputs
	glfwSetCursorPosCallback(window, motion);		// callback for mouse movements

	double r = (double)1.0 / 60.0;
	double before_time = 0;
	double timer = 0;

	// enters rendering/event loop
	for (frame = 0; !glfwWindowShouldClose(window); frame++) {
		double present = glfwGetTime();
		double elapse = present - before_time;
		before_time = present;
		timer += elapse;
		if (timer >= r) {
			glfwPollEvents();	// polling and processing of events
			update();			// per-frame update
			render();			// per-frame render
			timer = 0;
		}
	}

	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}

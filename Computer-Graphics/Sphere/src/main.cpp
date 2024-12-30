#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include <stdio.h>

#define NUM_SPHERE 3
//*******************************************************************
// global constants
static const char*	window_name = "cgbase - circle";
static const char*	vert_shader_path = "../bin/shaders/circ.vert";
static const char*	frag_shader_path = "../bin/shaders/circ.frag";
static const uint	MIN_TESS = 3;		// minimum tessellation factor (down to a triangle)
static const uint	MAX_TESS = 256;		// maximum tessellation factor (up to 256 triangles)
static const uint	H_TESS = 6;
static const uint	NUM_TESS = 96;
//uint				NUM_TESS = 7;		// initial tessellation factor of the circle as a polygon


//*******************************************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = ivec2(1024, 576);	// initial window size

											//*******************************************************************
											// OpenGL objects
GLuint	program = 0;	// ID holder for GPU program
GLuint	vertex_buffer = 0;	// ID holder for vertex buffer
GLuint	index_buffer = 0;	// ID holder for index buffer

							//*******************************************************************
							// global variables
int		frame = 0;				// index of rendering frames
vec4    solid_color = vec4(1.0f, 0.5f, 0.5f, 1.0f);
float	radius = 1.0f;
int		bUseSolidColor = 0;
bool	bUseIndexBuffer = true;
bool	bWireframe = false;			// this is the default
bool	bRotation = false;
float   speed;
vec3	unit = vec3(0,0,1.0f);
float	t;
mat4	projection_matrix;

									
std::vector<vertex>	vertex_list;	// host-side vertices
std::vector<uint>	index_list;		// host-side indices
float offset_list[3 * NUM_SPHERE];
int unit_t, unit_p;
void make_unit(int i, int j)
{
	float theta = 2 * PI * i / 18;
	float p = 2 * PI * j / 18;
	float x = sin(theta) * cos(p);
	float y = sin(theta) * sin(p);
	float z = cos(theta);
	unit = vec3(x, y, z);
}
									//*******************************************************************
void update()
{
	if (bRotation) t += 0.1f * speed;
	make_unit(unit_t, unit_p);
	mat4 rotation_matrix = mat4::rotate(unit, t);

	// update uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation(program, "bUseSolidColor");	if (uloc>-1) glUniform1i(uloc, bUseSolidColor);
	uloc = glGetUniformLocation(program, "solid_color");		if (uloc>-1) glUniform4fv(uloc, 1, solid_color);	// pointer version
	uloc = glGetUniformLocation(program, "aspect_ratio");		if (uloc>-1) glUniform1f(uloc, window_size.x / float(window_size.y));
	uloc = glGetUniformLocation(program, "radius");			if (uloc>-1) glUniform1f(uloc, radius);
	uloc = glGetUniformLocation(program, "model_matrix");		if (uloc>-1) glUniformMatrix4fv(uloc, 1, GL_TRUE, rotation_matrix);
	uloc = glGetUniformLocation(program, "projection_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, projection_matrix);
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// notify GL that we use our own program
	glUseProgram(program);

	// bind vertex attributes to your shader program
	const char*	vertex_attrib[] = { "position", "normal", "texcoord" };
	size_t		attrib_size[] = { sizeof(vertex().pos), sizeof(vertex().norm), sizeof(vertex().tex) };
	for (size_t k = 0, kn = std::extent<decltype(vertex_attrib)>::value, byte_offset = 0; k<kn; k++, byte_offset += attrib_size[k - 1])
	{
		GLuint loc = glGetAttribLocation(program, vertex_attrib[k]); if (loc >= kn) continue;
		glEnableVertexAttribArray(loc);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glVertexAttribPointer(loc, attrib_size[k] / sizeof(GLfloat), GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*)byte_offset);
	}

	// render vertices: trigger shader programs to process vertex data
	if (bUseIndexBuffer)
	{
		if (index_buffer) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glDrawElements(GL_TRIANGLES, index_list.size(), GL_UNSIGNED_INT, nullptr);
	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, NUM_TESS * 3); // NUM_TESS = N
	}

	// swap front and back buffers, and display to screen
	glfwSwapBuffers(window);
}

void reshape(GLFWwindow* window, int width, int height)
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width, height);
	glViewport(0, 0, width, height);
}

void print_help()
{
	printf("[help]\n");
	printf("- press ESC or 'q' to terminate the program\n");
	printf("- press F1 or 'h' to see help\n");
	printf("- press 'w' to toggle wireframe\n");
	printf("- press 'd' to toggle (tc.xy,0) > (tc.xxx) > (tc.yyy)\n");
	printf("- press 'r' to rotate the sphere\n");
	printf("- press '+/-' to increase/decrease speed factor (min=%f, max=%f)\n", 0.0f, 2.0f);
	printf("- press '→ ← ↑ ↓' to change rotation factor\n");
	printf("\n");
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	void update_vertex_buffer(uint N);	// forward declaration
	void update_circle_vertices(uint N);	// forward declaration

	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)	glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_H || key == GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_W)
		{
			bWireframe = !bWireframe;
			glPolygonMode(GL_FRONT_AND_BACK, bWireframe ? GL_LINE : GL_FILL);
			printf("> using %s mode\n", bWireframe ? "wireframe" : "solid");
		}
		else if (key == GLFW_KEY_D)
		{
			bUseSolidColor = (bUseSolidColor + 1) % 3;
			printf("> using %s\n", bUseSolidColor == 0 ? "vec4(tc.xy,0,1)" : bUseSolidColor == 1 ? "vec4(tc.xxx,1)" : "vec4(tc.yyy,1)");
		}
		else if (key == GLFW_KEY_R)
		{
			bRotation = !bRotation;
			printf("> rotation %s\n", bRotation ? "start" : "end");
		}
		else if (key == GLFW_KEY_KP_ADD || (key == GLFW_KEY_EQUAL && (mods&GLFW_MOD_SHIFT)))
		{
			if(speed < 2.0f) speed += 0.1f;
			printf("> speed : %f / %f\n", speed, 2.0f);
		}
		else if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_MINUS)
		{
			if(speed > 0.0f) speed -= 0.1f;
			printf("> speed : %f / %f\n", speed, 2.0f);
		}
		//*/
		else if (key == GLFW_KEY_UP)
		{
			if (unit_t < 18) unit_t += 1;
			printf("> unit_t, unit_p  (%d / 18), (%d / 18)\n", unit_t, unit_p);
		}
		else if (key == GLFW_KEY_DOWN)
		{
			if (unit_t > -18) unit_t -= 1;
			printf("> unit_t, unit_p  (%d / 18), (%d / 18)\n", unit_t, unit_p);
		}
		else if (key == GLFW_KEY_RIGHT)
		{
			if (unit_p < 18) unit_p += 1;
			printf("> unit_t, unit_p  (%d / 18), (%d / 18)\n", unit_t, unit_p);
		}
		else if (key == GLFW_KEY_LEFT)
		{
			if (unit_p > -18) unit_p -= 1;
			printf("> unit_t, unit_p  (%d / 18), (%d / 18)\n", unit_t, unit_p);
		}
		//*/
	}
}

void mouse(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
		printf("> Left mouse button pressed at (%d, %d)\n", int(pos.x), int(pos.y));
	}
}

void motion(GLFWwindow* window, double x, double y)
{
}

void update_vertex_buffer(uint N)
{
	// clear and create new buffers
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;

	// check exceptions
	if (vertex_list.empty()) { printf("[error] vertex_list is empty.\n"); return; }

	// create buffers
	if (bUseIndexBuffer)
	{
		index_list.clear();
		for (uint i = 0; i < N; i++) {
			index_list.push_back(0);	
			index_list.push_back(i + 1);
			index_list.push_back(i + 2);
		}
		uint k;
		for (k = 1; k < (N + 1) * (N - 2); k+=(N+1))
		{
			for (uint j = k; j < k+N; j++) {
				index_list.push_back(j);
				index_list.push_back(j+N+1);
				index_list.push_back(j+N+2);

				index_list.push_back(j);
				index_list.push_back(j+N+2);
				index_list.push_back(j+1);
			}
		}
		for (uint i = k; i < k + N; i++) {
			index_list.push_back(i);
			index_list.push_back(k + 1 + N);
			index_list.push_back(i + 1);
		}
		

		// generation of vertex buffer: use vertex_list as it is
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*vertex_list.size(), &vertex_list[0], GL_STATIC_DRAW);

		// geneation of index buffer
		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*index_list.size(), &index_list[0], GL_STATIC_DRAW);
	}
	else
	{
		std::vector<vertex> triangle_vertices;
		for (uint k = 0; k < N; k++)
		{
			triangle_vertices.push_back(vertex_list.front());	// the origin
			triangle_vertices.push_back(vertex_list[k + 1]);
			triangle_vertices.push_back(vertex_list[k + 2]);
		}

		// generation of vertex buffer: use triangle_vertices instead of vertex_list
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*triangle_vertices.size(), &triangle_vertices[0], GL_STATIC_DRAW);
	}
}

void update_circle_vertices(uint N)
{
	vertex_list.clear();

	for (int i = 0; i <= NUM_TESS; i++) {
		float theta = PI * i / NUM_TESS;
		for (int j = 0; j <= NUM_TESS; j++) {
			float p = PI * 2.0f / float(NUM_TESS) * float(j);
			float x = sin(theta) * cos(p);
			float y = sin(theta) * sin(p);
			float z = cos(theta);
			vertex_list.push_back({ vec3(x, y, z), vec3(x, y, z), vec2((p / (2 * PI)), (1.0f - (theta / PI))) });
			if (i == 0 || i == NUM_TESS) break;
		}
	}
	
}

bool user_init()
{
	// log hotkeys
	print_help();
	speed = 0.1f;
	// init GL states
	glLineWidth(1.0f);
	glClearColor(39 / 255.0f, 40 / 255.0f, 34 / 255.0f, 1.0f);	// set clear color
	glEnable(GL_CULL_FACE);								// turn on backface culling
	glEnable(GL_DEPTH_TEST);								// turn on depth tests

	projection_matrix = mat4::look_at(vec3(1.0f, 0.0f, 0.0f), vec3(0, 0, 0), vec3(0, 0, 1));
	update_circle_vertices(NUM_TESS);

	// create vertex buffer; called again when index buffering mode is toggled
	update_vertex_buffer(NUM_TESS);

	return true;
}

void user_finalize()
{
}

int main(int argc, char* argv[])
{
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

												// enters rendering/event loop
	for (frame = 0; !glfwWindowShouldClose(window); frame++)
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
	}

	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}

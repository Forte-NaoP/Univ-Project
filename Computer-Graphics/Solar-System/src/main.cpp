#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include "trackball.h"		// virtual trackball

//*******************************************************************
// global constants
static const char*	window_name = "cgbase - trackball";
static const char*	vert_shader_path = "../bin/shaders/trackball.vert";
static const char*	frag_shader_path = "../bin/shaders/trackball.frag";
static const char*  meshes[19] = {
	"../bin/mesh/sun.jpg", "../bin/mesh/mercury.jpg", "../bin/mesh/venus.jpg","../bin/mesh/earth.jpg", "../bin/mesh/mars.jpg",
	"../bin/mesh/jupiter.jpg", "../bin/mesh/saturn.jpg", "../bin/mesh/uranus.jpg", "../bin/mesh/neptune.jpg", "../bin/mesh/moon.jpg",
	"../bin/mesh/saturn-ring.jpg", "../bin/mesh/saturn-ring-alpha.jpg","../bin/mesh/uranus-ring.jpg", "../bin/mesh/uranus-ring-alpha.jpg",
	"../bin/mesh/mercury-bump.jpg", "../bin/mesh/venus-bump.jpg","../bin/mesh/earth-bump.jpg", "../bin/mesh/mars-bump.jpg", "../bin/mesh/moon-bump.jpg"
};
static const char*  bumps[5] = {
	"../bin/mesh/mercury-bump.jpg", "../bin/mesh/venus-bump.jpg","../bin/mesh/earth-bump.jpg", "../bin/mesh/mars-bump.jpg", "../bin/mesh/moon-bump.jpg"
};
static const char*  rings[4] = {
	"../bin/mesh/saturn-ring.jpg", "../bin/mesh/saturn-ring-alpha.jpg","../bin/mesh/uranus-ring.jpg", "../bin/mesh/uranus-ring-alpha.jpg"
};
static const uint	NUM_SPHERE = 19;
static const uint	NUM_TESS = 96;
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//*******************************************************************
// common structures
struct camera
{
	vec3	eye = vec3( 2000, 0, 0 );
	vec3	at = vec3( 0, 0, 0 );
	vec3	up = vec3( 0, 0, 1 );
	mat4	view_matrix = mat4::look_at( eye, at, up );

	float	fovy = PI/4.0f; // must be in radian
	float	aspect_ratio;
	float	dnear = 1.0f;
	float	dfar = 10000.0f;
	mat4	projection_matrix;
};
struct light_t
{
	vec4	position = vec4(0.0f, 0.0f, 0.0f, 1.0f);   // directional light
	vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
};

struct material_t
{
	vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float	shininess = 1000.0f;
};
//*******************************************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = ivec2( 1280, 800 );	// initial window size

//*******************************************************************
// OpenGL objects
GLuint	program	= 0;	// ID holder for GPU program
GLuint	vertex_buffer = 0;	// ID holder for vertex buffer
GLuint	index_buffer = 0;
GLuint	ring_vertex_buffer = 0;	// ID holder for vertex buffer
GLuint	ring_index_buffer = 0;
//*******************************************************************
// global variables
int		frame = 0;	// index of rendering frames
std::vector<vertex>	vertex_list;	// host-side vertices
std::vector<uint>	index_list;
std::vector<vertex> ring_vertex_list;
std::vector<uint> ring_index_list;
bool key_shift=false;
bool key_ctrl=false;
vec2 prev;
mat4 prev_pan;
vec3 prev_eye;
vec3 prev_at;
mat4 prev_view;

GLuint textures[19];
GLuint bump[5];
GLuint texnorm[5];
GLuint ringtex[4];
//*******************************************************************
// scene objects
mesh*		pMesh = nullptr;
camera		cam;
trackball	tb;
light_t		light;
material_t	material;
//*******************************************************************
//float radius = 50.0f;
float radius[NUM_SPHERE] = {200.0f, 15.0f, 25.0f, 30.0f, 20.0f, 75.0f, 60.0f, 40.0f, 35.0f, 
							10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f};
float ring_radius[4] = {100.0f, 150.0f, 80.0f, 130.0f};
float rot_rad[NUM_SPHERE] = { 0.0f, 300.0f, 400.0f, 550.0f, 670.0f, 975.0f, 1275.0f, 1500.0f, 2000.0f, 
							  50.0f, 100.0f, 130.0f, 160.0f, 190.0f, 60.0f, 90.0f, 70.0f, 100.0f, 130.0f};
int center_idx[NUM_SPHERE] = { 0,0,0,0,0,0,0,0,0,3,5,5,5,5,7,7,8,8,8 };
float init_pos[NUM_SPHERE];
float offset[3 * NUM_SPHERE];
float ring_offset[3 * 2];
float rotate_mat[4* 4 * NUM_SPHERE];
float bt, t;
bool isPause = false;
mat4 panning;
GLint tex_index[24] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23 };
float axis[19] = {0, 0, 3.0f, 0.4f, 0.43f, 0.05f, 0.46f, 1.70f, 0.5f, 0.11f, 0.11f, 0.11f, 0.11f, 0.11f, 0.11f, 0.11f, 0.11f, 0.11f, 0.11f};
float ring_axis[2] = {0.46f, 1.7f};
//*******************************************************************
void update()
{
	// update projection matrix
	cam.aspect_ratio = window_size.x/float(window_size.y);
	cam.projection_matrix = mat4::perspective( cam.fovy, cam.aspect_ratio, cam.dnear, cam.dfar );

	// build the model matrix for oscillating scale
	if (!isPause) t = float(glfwGetTime()) - bt;
	else bt = float(glfwGetTime()) - t;

	float speed;
	for (int i = 0; i < NUM_SPHERE; i++) {
		speed = t * 100 / radius[i];
		mat4 rot_mat = {
			cosf(speed), -sinf(speed), 0, 0,
			sinf(speed), cosf(speed), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
		
		if (i < 9) {
			rot_mat = mat4::rotate(vec3(1, 0, 0), axis[i]) * rot_mat;
			speed = init_pos[i] + speed / ((i + 1) * 1.0f);
		}
		else {
			rot_mat = mat4::rotate(vec3(1, 0, 0), axis[i]) * rot_mat;
			speed = init_pos[i] + speed / ((i + 1) * 0.2f);
		}
		mat4 trans_mat = {
			cosf(speed), -sinf(speed), 0, 0,
			sinf(speed), cosf(speed), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
		vec3 temp_pos = vec3(0, 1, 0); 
		temp_pos = mat3(trans_mat) * temp_pos;
		temp_pos *= rot_rad[i];
		temp_pos.x += offset[3 * center_idx[i] + 0];
		temp_pos.y += offset[3 * center_idx[i] + 1];
		temp_pos.z += offset[3 * center_idx[i] + 2];
		
		memcpy(offset + i * 3, (float*)temp_pos, 3 * sizeof(float));
		memcpy(rotate_mat + i * 16, (float*)rot_mat, 16 * sizeof(float));
	}
	//offset = planet_pos, rotate_mat = matrix
	// update uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation(program, "isRing");					if (uloc > -1) glUniform1i(uloc, 0);
	uloc = glGetUniformLocation( program, "view_matrix" );			if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, cam.view_matrix );
	uloc = glGetUniformLocation( program, "projection_matrix" );	if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, cam.projection_matrix );
	uloc = glGetUniformLocation( program, "radius");				if(uloc>-1) glUniform1fv(uloc, NUM_SPHERE, radius);
	uloc = glGetUniformLocation(program, "rotate_mat");				if (uloc>-1) glUniformMatrix4fv(uloc, NUM_SPHERE, GL_TRUE, rotate_mat);
	uloc = glGetUniformLocation(program, "location");				if (uloc > -1) glUniform3fv(uloc, NUM_SPHERE, offset);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTextures(0, 10, textures);
	glUniform1iv(glGetUniformLocation(program, "tex"), 10, tex_index);

	glActiveTexture(GL_TEXTURE0);
	glBindTextures(14, 5, textures+14);
	glUniform1iv(glGetUniformLocation(program, "bump"), 10, tex_index + 14);

	// setup light properties
	glUniform4fv(glGetUniformLocation(program, "light_position"), 1, light.position);
	glUniform4fv(glGetUniformLocation(program, "Ia"), 1, light.ambient);
	glUniform4fv(glGetUniformLocation(program, "Id"), 1, light.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Is"), 1, light.specular);

	// setup material properties
	glUniform4fv(glGetUniformLocation(program, "Ka"), 1, material.ambient);
	glUniform4fv(glGetUniformLocation(program, "Kd"), 1, material.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Ks"), 1, material.specular);
	glUniform1f(glGetUniformLocation(program, "shininess"), material.shininess);

}
GLuint vertexarray, ringarray;

void render()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glUseProgram( program );
	glGenVertexArrays(1, &vertexarray);
	glBindVertexArray(vertexarray);
	const char*	vertex_attrib[]	= { "position", "normal", "texcoord" };
	size_t		attrib_size[]	= { sizeof(vertex().pos), sizeof(vertex().norm), sizeof(vertex().tex) };
	for (size_t k = 0, kn = std::extent<decltype(vertex_attrib)>::value, byte_offset = 0; k < kn; k++, byte_offset += attrib_size[k - 1])
	{
		GLuint loc = glGetAttribLocation(program, vertex_attrib[k]); if (loc >= kn) continue;
		glEnableVertexAttribArray(loc);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glVertexAttribPointer(loc, attrib_size[k] / sizeof(GLfloat), GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*)byte_offset);
	}
	// render vertices: trigger shader programs to process vertex data
	if(index_buffer) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glDrawElementsInstanced(GL_TRIANGLES, index_list.size(), GL_UNSIGNED_INT, nullptr, NUM_SPHERE);
	// swap front and back buffers, and display to screen
	glBindVertexArray(0);
	
	for (uint i = 0; i < 2; ++i) {
		float speed = t * 10 / radius[i+6];
		mat4 rot_mat =
		{
			cosf(speed), -sinf(speed), 0, 0,
			sinf(speed), cosf(speed), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
		rot_mat = mat4::rotate(vec3(1, 0, 0), ring_axis[i]) * rot_mat;
		vec3 temp_pos = vec3(offset[(i + 6) * 3 + 0], offset[(i + 6) * 3 + 1], offset[(i + 6) * 3 + 2]);
		memcpy(ring_offset + i * 3, (float*)temp_pos, 3 * sizeof(float));
		memcpy(rotate_mat + i * 16, (float*)rot_mat, 16 * sizeof(float));
	}
	GLint uloc;
	uloc = glGetUniformLocation(program, "isRing");					if (uloc > -1) glUniform1i(uloc, 1);
	uloc = glGetUniformLocation(program, "radius");					if (uloc > -1) glUniform1fv(uloc, 4, ring_radius);
	uloc = glGetUniformLocation(program, "rotate_mat");				if (uloc > -1) glUniformMatrix4fv(uloc, 2, GL_TRUE, rotate_mat);
	uloc = glGetUniformLocation(program, "location");				if (uloc > -1) glUniform3fv(uloc, 2, ring_offset);
	

	glActiveTexture(GL_TEXTURE0);
	glBindTextures(10, 4, textures + 10);
	glUniform1iv(glGetUniformLocation(program, "tex"), 4, tex_index + 10);

	glGenVertexArrays(1, &ringarray);
	glBindVertexArray(ringarray);
	for (size_t k = 0, kn = std::extent<decltype(vertex_attrib)>::value, byte_offset = 0; k < kn; k++, byte_offset += attrib_size[k - 1])
	{
		GLuint loc = glGetAttribLocation(program, vertex_attrib[k]); if (loc >= kn) continue;
		glEnableVertexAttribArray(loc);
		glBindBuffer(GL_ARRAY_BUFFER, ring_vertex_buffer);
		glVertexAttribPointer(loc, attrib_size[k] / sizeof(GLfloat), GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*)byte_offset);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ring_index_buffer);
	glDrawElementsInstanced(GL_TRIANGLES, ring_index_list.size(), GL_UNSIGNED_INT, nullptr, 2);
	glBindVertexArray(0);
	glfwSwapBuffers( window );
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
	printf( "- press Home to reset camera\n" );
	printf("- press 'w' to toggle wireframe\n");
	printf("- mouse left click drag : view rotation\n");
	printf("- mouse right click drag or shift + mouse left: zoom\n");
	printf("- mouse wheel click drag or ctrl + mouse left: panning\n");
	printf( "\n" );
}
bool bWireframe = false;

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_HOME || key == GLFW_KEY_KP_7) {
			cam = camera(); 
			prev = vec2(); prev_pan = mat4();
			prev_eye = vec3(); prev_at = vec3();
			prev_view = mat4();
		}
		else if (key == GLFW_KEY_LEFT_SHIFT) key_shift = true;
		else if (key == GLFW_KEY_LEFT_CONTROL) key_ctrl = true;
		else if (key == GLFW_KEY_W)
		{
			bWireframe = !bWireframe;
			glPolygonMode(GL_FRONT_AND_BACK, bWireframe ? GL_LINE : GL_FILL);
			printf("> using %s mode\n", bWireframe ? "wireframe" : "solid");
		}
		else if (key == GLFW_KEY_PAUSE) isPause = !isPause;

	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT_SHIFT) key_shift = false;
		else if (key == GLFW_KEY_LEFT_CONTROL) key_ctrl = false;
	}
}
vec2 prev_mouse;
void mouse( GLFWwindow* window, int button, int action, int mods )
{
	if(button==GLFW_MOUSE_BUTTON_LEFT || button==GLFW_MOUSE_BUTTON_MIDDLE || button==GLFW_MOUSE_BUTTON_RIGHT)
	{
		dvec2 pos; glfwGetCursorPos(window,&pos.x,&pos.y);
		vec2 npos = vec2( float(pos.x)/float(window_size.x-1), float(pos.y)/float(window_size.y-1) );
		prev = npos;
		prev_at = cam.at;
		prev_eye = cam.eye;
		prev_pan = panning;
		prev_view = cam.view_matrix;
		if (action == GLFW_PRESS) {
			if(button== GLFW_MOUSE_BUTTON_MIDDLE) key_ctrl = true;
			if (button == GLFW_MOUSE_BUTTON_RIGHT) key_shift = true;
			tb.begin(cam.view_matrix, npos.x, npos.y);
			prev_mouse = npos;
		}
		else if (action == GLFW_RELEASE) {
			if (button == GLFW_MOUSE_BUTTON_MIDDLE) key_ctrl = false;
			if (button == GLFW_MOUSE_BUTTON_RIGHT) key_shift = false;
			tb.end();
		}
	}
}

void motion( GLFWwindow* window, double x, double y )
{
	if(!tb.is_tracking()) return;
	vec2 npos = vec2( float(x)/float(window_size.x-1), float(y)/float(window_size.y-1) );
	if (key_shift) {
		float rate = (npos.y - prev_mouse.y);
		if (rate >= 0.99f) rate = 0.99f;
		cam.eye = (1 - rate) * prev_eye + rate * cam.at;
		cam.view_matrix = mat4::look_at(cam.eye, cam.at, cam.up);
	}
	else if (key_ctrl) {
		panning = mat4::translate((npos.x - prev.x) * 300, (prev.y - npos.y) * 300, 0);
		cam.view_matrix = panning * prev_view;
		cam.eye = ((mat3)cam.view_matrix).inverse() * -vec3(cam.view_matrix.at(3), cam.view_matrix.at(7), cam.view_matrix.at(11));
		cam.at = (cam.eye - prev_eye) + prev_at;
	}
	else {
		cam.view_matrix = tb.update(npos.x, npos.y, cam.at);
		cam.eye = ((mat3)cam.view_matrix).inverse() * -vec3(cam.view_matrix.at(3), cam.view_matrix.at(7), cam.view_matrix.at(11));
		vec3 u = vec3(cam.view_matrix._11, cam.view_matrix._12, cam.view_matrix._13);
		vec3 n = vec3(cam.view_matrix._31, cam.view_matrix._32, cam.view_matrix._33);
		cam.up = n.cross(u);
	}
}

void update_ring_vertex_buffer(uint N)
{
	if (ring_vertex_buffer)	glDeleteBuffers(1, &ring_vertex_buffer);	ring_vertex_buffer = 0;
	if (ring_index_buffer)	glDeleteBuffers(1, &ring_index_buffer);	ring_index_buffer = 0;
	if (ring_vertex_list.empty()) { printf("[error] vertex_list is empty.\n"); return; }

	ring_index_list.clear();
	for (uint i = 0; i < N; i++) {
		ring_index_list.push_back(2 * i);
		ring_index_list.push_back(2 * i + 1);
		ring_index_list.push_back(2 * (i + 1));

		ring_index_list.push_back(2 * (i + 1));
		ring_index_list.push_back(2 * i + 1);
		ring_index_list.push_back(2 * (i + 1) + 1);

		ring_index_list.push_back(2 * i);
		ring_index_list.push_back(2 * (i + 1));
		ring_index_list.push_back(2 * i + 1);

		ring_index_list.push_back(2 * (i + 1));
		ring_index_list.push_back(2 * (i + 1) + 1);
		ring_index_list.push_back(2 * i + 1);
	}
	glGenBuffers(1, &ring_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, ring_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*ring_vertex_list.size(), &ring_vertex_list[0], GL_STATIC_DRAW);

	glGenBuffers(1, &ring_index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ring_index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*ring_index_list.size(), &ring_index_list[0], GL_STATIC_DRAW);
}

void update_vertex_buffer(uint N)
{
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;
	if (vertex_list.empty()) { printf("[error] vertex_list is empty.\n"); return; }

	index_list.clear();
	for (uint i = 0; i < N; i++) {
		index_list.push_back(0);
		index_list.push_back(i + 1);
		index_list.push_back(i + 2);
	}
	uint k;
	for (k = 1; k < (N + 1) * (N - 2); k += (N + 1))
	{
		for (uint j = k; j < k + N; j++) {
			index_list.push_back(j);
			index_list.push_back(j + N + 1);
			index_list.push_back(j + N + 2);

			index_list.push_back(j);
			index_list.push_back(j + N + 2);
			index_list.push_back(j + 1);
		}
	}
	for (uint i = k; i < k + N; i++) {
		index_list.push_back(i);
		index_list.push_back(k + 1 + N);
		index_list.push_back(i + 1);
	}
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*vertex_list.size(), &vertex_list[0], GL_STATIC_DRAW);

	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*index_list.size(), &index_list[0], GL_STATIC_DRAW);
}

void update_ring_vertices(uint N)
{
	ring_vertex_list.clear();
	for (uint i = 0; i <= N; i++) {
		float theta = PI * 2.0f / float(N) * float(i);
		float x = cos(theta); float y = sin(theta);
		ring_vertex_list.push_back({ vec3(0, 0, 0), vec3(x, y, 0), vec2(0, 0) });
		ring_vertex_list.push_back({ vec3(x, y, 0), vec3(x, y, 0), vec2(1, 0) });
	}
}


void update_circle_vertices(uint N)
{
	vertex_list.clear();
	for (int i = 0; i <= NUM_TESS; i++) {
		float theta = PI * i / NUM_TESS;
		for (int j = 0; j <= NUM_TESS; j++) {
			float p = PI * 2.0f / float(NUM_TESS) * float(j);
			float x = sin(theta) * cos(p); float y = sin(theta) * sin(p); float z = cos(theta);
			vertex_list.push_back({ vec3(x, y, z), vec3(x, y, z), vec2((p / (2 * PI)), (1.0f - (theta / PI))) });
			if (i == 0 || i == NUM_TESS) break;
		}
	}
}


bool user_init()
{
	print_help();
	for (int i = 0; i < NUM_SPHERE; i++) {
		init_pos[i] = (float)rand() / (float)RAND_MAX * 2 * PI;
	}
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(19, textures);
	for (uint i = 0; i < 19; i++) {
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		int width, height, comp = 3;
		unsigned char* pimage0 = stbi_load(meshes[i], &width, &height, &comp, 3); if (comp == 1) comp = 3; /* convert 1-channel to 3-channel image */
		int stride0 = width * comp, stride1 = (stride0 + 3)&(~3);	// 4-byte aligned stride
		unsigned char* pimage = (unsigned char*)malloc(sizeof(unsigned char)*stride1*height);
		for (int y = 0; y < height; y++) memcpy(pimage + (height - 1 - y)*stride1, pimage0 + y * stride0, stride0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8 /* GL_RGB for legacy GL */, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pimage);
		int mip_levels = miplevels(window_size.x, window_size.y);
		for (int k = 1, w = width >> 1, h = height >> 1; k < mip_levels; k++, w = max(1, w >> 1), h = max(1, h >> 1))
			glTexImage2D(GL_TEXTURE_2D, k, GL_RGB8 /* GL_RGB for legacy GL */, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glGenerateMipmap(GL_TEXTURE_2D);

		// configure texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glActiveTexture(GL_TEXTURE0);
		// release the new image
		free(pimage);
	}
	
	// turn on depth tests
	update_circle_vertices(NUM_TESS);
	update_vertex_buffer(NUM_TESS);
	update_ring_vertices(NUM_TESS);
	update_ring_vertex_buffer(NUM_TESS);
	return true;
}

void user_finalize()
{
}

int main( int argc, char* argv[] )
{
	// initialization
	if(!glfwInit()){ printf( "[error] failed in glfwInit()\n" ); return 1; }

	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// version and extensions

	// initializations and validations
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movement

	// enters rendering/event loop
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
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

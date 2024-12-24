#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include <random>

#define NUM_CIRCLES 21
//*******************************************************************
// global constants
static const char*	window_name = "cgbase - circle";
static const char*	vert_shader_path = "../bin/shaders/circ.vert";
static const char*	frag_shader_path = "../bin/shaders/circ.frag";
static const uint	MIN_TESS = 3;		// minimum tessellation factor (down to a triangle)
static const uint	MAX_TESS = 256;		// maximum tessellation factor (up to 256 triangles)
uint				NUM_TESS = 256;		// initial tessellation factor of the circle as a polygon

//*******************************************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = ivec2( 1280, 720 );	// initial window size
vec2 aspect_coeff = vec2(1.0f, 1.0f);
vec2 real_coeff = vec2(1.0f, 1.0f);
float aspect_ratio = 1280.0f/720.0f;
//*******************************************************************
// OpenGL objects
GLuint	program			= 0;	// ID holder for GPU program
GLuint	vertex_buffer	= 0;	// ID holder for vertex buffer
GLuint	index_buffer	= 0;	// ID holder for index buffer
GLuint  offset_buffer	= 0;
//*******************************************************************
// global variables
int		frame = 0;				// index of rendering frames
//vec4    solid_color = vec4( 1.0f, 0.5f, 0.5f, 1.0f );
float	solid_color[4 * NUM_CIRCLES];
float	radius[NUM_CIRCLES];
float	tmp_radius[NUM_CIRCLES];
bool    bUseSolidColor = true;
bool	bUseIndexBuffer = true;
bool	bWireframe = false;			// this is the default
//*******************************************************************
// holder of vertices and indices
std::vector<vertex>	vertex_list;	// host-side vertices
std::vector<uint>	index_list;		// host-side indices
float offset_list[3 * NUM_CIRCLES];
float tmp_offset[3 * NUM_CIRCLES];
float speed[3 * NUM_CIRCLES];

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
	float solid_color[4];
	float radius;
	float offset[3];
}Circle;

bool collision(float a, float b, float c, float d, float e)
{
	//float aspect_ratio = window_size.x / (float)window_size.y;
	float dx = (a - c);// * (aspect_ratio > 1 ? 1/aspect_ratio : 1);
	float dy = (b - d);// * (aspect_ratio > 1 ? 1 : aspect_ratio);
	return !(dx* dx + dy * dy > e*e);
}

void elastic(float theta1, float theta2, float theta3, int i, int j)
{
	int iidx = 3 * i, jidx = 3 * j;
	float m1 = radius[i] * radius[i];
	float m2 = radius[j] * radius[j];
	float tmp_speed1 = sqrt(speed[iidx] * speed[iidx] + speed[iidx + 1] * speed[iidx + 1]);
	float tmp_speed2 = sqrt(speed[jidx] * speed[jidx] + speed[jidx + 1] * speed[jidx + 1]);
	float head1 = (tmp_speed1 * cos(theta1 - theta3) * (m1 - m2) + 2 * m2 * tmp_speed2 * cos(theta2 - theta3)) / (m1 + m2);
	float head2 = (tmp_speed2 * cos(theta2 - theta3) * (m2 - m1) + 2 * m1 * tmp_speed1 * cos(theta1 - theta3)) / (m1 + m2);

	speed[iidx] = head1 * cos(theta3) + tmp_speed1 * sin(theta1 - theta3) * sin(theta3);
	speed[iidx+1] = head1 * sin(theta3) + tmp_speed1 * sin(theta1 - theta3) * cos(theta3);
	
	speed[jidx] = head2 * cos(theta3) + tmp_speed2 * sin(theta2 - theta3) * sin(theta3);
	speed[jidx + 1] = head2 * sin(theta3) + tmp_speed2 * sin(theta2 - theta3) * cos(theta3);
	
	
}

void update()
{
	//*/
	for (int i = 0; i < NUM_CIRCLES - 1; i++) {
		for (int j = i + 1; j < NUM_CIRCLES; j++) {
			int iidx = 3 * i, jidx = 3 * j;
			if (collision(offset_list[iidx], offset_list[iidx + 1], offset_list[jidx], offset_list[jidx + 1], radius[i] + radius[j])) {
				offset_list[iidx] -= speed[iidx];
				offset_list[iidx + 1] -= speed[iidx + 1];
				offset_list[jidx] -= speed[jidx];
				offset_list[jidx + 1] -= speed[jidx + 1];
				float theta1 = atan2(speed[iidx+1], speed[iidx]);
				float theta2 = atan2(speed[jidx + 1], speed[jidx]);
				float theta3 = atan2(offset_list[jidx + 1] - offset_list[iidx + 1], offset_list[jidx] - offset_list[iidx]);
				elastic(theta1, theta2, theta3, i, j);
				offset_list[iidx] += speed[iidx];
				offset_list[iidx + 1] += speed[iidx + 1];
				offset_list[jidx] += speed[jidx];
				offset_list[jidx + 1] += speed[jidx + 1];

			}
		}
	}//*/
	for (int i = 0; i < NUM_CIRCLES; i++) {
		int idx = 3 * i;
		if (offset_list[idx] + radius[i] + speed[idx] >= 1.0f * aspect_coeff.x) {
			speed[idx] = -speed[idx];
		}
		if (offset_list[idx + 1] + radius[i] + speed[idx+1] >= 1.0f * aspect_coeff.y) {
			speed[idx + 1] = -speed[idx + 1];
		}
		if (offset_list[idx] - radius[i] + speed[idx] <= -1.0f * aspect_coeff.x) {
			speed[idx] = -speed[idx];
		}
		if (offset_list[idx + 1] - radius[i] + speed[idx+1] <= -1.0f  * aspect_coeff.y) {
			speed[idx + 1] = -speed[idx + 1];
		}
		offset_list[3 * i] += speed[3 * i];
		offset_list[3 * i + 1] += speed[3 * i + 1];
		offset_list[3 * i + 2] = 0;
	}



	for (int i = 0; i < NUM_CIRCLES; i++) {
		int idx = 3 * i;
		tmp_offset[idx] = offset_list[idx] / real_coeff.x;
		tmp_offset[idx + 1] = offset_list[idx+1] / real_coeff.y;
		tmp_offset[idx + 2] = 0.0f;
		tmp_radius[i] = radius[i] / real_coeff.x;
	}

	GLint uloc;
	uloc = glGetUniformLocation( program, "bUseSolidColor" );	if(uloc>-1) glUniform1i( uloc, bUseSolidColor );
	uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, NUM_CIRCLES, solid_color);
	uloc = glGetUniformLocation( program, "aspect_ratio" );		if(uloc>-1) glUniform1f( uloc, window_size.x/float(window_size.y) );
	uloc = glGetUniformLocation(program, "radius");				if (uloc > -1) glUniform1fv(uloc, NUM_CIRCLES, tmp_radius);
	uloc = glGetUniformLocation(program, "offset");				if (uloc > -1) glUniform3fv(uloc, NUM_CIRCLES, tmp_offset);
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// notify GL that we use our own program
	glUseProgram( program );

	// bind vertex attributes to your shader program
	const char*	vertex_attrib[]	= { "position", "normal", "texcoord"};
	size_t		attrib_size[]	= { sizeof(vertex().pos), sizeof(vertex().norm), sizeof(vertex().tex) };
	for( size_t k=0, kn=std::extent<decltype(vertex_attrib)>::value, byte_offset=0; k<kn; k++, byte_offset+=attrib_size[k-1] )
	{
		GLuint loc = glGetAttribLocation( program, vertex_attrib[k] ); if(loc>=kn) continue;
		glEnableVertexAttribArray( loc );
		glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
		glVertexAttribPointer( loc, attrib_size[k]/sizeof(GLfloat), GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*) byte_offset );
	}
	
	//render vertices: trigger shader programs to process vertex data
	if(bUseIndexBuffer)
	{
		if(index_buffer) glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );	
		glDrawElementsInstanced( GL_TRIANGLES, index_list.size(), GL_UNSIGNED_INT, nullptr, 21);
	}
	else
	{
		glDrawArrays( GL_TRIANGLES, 0, NUM_TESS*3 ); // NUM_TESS = N
	}

	// swap front and back buffers, and display to screen
	glfwSwapBuffers( window );
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	float real_ratio = (window_size.x) / (float)window_size.y;
	if (real_ratio > aspect_ratio) real_coeff = vec2(1.0f, 1.0f);
	else if (real_ratio > 1) real_coeff = vec2(aspect_ratio / real_ratio, aspect_ratio / real_ratio);
	else real_coeff = vec2(aspect_ratio, aspect_ratio);

	
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
	printf( "- press 'd' to toggle between solid color and texture coordinates\n" );
	printf( "- press '+/-' to increase/decrease tessellation factor (min=%d, max=%d)\n", MIN_TESS, MAX_TESS );
	printf( "- press 'i' to toggle between index buffering and simple vertex buffering\n" );
	printf( "- press 'w' to toggle wireframe\n" );
	printf( "\n" );
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	void update_vertex_buffer( uint N );	// forward declaration
	void update_circle_vertices( uint N );	// forward declaration
	
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_W)
		{
			bWireframe = !bWireframe;
			glPolygonMode( GL_FRONT_AND_BACK, bWireframe ? GL_LINE:GL_FILL );
			printf( "> using %s mode\n", bWireframe ? "wireframe" : "solid" );
		}
		else if(key==GLFW_KEY_D)
		{
			bUseSolidColor = !bUseSolidColor;
			printf( "> using %s\n", bUseSolidColor ? "solid color" : "texture coordinates as color" );
		}
	}
}

void mouse( GLFWwindow* window, int button, int action, int mods )
{
	if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS )
	{
		dvec2 pos; glfwGetCursorPos(window,&pos.x,&pos.y);
		printf( "> Left mouse button pressed at (%d, %d)\n", int(pos.x), int(pos.y) );
	}
}

void motion( GLFWwindow* window, double x, double y )
{
}

void update_vertex_buffer( uint N )
{
	// clear and create new buffers
	if(vertex_buffer)	glDeleteBuffers( 1, &vertex_buffer );	vertex_buffer = 0;
	if(index_buffer)	glDeleteBuffers( 1, &index_buffer );	index_buffer = 0;

	// check exceptions
	if(vertex_list.empty()){ printf("[error] vertex_list is empty.\n"); return; }

	// create buffers
	if(bUseIndexBuffer)
	{
		index_list.clear();
		for( uint k=0; k < N; k++ )
		{
			index_list.push_back(0);	// the origin
			index_list.push_back(k+1);
			index_list.push_back(k+2);
		}

		// generation of vertex buffer: use vertex_list as it is
		glGenBuffers( 1, &vertex_buffer );
		glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertex)*vertex_list.size(), &vertex_list[0], GL_STATIC_DRAW);

		// geneation of index buffer
		glGenBuffers( 1, &index_buffer );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*index_list.size(), &index_list[0], GL_STATIC_DRAW );

	}
	else
	{
		std::vector<vertex> triangle_vertices;
		for( uint k=0; k < N; k++ )
		{
			triangle_vertices.push_back(vertex_list.front());	// the origin
			triangle_vertices.push_back(vertex_list[k+1]);
			triangle_vertices.push_back(vertex_list[k+2]);
		}

		// generation of vertex buffer: use triangle_vertices instead of vertex_list
		glGenBuffers( 1, &vertex_buffer );
		glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertex)*triangle_vertices.size(), &triangle_vertices[0], GL_STATIC_DRAW);
	}
}

void update_circle_vertices( uint N )
{
	vertex_list.clear();

	// define the position of four corner vertices
	vertex_list.push_back( { vec3(0,0,0), vec3(0.0f,0.0f,-1.0f), vec2(0.5f)} );	// the origin
	for( uint k=0; k <= N; k++ )
	{
		float t = PI*2.0f/float(N)*float(k);
		float c=cos(t), s=sin(t);
		vertex_list.push_back( { vec3(c,s,0.0f) , vec3(0.0f,0.0f,-1.0f), vec2(c*0.5f+0.5f,s*0.5f+0.5f)} );
	}
}

bool user_init()
{
	// log hotkeys
	print_help();
	float real_window = (window_size.x / float(window_size.y));
	aspect_coeff = real_window > 1 ? vec2(real_window, 1) : vec2(1, 1 / real_window);
	// init GL states
	glLineWidth( 1.0f );
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests
	for (int i = 0; i < 4 * NUM_CIRCLES; i++) { solid_color[i] = ran(gen); }
	for (int i = 0; i < NUM_CIRCLES; i++) { radius[i] = ran_rad(gen); }
	
	while (true) {
		//float tmpx = ran_off(gen);
		//float tmpy = ran_off(gen);
		float tmpx = ran_off(gen) * aspect_coeff.x;
		float tmpy = ran_off(gen) * aspect_coeff.y;
		//if (!(tmpx >= 1.0f - radius[0] || tmpx <= -1.0f + radius[0] || tmpy >= 1.0f - radius[0] || tmpy <= -1.0f + radius[0])) 
		if (!(tmpx >= 1.0f * aspect_coeff.x - radius[0] || tmpx <= -1.0f * aspect_coeff.x + radius[0] || tmpy >= 1.0f * aspect_coeff.y - radius[0] || tmpy <= -1.0f * aspect_coeff.y + radius[0])) 
		{
			offset_list[0] = tmpx;
			offset_list[0 + 1] = tmpy;
			offset_list[0 + 2] = 0.0f;
			break;
		}
	}


	for (int i = 3; i < 3 * NUM_CIRCLES; i+=3) {
		while (true) {
			//float tmpx = ran_off(gen);
			//float tmpy = ran_off(gen);
			float tmpx = ran_off(gen) * aspect_coeff.x;
			float tmpy = ran_off(gen) * aspect_coeff.y;
			int j;
			for (j = 0; j < i; j += 3) {
				if (tmpx >= aspect_coeff.x - radius[i / 3] || tmpx <= -aspect_coeff.x + radius[i / 3] || tmpy >= aspect_coeff.y - radius[i / 3] || tmpy <= -aspect_coeff.y + radius[i / 3] || collision(tmpx, tmpy, offset_list[j], offset_list[j + 1], radius[i / 3] + radius[j / 3] + 0.03f)) break;
				//if (tmpx >= 1.0f - radius[i / 3] || tmpx <= -1.0f + radius[i / 3] || tmpy >= 1.0f - radius[i / 3] || tmpy <= -1.0f + radius[i / 3] || collision(tmpx, tmpy, offset_list[j], offset_list[j + 1], radius[i / 3] + radius[j / 3])) break;
			}
			if (j >= i) {
				offset_list[i] = tmpx;
				offset_list[i + 1] = tmpy;
				offset_list[i + 2] = 0.0f;
				break;
			}
		}
	}
	//*/
	for (int i = 0; i < NUM_CIRCLES; i++) {
		speed[3 * i] = ran_spd(gen) * (0.02f - radius[i] * 0.03f);
		speed[3 * i + 1] = ran_spd(gen) * (0.02f - radius[i] * 0.03f);
		speed[3*i + 2] = 0.0f;
	}//*/
	// define the position of four corner vertices
	update_circle_vertices( NUM_TESS );

	// create vertex buffer; called again when index buffering mode is toggled
	update_vertex_buffer( NUM_TESS );

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
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movements

	double r = (double)1.0 / 60.0;
	double before_time = 0;
	double timer = 0;
	// enters rendering/event loop
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
	{
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

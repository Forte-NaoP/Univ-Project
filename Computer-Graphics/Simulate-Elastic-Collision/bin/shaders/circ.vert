#version 330

// input attributes of vertices
layout (location = 0) in vec3 position;	
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
//layout (location = 3) in vec3 offset;
// outputs of vertex shader = input to fragment shader
// out vec4 gl_Position: a built-in output variable that should be written in main()
out vec3 norm;	// the second output: not used yet
out vec2 tc;	// the third output: not used yet

// uniform variables
uniform float	aspect_ratio;	// to correct a distortion of the shape
uniform float	radius[21];			// scale of a circle
uniform vec3	offset[21];
flat out int instanceID;

void main()
{
	gl_Position = vec4( position*radius[gl_InstanceID], 1 );
	gl_Position.xyz += offset[gl_InstanceID];
	gl_Position.xy *= aspect_ratio>1 ? vec2(1/aspect_ratio,1) : vec2(1,aspect_ratio);
	
	
	// another output passed via varying variable
	norm = normal;
	tc = texcoord;
	instanceID = gl_InstanceID;
}
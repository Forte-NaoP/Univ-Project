#version 400

const int NUM_SPHERE = 19;
// vertex attributes
in vec3 position;
in vec3 normal;
in vec2 texcoord;

// matrices
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform mat4 rotate_mat[NUM_SPHERE];
uniform vec3 location[NUM_SPHERE];
uniform float scale;
uniform int isRing;

out vec3 norm;
out vec2 tc;
out vec4 epos;	// eye-coordinate position

uniform float radius[NUM_SPHERE];
uniform float ring_radius[4];
flat out int instanceID;
flat out int Ring;
void main()
{
	vec4 wpos;
	if(isRing==0){
		wpos =  (rotate_mat[gl_InstanceID] * vec4(position*radius[gl_InstanceID],1));
		
	}
	else{
		wpos = rotate_mat[gl_InstanceID]*vec4(radius[2*gl_InstanceID]*(normal) + (radius[2*gl_InstanceID+1]-radius[2*gl_InstanceID])*(position), 1 );
	}
	wpos.xyz += location[gl_InstanceID];
	epos = view_matrix * wpos;
	gl_Position = projection_matrix * epos;
	// pass eye-coordinate normal to fragment shader
	norm = normalize(mat3(view_matrix * rotate_mat[gl_InstanceID])*normal);
	tc = texcoord;
	instanceID = gl_InstanceID;
	Ring = isRing;
}

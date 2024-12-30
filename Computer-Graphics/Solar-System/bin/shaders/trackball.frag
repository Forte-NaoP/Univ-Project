#version 400

// input from vertex shader
in vec3 norm;
in vec4 epos;
in vec2 tc;
flat in int instanceID;
flat in int Ring;
out vec4 fragColor;
uniform sampler2D tex[14];
uniform sampler2D bump[5];
uniform mat4	view_matrix;
uniform vec4	light_position, Ia, Id, Is;	// light
uniform vec4	Ka, Kd, Ks;					// material properties
uniform float	shininess;

const vec2 size = vec2(1.0,0.0);
const ivec3 off = ivec3(-1,0,1);

void main()
{
	//fragColor = texture2D( tex[instanceID], tc );
	vec4 bumping = vec4(0,0,0,0);
	if(Ring==0){
		if(instanceID < 9){ fragColor = texture2D( tex[instanceID], tc );}
		else{ fragColor = texture2D( tex[9], tc );}
	}
	else{
		vec4 tex1 = texture2D(tex[2*instanceID], tc);
		vec4 tex2 = texture2D(tex[2*instanceID+1], tc);
		fragColor = tex1;
		fragColor.a = tex2.x;
	}
	if(Ring == 0 && ((instanceID>0 && instanceID<5) || instanceID >= 9) ){
		float s01, s21, s10, s12, s11;
		vec4 wave;
		if(instanceID>=9){
			wave = texture2D(bump[4], tc);
			s11 = wave.x;
			s01 = textureOffset(bump[4], tc, off.xy).x;
			s21 = textureOffset(bump[4], tc, off.zy).x;
			s10 = textureOffset(bump[4], tc, off.yx).x;
			s12 = textureOffset(bump[4], tc, off.yz).x;
		}
		else{
			wave = texture2D(bump[instanceID-1], tc);
			s11 = wave.x;
			s01 = textureOffset(bump[instanceID-1], tc, off.xy).x;
			s21 = textureOffset(bump[instanceID-1], tc, off.zy).x;
			s10 = textureOffset(bump[instanceID-1], tc, off.yx).x;
			s12 = textureOffset(bump[instanceID-1], tc, off.yz).x;
		}
		vec3 va = vec3(size.xy,(s21-s01)*10.0f);
		vec3 vb = vec3(size.yx,(s12-s10)*10.0f);
		bumping = vec4( cross(va,vb), s11 );
	}
	vec4 lpos = view_matrix*light_position;

	vec3 n = normalize(norm + bumping.xyz);	// norm interpolated via rasterizer should be normalized again here
	vec3 p = epos.xyz;			// 3D position of this fragment
	vec3 l = normalize(lpos.xyz-(lpos.a==0.0?vec3(0):p));	// lpos.a==0 means directional light
	vec3 v = normalize(-p);		// eye-epos = vec3(0)-epos
	vec3 h = normalize(l+v);	// the halfway vector

	vec4 Ira = Ka*Ia;									// ambient reflection
	vec4 Ird = max(Kd*dot(l,n)*Id,0.0);					// diffuse reflection
	vec4 Irs = max(Ks*pow(dot(h,n),shininess)*Is,0.0);	// specular reflection
	vec4 shading = Ira + Ird + Irs;

	if(instanceID > 0 || Ring == 1){
		fragColor = shading * fragColor;
	}
}

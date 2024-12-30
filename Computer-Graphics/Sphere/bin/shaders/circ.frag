#version 330

// inputs from vertex shader
in vec2 tc;	// used for texture coordinate visualization

// output of the fragment shader
out vec4 fragColor;

// shader's global variables, called the uniform variables
uniform int bUseSolidColor;
uniform vec4 solid_color;

void main()
{
	fragColor = (bUseSolidColor % 3 == 0) ? vec4(tc.xy,0,1) : (bUseSolidColor % 3 == 1) ? vec4(tc.xxx,1) : vec4(tc.yyy,1);
	switch(bUseSolidColor){
		case 0:
			fragColor = vec4(tc.xy, 0, 1);
			break;
		case 1:
			fragColor = vec4(tc.xxx, 1);
			break;
		case 2:
			fragColor = vec4(tc.yyy, 1);
			break;
		default:
			fragColor = vec4(tc.xy, 0, 1);
			break;
	}
}
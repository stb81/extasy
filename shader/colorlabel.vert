varying vec2 coord;
varying vec3 color;

void main()
{
	gl_Position=ftransform();
	coord=gl_MultiTexCoord0.xy;
	color=gl_Color.rgb;
}


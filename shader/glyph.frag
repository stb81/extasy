#extension GL_EXT_texture_array:require

uniform sampler2DArray font;
varying vec2 texcoord;
varying vec4 color_frag;

void main()
{
	if (texture2DArray(font, vec3(texcoord, color_frag.a)).r < 0.5) discard;
	
	gl_FragColor=color_frag;
}


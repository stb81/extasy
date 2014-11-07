#extension GL_EXT_texture_array:require

uniform sampler2DArray font;
uniform sampler2DRect text;

uniform vec4 bgcolor;

varying vec2 coord;

void main()
{
	vec4 chr=texture2DRect(text, vec2(floor(coord.x), floor(coord.y)));
	float val=texture2DArray(font, vec3(fract(coord.x),fract(coord.y), chr.a*255.0)).r;

	gl_FragColor=mix(bgcolor, vec4(chr.rgb*val, 1.0), val);
}


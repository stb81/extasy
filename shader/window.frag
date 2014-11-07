uniform sampler2D background;

varying vec2 coord;

const vec2 scale=vec2(1.0/256.0, 1.0/128.0);
const vec4 wndcol=vec4(0.5,0.75,1.0,0.0);

float sqr(float v)
{
	return v*v;
}

float cube(float v)
{
	return v*v;
}

void main()
{
	vec2 warped=coord + coord*(coord-vec2(0.5,0.5))*(coord-vec2(1,1));

	float v=max(2.0*coord.y-1.0, 0.0);
	v*=v*coord.x;
	
	v+=sqr(max(0.8-coord.x, 0.0) * max(0.5-coord.y, 0.0)) * coord.x * coord.y * 128.0;

	gl_FragColor=(
		texture2D(background, warped)*2.0 +
		texture2D(background, warped + scale*vec2(-0.5,-0.5)) +
		texture2D(background, warped + scale*vec2(-0.5, 0.5)) +
		texture2D(background, warped + scale*vec2( 0.5,-0.5)) +
		texture2D(background, warped + scale*vec2( 0.5, 0.5)) +
		texture2D(background, warped + scale*vec2(-1.5, 0.0)) +
		texture2D(background, warped + scale*vec2( 1.5, 0.0)) +
		texture2D(background, warped + scale*vec2(0.0, -1.5)) +
		texture2D(background, warped + scale*vec2(0.0,  1.5)) +
		vec4(1,1,1,1)*(6.0+12.0*v)) * 0.0625 * wndcol;
}


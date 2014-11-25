uniform float display_value;
uniform float length;
uniform vec3 color_scheme[4];

varying vec2 coord;

float border(float v)
{
	float a=clamp(v*0.125, 0.0, 1.0);
	a*=a*(3.0-2.0*a);
	return a;
}

void main()
{
	float alpha=border(coord.x) * border(length-coord.x) * border(coord.y) * border(16.0-coord.y);
	alpha*=alpha*(3.0-2.0*alpha);
	
	float beta=border(coord.x) * border(display_value-coord.x) * border(coord.y) * border(16.0-coord.y);
	beta*=beta*(3.0-2.0*beta);
	beta=max((beta-0.5) * 2.0, 0.0);
	beta*=beta;
	
	float lambda=4.0*coord.x/length;
	
	vec3 color;
	if (lambda<1)
		color=mix(color_scheme[0], mix(color_scheme[0], color_scheme[1], lambda*0.5), lambda);
	else if (lambda<2)
		color=mix(mix(color_scheme[0], color_scheme[1], lambda*0.5), mix(color_scheme[1], color_scheme[2], (lambda-1.0)*0.5), lambda-1.0);
	else if (lambda<3)
		color=mix(mix(color_scheme[1], color_scheme[2], (lambda-1.0)*0.5), mix(color_scheme[2], color_scheme[3], (lambda-2.0)*0.5), lambda-2.0);
	else
		color=mix(mix(color_scheme[2], color_scheme[3], (lambda-2.0)*0.5), color_scheme[3], lambda-3.0);
	
	vec3 framecol=mix(vec3(0.6, 0.9, 1.2) * (16.0-coord.y) / 16.0, color*beta, alpha);
	
	gl_FragColor=vec4(framecol, alpha);
}


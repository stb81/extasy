varying vec2 coord;
varying vec3 color;

uniform vec3 shadefactors;

void main()
{
	float v=16.0*coord.x*coord.y*(1.0-coord.x)*(1.0-coord.y);
	
	gl_FragColor=vec4(mix(color*(shadefactors.r+shadefactors.g*sqrt(v)), vec3(1.0), v*v*v*shadefactors.b), 1.0);
}


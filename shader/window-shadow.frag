uniform vec2 wndsize;

varying vec2 coord;

float scurve(float x)
{
	return x*x*x*(x*(x*6.0-15.0)+10.0);
}

void main()
{
	float alpha=0.5;
	
	if (coord.x<32.0)
		alpha*=scurve(coord.x/32.0);
	else if (coord.x>wndsize.x)
		alpha*=scurve((wndsize.x+32.0-coord.x)/32.0);
	
	if (coord.y<32.0)
		alpha*=scurve(coord.y/32.0);
	else if (coord.y>wndsize.y)
		alpha*=scurve((wndsize.y+32.0-coord.y)/32.0);
	
	gl_FragColor=vec4(0,0,0,alpha);
}


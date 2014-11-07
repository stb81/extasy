uniform vec2 origin;

varying vec2 coord;

void main()
{
	gl_Position=gl_ModelViewProjectionMatrix * vec4(origin.x+gl_Vertex.x*8, origin.y+gl_Vertex.y*16, 0, 1);
	coord=gl_Vertex.xy;
}


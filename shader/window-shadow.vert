uniform vec2 wndorigin;
uniform vec2 wndsize;

varying vec2 coord;

void main()
{
	coord=(wndsize + vec2(32, 32))*gl_Vertex.xy;
	gl_Position=gl_ModelViewProjectionMatrix * vec4(wndorigin + coord - vec2(8, 8), 0, 1);
}


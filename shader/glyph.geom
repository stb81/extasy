#extension GL_EXT_geometry_shader4 : require

uniform vec2 maxglyphsize;

varying in vec4 color_vert[];

varying out vec4 color_frag;
varying out vec2 texcoord;

void main()
{
	vec4 black=vec4(0,0,0,color_vert[0].a);
	
	gl_Position=gl_ModelViewProjectionMatrix * (gl_PositionIn[0] + vec4(1,1,0,0));
	color_frag=black;
	texcoord=vec2(0,0);
	EmitVertex();

	gl_Position=gl_ModelViewProjectionMatrix * (gl_PositionIn[0] + vec4(1+maxglyphsize.x,1,0,0));
	color_frag=black;
	texcoord=vec2(1,0);
	EmitVertex();

	gl_Position=gl_ModelViewProjectionMatrix * (gl_PositionIn[0] + vec4(1,1+maxglyphsize.y,0,0));
	color_frag=black;
	texcoord=vec2(0,1);
	EmitVertex();

	gl_Position=gl_ModelViewProjectionMatrix * (gl_PositionIn[0] + vec4(vec2(1,1)+maxglyphsize,0,0));
	color_frag=black;
	texcoord=vec2(1,1);
	EmitVertex();
	EndPrimitive();
	
	gl_Position=gl_ModelViewProjectionMatrix * (gl_PositionIn[0] + vec4(0,0,0,0));
	color_frag=color_vert[0];
	texcoord=vec2(0,0);
	EmitVertex();

	gl_Position=gl_ModelViewProjectionMatrix * (gl_PositionIn[0] + vec4(maxglyphsize.x,0,0,0));
	color_frag=color_vert[0];
	texcoord=vec2(1,0);
	EmitVertex();

	gl_Position=gl_ModelViewProjectionMatrix * (gl_PositionIn[0] + vec4(0,maxglyphsize.y,0,0));
	color_frag=color_vert[0];
	texcoord=vec2(0,1);
	EmitVertex();

	gl_Position=gl_ModelViewProjectionMatrix * (gl_PositionIn[0] + vec4(maxglyphsize,0,0));
	color_frag=color_vert[0];
	texcoord=vec2(1,1);
	EmitVertex();
}


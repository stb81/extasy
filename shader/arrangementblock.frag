uniform sampler1D blocktex;
uniform float highlight;

varying vec2 texcoord;

void main()
{
	vec4 col=texture1D(blocktex, texcoord.y);
	
	float v=mix(mix(1.0, col.a, texcoord.x), mix(col.a, 0.0, texcoord.x), texcoord.x);
	
	gl_FragColor=vec4(mix(vec3(0.1,0.1,0.1), col.rgb, v) * highlight, 1.0);
}

uniform sampler2D spectrum;

varying vec2 texcoord;

const vec4 C0=vec4(0,0,0,0);
const vec4 C1=vec4(0,0,0.75,0);
const vec4 C2=vec4(1.0,0,0.25,0);
const vec4 C3=vec4(1.0,1.0,0.0,0);
const vec4 C4=vec4(1.0,1.0,1.0,0);

void main()
{
	float v=texture2D(spectrum, texcoord).r;
	//float v=texture2D(spectrum, vec2(pow(512.0, texcoord.x-1.0), texcoord.y)).r;
	
	v=(log(sqrt(v/2048) + 0.0001) - log(0.0001)) * 0.5;
	
	if (v<0.0)
		gl_FragColor=C0;
	else if (v<1.0)
		gl_FragColor=mix(C0, mix(C0, C1, v*0.5), v);
	else if (v<2.0)
		gl_FragColor=mix(mix(C0, C1, v*0.5), mix(C1, C2, (v-1.0)*0.5), v-1.0);
	else if (v<3.0)
		gl_FragColor=mix(mix(C1, C2, (v-1.0)*0.5), mix(C2, C3, (v-2.0)*0.5), v-2.0);
	else if (v<4.0)
		gl_FragColor=mix(mix(C2, C3, (v-2.0)*0.5), mix(C3, C4, (v-3.0)*0.5), v-3.0);
	else if (v<5.0)
		gl_FragColor=mix(mix(C3, C4, (v-3.0)*0.5), C4, v-4.0);
	else
		gl_FragColor=C4;
}

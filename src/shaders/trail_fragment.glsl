#version 410 core

out vec4 color;
//in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
// uniform vec3 campos;

uniform sampler2D tex;
uniform float glTime;

void main()
{
	//vec3 n = normalize(vertex_normal);
	//vec3 lp=vec3(10,-20,-100);
	//vec3 ld = normalize(vertex_pos - lp);
	//float diffuse = dot(n,ld);

	//vec4 tcol = texture(tex, vertex_tex);
	// get rid of the background color of the texture in tex
	//tcol.a = (tcol.r + tcol.g + tcol.b) / 3.f;
	//tcol.a = pow(tcol.a, 3);
	//color = tcol;
	color = vec4(1);
}

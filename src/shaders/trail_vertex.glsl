#version 410 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout (location = 3) in vec4 InstancePos;
layout (location = 4) in float EnterTime;

uniform float glTime;
uniform mat4 P;
uniform mat4 V;

out vec3 vertex_pos;
out vec2 vertex_tex;

void main()
{
	gl_Position = P * V * InstancePos;
	vertex_tex = vertTex;

	vertex_pos = vec3(P * V * InstancePos);
	//gl_Position = vec4(0, 0, 0, 1);
}

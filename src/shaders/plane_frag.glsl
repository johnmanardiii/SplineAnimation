#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex;

void main()
{
vec3 lightpos = vec3(100,100,100);
vec3 lightdir = normalize(lightpos - vertex_pos);
vec3 camdir = normalize(campos - vertex_pos);
vec3 frag_norm = normalize(vertex_normal);

float diffuse_fact = clamp(dot(lightdir,frag_norm),0,1);
diffuse_fact = 0.3 + diffuse_fact * 0.7;

vec3 half = normalize(camdir + lightdir);
float spec_fact = clamp(dot(half,frag_norm),0,1);

vec4 tcol = texture(tex, vec2(vertex_tex.x,1-vertex_tex.y)); //the plane has reversed texture coordinates in y... blame the model!
//color = tcol;

color.rgb =  tcol.rgb*diffuse_fact + vec3(0.6, 0.5882, 0.5882)*spec_fact;
color.a=1;

}

#version 330 core
out vec4 color;
in vec3 frag_pos;
in vec2 frag_tex;
in vec3 frag_norm;

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec3 camoff;
uniform vec3 campos;
uniform vec3 bgcolor;
uniform int renderstate;

void main()
{

vec2 texcoords=frag_tex;
float t=1./1000.;
texcoords -= vec2(camoff.x,camoff.z)*t;

vec3 heightcolor = texture(tex, texcoords).rgb;
heightcolor.r = 0.1 + heightcolor.r*0.9;
color.rgb = texture(tex2, texcoords*50).rgb * heightcolor.r;
color.a=1;

float len = length(frag_pos.xz+campos.xz);
len-=350;
len/=140.;
len=clamp(len,0,1);

vec3 lp=vec3(100,-100,100);
vec3 ld = normalize(lp - frag_pos);
ld = vec3(1,-1,1);
float light = dot(frag_norm,ld);
light=clamp(light,0,1);

vec3 cd = normalize(campos - frag_pos);
vec3 h = normalize(ld+cd);
float spec = dot(frag_norm,h);
spec=clamp(spec,0,1);

float ambientlight = 0.20+light*0.95;
float heightthreshhold = 15.0f;
vec3 mountainBaseColor = vec3(0.839,0.592,0.133);
const float grass = .60;
if(light > grass)
{
	mountainBaseColor = mix(mountainBaseColor, 
			normalize(vec3(.3176, .75, .13333) * light) * .40f, .5);
}

if(frag_pos.y > heightthreshhold)
{
	float diff = frag_pos.y - heightthreshhold;
	const float range = 15.0f;
	diff = diff / range;
	mountainBaseColor = mix(mountainBaseColor, vec3(1, 1, 1), diff);
}
vec3 mountainFinalColor = mountainBaseColor;
color.rgb = ambientlight*mountainBaseColor;

color.a=1;//1-len;
if(renderstate==2)
	color=vec4(0,0,0,1);	
color.rgb = (color.rgb*0.8+ vec3(1,1,1)*spec*0.5)*(1-len) + bgcolor*len ;
color.a=1;
}

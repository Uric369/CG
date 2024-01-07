#version 330 core
layout (location = 3)in vec3 position;
layout (location = 4)in float alpha;
layout (location = 5)in float size;
layout (location = 6)in float lifetimeMills;
layout (location = 7)in float life;

out vec3 pos;
out float Alpha;
out float Age;
out float Life;
out float Size;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main()
{
	pos = position;
	gl_PointSize = size;
	gl_Position = projection * view * model * vec4(position,1.0f);
    Alpha = alpha;
    Age = lifetimeMills;
    Life = life;
    Size =size;
}

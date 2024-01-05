#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform vec3 displacement;

void main()
{
    gl_Position = model * vec4(aPos + displacement, 1.0);
}
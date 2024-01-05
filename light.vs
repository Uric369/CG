#version 330 core
layout (location = 0) in vec3 aPos;

out VS_OUT {
    vec2 texCoords;
} vs_out;

// 传递给片段着色器的输出
out vec2 TexCoords; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{  
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

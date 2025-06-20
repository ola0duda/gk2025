#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; //normalne są w danych, ale nie używane są do oświetlenia
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 camMatrix; //View * Projection

void main()
{
    TexCoords = aTexCoords;
    gl_Position = camMatrix * model * vec4(aPos, 1.0);
}
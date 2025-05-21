#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aNormal;
out vec2 texCoord;
out vec3 FragPos_world;
out vec3 Normal_world;
uniform mat4 camMatrix;
uniform mat4 model;
void main()
{
FragPos_world = vec3(model * vec4(aPos, 1.0f));
gl_Position = camMatrix * vec4(FragPos_world, 1.0f);
texCoord = aTex;
Normal_world = normalize(mat3(transpose(inverse(model))) * aNormal);
} 
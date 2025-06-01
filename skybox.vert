#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view; // Macierz widoku bez translacji

void main()
{
    TexCoords = aPos;
    // Usuń translację z macierzy widoku przed mnożeniem
    mat4 viewNoTranslation = mat4(mat3(view));
    vec4 pos = projection * viewNoTranslation * vec4(aPos, 1.0);
    // Zapewnij, że skybox jest zawsze na dalekiej płaszczyźnie przycinania (z = w)
    gl_Position = pos.xyww;
}
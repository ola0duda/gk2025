#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sunTexture;
uniform vec4 sunColor; //dodatkowy kolor, gdyby tekstura była modulowana

void main()
{
    FragColor = texture(sunTexture, TexCoords) * sunColor;	//pobranie koloru z tekstury
}
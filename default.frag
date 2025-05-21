// default.frag (POPRAWIONY)
#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 FragPos_world;
in vec3 Normal_world;

uniform sampler2D tex0;

uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;
uniform int u_lightingMode;

void main()
{
    vec3 objectBaseColor = texture(tex0, texCoord).rgb;

    //obliczenia komponentów modelu Phonga
    //swiatlo ambientowe - ambient light
    float ambientStrength = 0.20f;		//sila swiatla ambientowego
    vec3 ambientComponent = ambientStrength * lightColor.rgb;

    //swiatlo rozproszone - diffuse light
    vec3 norm = normalize(Normal_world);
    vec3 lightDir = normalize(lightPos - FragPos_world);		//kierunek swiatla
    float diffFactor = max(dot(norm, lightDir), 0.0f);		//wspolczynnik rozproszenia
    vec3 diffuseComponent = diffFactor * lightColor.rgb;

    //swiatlo odbite - specular light
    float specularStrength = 0.7f;			//sila odbicia lustrzanego
    int shininess = 64;		//im wyższa wartość błyszczenia, tym mniejsze i ostrzejsze odbicie
    vec3 viewDir = normalize(camPos - FragPos_world);			//kierunek od fragmentu do kamery
    vec3 reflectDir = reflect(-lightDir, norm);		//wektor odbicia światła
    float specFactor = pow(max(dot(viewDir, reflectDir), 0.0f), float(shininess));
    vec3 specularComponent = specularStrength * specFactor * lightColor.rgb;		//odbicie kolorem swiatla

    //wybor i zastosowanie modelu oswietlenia
    vec3 finalColor;

    if (u_lightingMode == 0)
    {
        finalColor = ambientComponent * objectBaseColor;
    }
    else if (u_lightingMode == 1)
    {
       finalColor = diffuseComponent * objectBaseColor;
    }
    else if (u_lightingMode == 2)
    {
        finalColor = (ambientComponent * objectBaseColor) + specularComponent;
    }
    else		//pelne oswietlenie (ambient + diffuse + specular) - klawisz 4
    {
       finalColor = (ambientComponent + diffuseComponent) * objectBaseColor + specularComponent;
    }
    FragColor = vec4(finalColor, 1.0f);
}
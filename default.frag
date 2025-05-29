// default.frag (POPRAWIONY z uniformem specular)
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

// --- Dodany uniform dla kontroli błyszczenia ---
uniform float u_specularStrength;
// ------------------------------------------------

// (Jeśli wcześniej dodałeś uniformy debugujace, możesz je zachować lub usunąć,
// upewnij się tylko, że nie są aktywne podczas normalnego renderowania)
// uniform bool u_debugOverrideColor;
// uniform vec3 u_debugColor;


void main()
{
    // --- Jeśli miałeś kod debugujący, pozostaw go na początku ---
    // if (u_debugOverrideColor) {
    //    FragColor = vec4(u_debugColor, 1.0f);
    //    return;
    // }
    // -----------------------------------------------------------


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
    // Zmieniono: Używamy uniformu u_specularStrength zamiast stałej
    // float specularStrength = 0.7f; // Usunięto tę stałą
    int shininess = 64;		//im wyższa wartość błyszczenia, tym mniejsze i ostrzejsze odbicie
    vec3 viewDir = normalize(camPos - FragPos_world);			//kierunek od fragmentu do kamery
    vec3 reflectDir = reflect(-lightDir, norm);		//wektor odbicia światła
    float specFactor = pow(max(dot(viewDir, reflectDir), 0.0f), float(shininess));
    // Zmieniono: Mnożymy przez uniform u_specularStrength
    vec3 specularComponent = u_specularStrength * specFactor * lightColor.rgb;

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
        // Tryb 2 to Specular (+Ambient)
        finalColor = (ambientComponent * objectBaseColor) + specularComponent;
    }
    else		//pelne oswietlenie (ambient + diffuse + specular) - klawisz 4
    {
       finalColor = (ambientComponent + diffuseComponent) * objectBaseColor + specularComponent;
    }
    FragColor = vec4(finalColor, 1.0f);
}
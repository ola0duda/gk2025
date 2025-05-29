#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <random> // Do losowego obrotu kaktusów
#include <ctime>  // Do inicjalizacji generatora losowego

// Upewnij się, że te pliki nagłówkowe są dostępne w ścieżce kompilatora
#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Camera.h"
#include "Texture.h"
// === Dodanie nagłówka nowej klasy Cactus ===
#include "Cactus.h"
// =========================================

//funkcja pomocnicza do generowania wierzcho�k�w sfery UV
// Format wierzchołka sfery: Pos(3) + Normal(3) + TexCoord(2) = 8 floats
void generateSphere(float radius, int sectorCount, int stackCount,
    std::vector<GLfloat>& outSphereVertices, std::vector<GLuint>& outSphereIndices)
{
    outSphereVertices.clear();
    outSphereIndices.clear();

    float x, y, z, xy;                              //vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    //vertex normal
    float s, t;                                     //vertex texCoord

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;      //starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             //r * cos(u)
        z = radius * sinf(stackAngle);              //r * sin(u)

        //add (sectorCount+1) outSphereVertices per stack
        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           //starting from 0 to 2pi

            //vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             //r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             //r * cos(u) * sin(v)
            outSphereVertices.push_back(x);
            outSphereVertices.push_back(y);
            outSphereVertices.push_back(z);

            //normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            outSphereVertices.push_back(nx);
            outSphereVertices.push_back(ny);
            outSphereVertices.push_back(nz);

            //vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            outSphereVertices.push_back(s);
            outSphereVertices.push_back(t);
        }
    }

    //generate CCW index list of sphere triangles
    int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     //beginning of current stack
        k2 = k1 + sectorCount + 1;      //beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            //2 triangles per sector excluding first and last stacks
            //k1 => k2 => k1+1
            if (i != 0)
            {
                outSphereIndices.push_back(k1);
                outSphereIndices.push_back(k2);
                outSphereIndices.push_back(k1 + 1);
            }

            //k1+1 => k2 => k2+1
            if (i != (stackCount - 1))
            {
                outSphereIndices.push_back(k1 + 1);
                outSphereIndices.push_back(k2);
                outSphereIndices.push_back(k2 + 1);
            }
        }
    }
     std::cout << "Generated Sphere: " << outSphereVertices.size() / 8 << " vertices, " << outSphereIndices.size() / 3 << " triangles." << std::endl;
}

// Funkcja pomocnicza do obliczania wysokości dla punktu (x, z)
float getHeight(float x, float z, float amplitude, float frequency) {
	float h = 0.0f;
	h += amplitude * sin((x + z * 0.5f) * frequency);
	h += (amplitude * 0.4f) * cos((x - z * 0.8f) * frequency * 1.5f);
	h += (amplitude * 0.15f) * sin((x * 2.5f + z * 1.5f) * frequency * 2.0f);
	h += (amplitude * 0.08f) * cos((z * 3.0f - x * 0.7f) * frequency * 3.0f);
	float total_coeffs = 1.0f + 0.4f + 0.15f + 0.08f;
	h /= total_coeffs;
	return h;
}

// Funkcja pomocnicza do obliczania normalnej numerycznie
glm::vec3 calculateNormal(float x, float z, float epsilon, float amplitude, float frequency) {
	float y_center = getHeight(x, z, amplitude, frequency);
	float y_dx = getHeight(x + epsilon, z, amplitude, frequency);
	float y_dz = getHeight(x, z + epsilon, amplitude, frequency);

	glm::vec3 tangentX = glm::vec3(epsilon, y_dx - y_center, 0.0f);
	glm::vec3 tangentZ = glm::vec3(0.0f, y_dz - y_center, epsilon);

	glm::vec3 normal = glm::normalize(glm::cross(tangentZ, tangentX));
	return normal;
}

// Funkcja, która generuje dane dla pofałdowanej siatki
void generateWavyGround(int segmentsX, int segmentsZ, float totalWidth, float totalDepth,
	float waveAmplitude, float waveFrequency, float textureTiling,
	std::vector<GLfloat>& outGroundVertices, std::vector<GLuint>& outGroundIndices)
{
	outGroundVertices.clear();
	outGroundIndices.clear();

	float segmentWidth = totalWidth / segmentsX;
	float segmentDepth = totalDepth / segmentsZ;
	float epsilon = 0.005f;

	for (int i = 0; i <= segmentsZ; ++i)
	{
		for (int j = 0; j <= segmentsX; ++j)
		{
			float x = (float)j * segmentWidth - totalWidth * 0.5f;
			float z = (float)i * segmentDepth - totalDepth * 0.5f;
			float y = getHeight(x, z, waveAmplitude, waveFrequency);
			float r = 1.0f, g = 1.0f, b = 1.0f;
			float s = (float)j / segmentsX * textureTiling;
			float t = (float)i / segmentsZ * textureTiling;
			glm::vec3 normal = calculateNormal(x, z, epsilon, waveAmplitude, waveFrequency);

			// Format: pos(3) + color(3) + tex(2) + normal(3) = 11 floats
			outGroundVertices.push_back(x); outGroundVertices.push_back(y); outGroundVertices.push_back(z);
			outGroundVertices.push_back(r); outGroundVertices.push_back(g); outGroundVertices.push_back(b); // Dummy color, not used by shader
			outGroundVertices.push_back(s); outGroundVertices.push_back(t);
			outGroundVertices.push_back(normal.x); outGroundVertices.push_back(normal.y); outGroundVertices.push_back(normal.z);
		}
	}

	int verticesPerSegmentRow = segmentsX + 1;
	for (int i = 0; i < segmentsZ; ++i)
	{
		for (int j = 0; j < segmentsX; ++j)
		{
			int vertexIndex_BL = i * verticesPerSegmentRow + j;
			int vertexIndex_BR = i * verticesPerSegmentRow + j + 1;
			int vertexIndex_TL = (i + 1) * verticesPerSegmentRow + j;
			int vertexIndex_TR = (i + 1) * verticesPerSegmentRow + j + 1;

			outGroundIndices.push_back(vertexIndex_BL); outGroundIndices.push_back(vertexIndex_BR); outGroundIndices.push_back(vertexIndex_TR);
			outGroundIndices.push_back(vertexIndex_BL); outGroundIndices.push_back(vertexIndex_TR); outGroundIndices.push_back(vertexIndex_TL);
		}
	}
	std::cout << "Generated Wavy Ground: " << outGroundVertices.size() / 11 << " vertices, " << outGroundIndices.size() / 3 << " triangles." << std::endl;
}


//globalna zmienna trybu o�wietlenia
static int currentLightingMode = 3; //domy�lnie pe�ne o�wietlenie

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_1) { currentLightingMode = 0; std::cout << "Tryb: Ambient" << std::endl; }
		else if (key == GLFW_KEY_2) { currentLightingMode = 1; std::cout << "Tryb: Diffuse" << std::endl; }
		else if (key == GLFW_KEY_3) { currentLightingMode = 2; std::cout << "Tryb: Specular (+Ambient)" << std::endl; }
		else if (key == GLFW_KEY_4) { currentLightingMode = 3; std::cout << "Tryb: Pe�ne (ADS)" << std::endl; }
		else if (key == GLFW_KEY_L)
		{
			currentLightingMode = (currentLightingMode + 1) % 4;
			if (currentLightingMode == 0) std::cout << "Tryb: Ambient" << std::endl;
			else if (currentLightingMode == 1) std::cout << "Tryb: Diffuse" << std::endl;
			else if (currentLightingMode == 2) std::cout << "Tryb: Specular (+Ambient)" << std::endl;
			else if (currentLightingMode == 3) std::cout << "Tryb: Pe�ne (ADS)" << std::endl;
		}
	}
}

//dane wierzcho�k�w (wspolrzedne, kolory, tekstury, normalne) dla piramidy (stałe)
GLfloat pyramidVertices[] =
{
	// Pozycje           // Kolory         // TexCoords   // Normalne
	-0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,  0.0f, 0.0f,   0.0f, -1.0f, 0.0f, // dol 0
	-0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,  0.0f, 5.0f,   0.0f, -1.0f, 0.0f, // dol 1
	 0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,  5.0f, 5.0f,   0.0f, -1.0f, 0.0f, // dol 2
	 0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,  5.0f, 0.0f,   0.0f, -1.0f, 0.0f, // dol 3

	-0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,  0.0f, 0.0f,  -0.8f, 0.5f, 0.0f, // lewa 4
	-0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,  5.0f, 0.0f,  -0.8f, 0.5f, 0.0f, // lewa 5
	 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,  2.5f, 5.0f,  -0.8f, 0.5f, 0.0f, // lewa 6

	-0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,  5.0f, 0.0f,   0.0f, 0.5f,-0.8f, // tyl 7
	 0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,  0.0f, 0.0f,   0.0f, 0.5f,-0.8f, // tyl 8
	 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,  2.5f, 5.0f,   0.0f, 0.5f,-0.8f, // tyl 9

	 0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,  0.0f, 0.0f,   0.8f, 0.5f, 0.0f, // prawa 10
	 0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,  5.0f, 0.0f,   0.8f, 0.5f, 0.0f, // prawa 11
	 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,  2.5f, 5.0f,   0.8f, 0.5f, 0.0f, // prawa 12

	 0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,  5.0f, 0.0f,   0.0f, 0.5f, 0.8f, // przod 13
	-0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,  0.0f, 0.0f,   0.0f, 0.5f, 0.8f, // przod 14
	 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,  2.5f, 5.0f,   0.0f, 0.5f, 0.8f  // przod 15
};

GLuint pyramidIndices[] =
{
	0, 1, 2, 0, 2, 3, //dol
	4, 6, 5,          //lewa
	7, 9, 8,          //tyl
	10, 12, 11,       //prawa
	13, 15, 14        //przod
};

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Projekt OpenGL", NULL, NULL);
	if (window == NULL) { std::cout << "Nie uda�o si� utworzy� okna GLFW" << std::endl; glfwTerminate(); return -1; }
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Nie uda�o si� zainicjalizowa� GLAD" << std::endl; return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glfwSetKeyCallback(window, key_callback);

	// Zainicjalizuj generator liczb losowych dla losowych obrotów kaktusów
	std::srand(static_cast<unsigned int>(std::time(0)));
	std::mt19937 rng(static_cast<unsigned int>(std::time(0)));
	std::uniform_real_distribution<float> dist(0.0f, 360.0f); // Rozkład na 0-360 stopni


	// === DEKLARACJE I INICJALIZACJA WSZYSTKICH OBIEKTÓW PRZED P�TLĄ GŁÓWNĄ ===

	Camera camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 2.0f, 10.0f)); // lekko podniesiona i dalej kamera

	Shader pyramidShaderProgram("default.vert", "default.frag"); // Uzywamy tego samego shadera dla podlogi, piramid i kaktusów
	if (pyramidShaderProgram.ID == 0) { std::cerr << "Shader 'default' nie za�adowany poprawnie." << std::endl; glfwTerminate(); return -1; }

	Shader sunShaderProgram("sun.vert", "sun.frag");
	if (sunShaderProgram.ID == 0) { std::cerr << "Shader 'sun' nie za�adowany poprawnie." << std::endl; pyramidShaderProgram.Delete(); glfwTerminate(); return -1; }

	Texture pyramidTexture("sand_texture.png", GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE);
	if (pyramidTexture.ID == 0) {
		std::cerr << "Tekstura 'sand_texture.png' nie za�adowana poprawnie." << std::endl;
		pyramidShaderProgram.Delete(); sunShaderProgram.Delete();
		glfwTerminate(); return -1;
	}
	pyramidTexture.texUnit(pyramidShaderProgram, "tex0");

	Texture sunTexture("sun_texture.png", GL_TEXTURE_2D, 1, GL_RGBA, GL_UNSIGNED_BYTE);
	if (sunTexture.ID == 0) {
		std::cerr << "Tekstura 'sun_texture.png' nie za�adowana poprawnie." << std::endl;
		pyramidShaderProgram.Delete(); sunShaderProgram.Delete(); pyramidTexture.Delete();
		glfwTerminate(); return -1;
	}
	sunTexture.texUnit(sunShaderProgram, "sunTexture");

	Texture groundSandTexture("groundSand_texture.png", GL_TEXTURE_2D, 2, GL_RGBA, GL_UNSIGNED_BYTE);
	if (groundSandTexture.ID == 0) {
		std::cerr << "Tekstura 'groundSand_texture.png' nie za�adowana poprawnie." << std::endl;
		pyramidShaderProgram.Delete(); sunShaderProgram.Delete(); pyramidTexture.Delete(); sunTexture.Delete();
		glfwTerminate(); return -1;
	}

	// === Dodanie tekstury dla kaktusów (jednostka 3) - Używamy JPG ===
	// Pamiętaj, że biblioteka stb_image musi być skonfigurowana do ładowania JPG!
	Texture cactusTexture("cactus_texture.jpg", GL_TEXTURE_2D, 3, GL_RGBA, GL_UNSIGNED_BYTE);
	if (cactusTexture.ID == 0) {
		std::cerr << "Tekstura 'cactus_texture.jpg' nie za�adowana poprawnie." << std::endl;
		pyramidShaderProgram.Delete(); sunShaderProgram.Delete(); pyramidTexture.Delete(); sunTexture.Delete(); groundSandTexture.Delete();
		glfwTerminate(); return -1;
	}
	// TexUnit dla kaktusa bedzie ustawiane w p�tli przed rysowaniem kaktusów


	// === Generowanie Danych dla Pofałdowanej Podłogi ===
	std::vector<GLfloat> groundVerticesVec;
	std::vector<GLuint> groundIndicesVec;

	// Parametry generowania podłogi
	int segmentsX = 60;
	int segmentsZ = 60;
	float totalGroundWidth = 6.0f; // Mniejsza powierzchnia
	float totalGroundDepth = 6.0f;
	float waveAmplitude = 0.25f;
	float waveFrequency = 0.8f;
	float textureTiling = 8.0f;

	generateWavyGround(segmentsX, segmentsZ, totalGroundWidth, totalGroundDepth,
		waveAmplitude, waveFrequency, textureTiling,
		groundVerticesVec, groundIndicesVec);

	// Przechowujemy te parametry, żeby użyć ich do obliczenia wysokości kaktusów
	float groundGenWaveAmplitude = waveAmplitude;
	float groundGenWaveFrequency = waveFrequency;


	// === Konfiguracja VAO, VBO, EBO dla Pofałdowanej Podłogi ===
	VAO groundVAO;
	groundVAO.Bind();
	VBO groundVBO(groundVerticesVec.data(), groundVerticesVec.size() * sizeof(GLfloat));
	EBO groundEBO(groundIndicesVec.data(), groundIndicesVec.size() * sizeof(GLuint));
	// Linkowanie atrybutów (format: pos(3) color(3) tex(2) normal(3) = 11 floats)
	groundVAO.LinkAttrib(groundVBO, 0, 3, GL_FLOAT, 11 * sizeof(float), (void*)0); // aPos
	groundVAO.LinkAttrib(groundVBO, 1, 3, GL_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float))); // aColor (dummy)
	groundVAO.LinkAttrib(groundVBO, 2, 2, GL_FLOAT, 11 * sizeof(float), (void*)(6 * sizeof(float))); // aTex
	groundVAO.LinkAttrib(groundVBO, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float))); // aNormal
	// groundVAO.Unbind(); // Usunięto Unbind


	// === Konfiguracja VAO, VBO, EBO dla piramidy (oryginalna) ===
	VAO pyramidVAO;
	pyramidVAO.Bind();
	VBO pyramidVBO(pyramidVertices, sizeof(pyramidVertices));
	EBO pyramidEBO(pyramidIndices, sizeof(pyramidIndices));
	//atrybuty: 0-pozycja, 1-kolor, 2-wsp. tekstury, 3-normalne (stride 11 floats)
	pyramidVAO.LinkAttrib(pyramidVBO, 0, 3, GL_FLOAT, 11 * sizeof(float), (void*)0); // aPos
	pyramidVAO.LinkAttrib(pyramidVBO, 1, 3, GL_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float))); // aColor
	pyramidVAO.LinkAttrib(pyramidVBO, 2, 2, GL_FLOAT, 11 * sizeof(float), (void*)(6 * sizeof(float))); // aTex
	pyramidVAO.LinkAttrib(pyramidVBO, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float))); // aNormal
	// pyramidVAO.Unbind(); // Usunięto Unbind


	// === Konfiguracja VAO, VBO, EBO dla sfery (współdzielona geometria) ===
	// Używana do budowy KAKTUSÓW ORAZ SŁOŃCA
	std::vector<GLfloat> sphereVertices;
	std::vector<GLuint> sphereIndices;
	float baseSphereRadius = 0.5f; // Promień bazowej sfery (przed skalowaniem)
	generateSphere(baseSphereRadius, 36, 18, sphereVertices, sphereIndices); // Generujemy raz dane dla sfery
    // Przechowujemy liczbę indeksów sfery do rysowania
    GLsizei sphereIndexCount = sphereIndices.size();


	// === Konfiguracja VAO/VBO/EBO dla sfery UŻYWANEJ PRZEZ DEFAULT.VERT (KAKTUSY) ===
	// Shader default.vert oczekuje atrybutów w layoutach 0 (pos), 1 (color), 2 (tex), 3 (normal).
	// Dane sfery mają format pos (3) + normal (3) + tex (2) = 8 floats na wierzchołek.
	// Musimy zmapować dane do layoutów oczekiwanych przez shader.
	VAO cactusSphereVAO;
	cactusSphereVAO.Bind();
	// VBO dla danych sfery (współdzielone z sunVAO)
	VBO sphereVBO(sphereVertices.data(), sphereVertices.size() * sizeof(GLfloat)); // Zmieniona nazwa na sphereVBO
	// EBO dla danych sfery (współdzielone z sunVAO)
	EBO sphereEBO(sphereIndices.data(), sphereIndices.size() * sizeof(GLuint)); // Zmieniona nazwa na sphereEBO
	// Linkowanie atrybutów DLA DEFAULT.VERT (stride 8 floats)
	// Layout 0: Pozycja (3 floats, offset 0)
	cactusSphereVAO.LinkAttrib(sphereVBO, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0); // aPos
	// Layout 1: Kolor (3 floats, offset 3*sizeof(float) - wskazujemy na dane normalnych)
	// Shader default.vert nie używa aColor, ale ten link musi istnieć w VAO zgodnie z shaderem.
	cactusSphereVAO.LinkAttrib(sphereVBO, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float))); // aColor (dummy, points to normals)
	// Layout 2: Współrzędne tekstury (2 floats, offset 6*sizeof(float))
	cactusSphereVAO.LinkAttrib(sphereVBO, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float))); // aTex
	// Layout 3: Normalne (3 floats, offset 3*sizeof(float) - wskazujemy na dane normalnych)
	cactusSphereVAO.LinkAttrib(sphereVBO, 3, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float))); // aNormal
	// cactusSphereVAO.Unbind(); // Usunięto Unbind


	// === Konfiguracja VAO, VBO, EBO dla s�o�ca UŻYWANEJ PRZEZ SUN.VERT (SŁOŃCE) ===
	// Shader sun.vert oczekuje atrybutów w layoutach 0 (pos), 1 (normal), 2 (texCoord).
	// To odpowiada naturalnemu układowi danych sfery (pos, normal, tex).
	VAO sunVAO;
	sunVAO.Bind();
	// VBO dla danych sfery (współdzielone z cactusSphereVAO)
    // Używamy tego samego obiektu VBO, ale bindowany do innego VAO
	sphereVBO.Bind(); // Bindujemy VBO zawierające dane sfery
    // EBO dla danych sfery (współdzielone z cactusSphereVAO)
    // Używamy tego samego obiektu EBO
    sphereEBO.Bind(); // Bindujemy EBO zawierające indeksy sfery

	// Linkowanie atrybutów DLA SUN.VERT (stride 8 floats)
	// Layout 0: Pozycja (3 floats, offset 0)
	sunVAO.LinkAttrib(sphereVBO, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0); // aPos
	// Layout 1: Normalne (3 floats, offset 3*sizeof(float))
	sunVAO.LinkAttrib(sphereVBO, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float))); // aNormal
	// Layout 2: Współrzędne tekstury (2 floats, offset 6*sizeof(float))
	sunVAO.LinkAttrib(sphereVBO, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float))); // aTexCoords
	// sunVAO.Unbind(); // Usunięto Unbind


	//pozycje piramid (oryginalne)
	glm::vec3 pyramidPositions[] = {
		glm::vec3(0.9f, 0.0f,  -0.3f), //piramida centralna
		glm::vec3(-0.7f, 0.0f, 0.0f), //piramida po lewej
		glm::vec3(0.2f, 0.0f, -1.5f), //piramida po prawej
		glm::vec3(-1.5f, 0.0f, -1.0f)  //piramida daleko z ty�u
	};

	//skale dla piramid (oryginalne)
	float pyramidScales[] = {
		1.1f,    //skala piramidy centralnej
		1.0f,    //skala piramidy po lewej
		0.85f,    //skala piramidy po prawej
		0.7f     //skala piramidy daleko z ty�u
	};

	//k�ty obrotu wok� osi Y dla piramid (oryginalne)
	float pyramidYRotations[] = {
		-20.0f,          //piramida centralna
		25.0f,         //piramida po lewej
		5.0f,        //piramida po prawej
		45.0f          //piramida z ty�u
	};

	int numPyramids = sizeof(pyramidPositions) / sizeof(glm::vec3);


	// === Definicja struktury kaktusa (przykład prostego kaktusa z 1 częścią) ===
    // Używamy teraz skali dla SFERY
	const std::vector<CactusPart> standardCactusPartsData = {
		// Pojedynczy pień - mała sferka rozciągnięta w pionie
		// relativePosition umieszcza środek sfery PO skalowaniu
		// Aby dolna krawędź dotykała Y=0, środek musi być przesunięty o +0.5*scale.y w górę.
        // Skala (grubość X, wysokość Y, grubość Z)
		{ glm::vec3(0.0f, 0.5f * 0.5f, 0.0f), glm::vec3(0.15f, 0.5f, 0.15f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f } // skala 0.15x0.5x0.15
	};

	// === Pozycje poszczególnych instancji kaktusów (Dodano 2 nowe) ===
	// Wybrane pozycje w obszarze mniejszej podłogi
	glm::vec3 cactusPositionsXZ[] = {
		glm::vec3(1.5f, 0.0f, 0.5f),  // obok piramidy centralnej/prawej
		glm::vec3(-1.0f, 0.0f, 1.0f), // obok piramidy lewej
		glm::vec3(0.0f, 0.0f, -2.0f), // obok piramidy prawej/dalekiej
		// Dodane nowe kaktusy
		glm::vec3(-2.0f, 0.0f, -1.5f), // Nowy kaktus 1 (lewo, tył)
		glm::vec3(2.0f, 0.0f, 1.5f)   // Nowy kaktus 2 (prawo, przód)
	};
	int numCacti = sizeof(cactusPositionsXZ) / sizeof(glm::vec3);

	// === Tworzenie instancji kaktusów z poprawną wysokością Y i losowym obrotem ===
	std::vector<Cactus> cacti;
	for (int i = 0; i < numCacti; ++i) {
		// Pobierz pozycję X i Z z tablicy
		glm::vec3 posXZ = cactusPositionsXZ[i];
		// Oblicz wysokość podłogi na tej pozycji (X, Z) używając funkcji getHeight
		// Użyj tych samych parametrów amplitudy i częstotliwości, co do generowania podłogi!
		float groundHeight = getHeight(posXZ.x, posXZ.z, groundGenWaveAmplitude, groundGenWaveFrequency);
		// Wylosuj obrót wokół osi Y dla tej instancji
		float randomYRotation = dist(rng); // dist(rng) daje float z zakresu [0, 360)
		// Utwórz instancję kaktusa z poprawną pozycją (X, wysokość_podłogi, Z) i losowym obrotem Y
		cacti.push_back(Cactus(glm::vec3(posXZ.x, groundHeight, posXZ.z), randomYRotation));
	}


	// parametry dla ruchu wsch�d-zach�d (oryginalne)
	float dayNightCycleSpeed = 0.05f;
	float sunPathRadius = 5.0f;
	float sunMaxHeight = 3.5f;
	float sunMinHeight = -0.5f;
	float sunPathDepth = -3.0f;

    // === Zmniejszona wielkość słońca ===
	float sunRadius = 0.05f; // Zmieniono z 0.1f na 0.05f


	// Oblicz �rodek grupy piramid, �eby przesun�� tam pod�og�
	glm::vec3 pyramidCenter(0.0f);
	if (numPyramids > 0) {
		for (int i = 0; i < numPyramids; ++i) {
			pyramidCenter += pyramidPositions[i];
		}
		pyramidCenter /= numPyramids;
	}
	glm::vec3 groundOffset = glm::vec3(pyramidCenter.x, 0.0f, pyramidCenter.z);


	// === GŁÓWNA PĘTLA RENDEROWANIA ===
	while (!glfwWindowShouldClose(window))
	{
		float currentTime = (float)glfwGetTime();
		glClearColor(0.45f, 0.55f, 0.65f, 1.0f); // Kolor t�a
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Czyści bufory

		camera.Inputs(window);
		camera.updateMatrix(45.0f, 0.1f, 100.0f);

		glm::vec4 lightColor = glm::vec4(1.0f, 0.9f, 0.75f, 1.0f); //ciep�e ��te �wiat�o
		glm::vec4 sunTintColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); //kolor moduluj�cy teksturę s�o�ca

		//obliczanie pozycji s�o�ca dla cyklu wsch�d-zach�d
		float normalizedTime = fmod(currentTime * dayNightCycleSpeed, 2.0f);
		float pathParam;
		if (normalizedTime < 1.0f) {
			pathParam = normalizedTime;
		}
		else {
			pathParam = 2.0f - normalizedTime;
		}

		float lightX = -sunPathRadius + (2.0f * sunPathRadius * pathParam);
		float angleY = pathParam * M_PI;
		float lightY = sin(angleY) * (sunMaxHeight - sunMinHeight) + sunMinHeight;
		float lightZ = sunPathDepth;

		glm::vec3 lightPos = glm::vec3(lightX, lightY, lightZ);


		// --- renderowanie podloza ---
		pyramidShaderProgram.Activate(); // Uzywamy tego samego shadera dla podlogi, piramid i kaktusów
		camera.Matrix(pyramidShaderProgram, "camMatrix");
		pyramidShaderProgram.setVec4("lightColor", lightColor);
		pyramidShaderProgram.setVec3("lightPos", lightPos);
		pyramidShaderProgram.setVec3("camPos", camera.Position);
		pyramidShaderProgram.setInt("u_lightingMode", currentLightingMode);

		// Macierz modelu dla podłogi - Przesunięta o groundOffset
		glm::mat4 groundModel = glm::mat4(1.0f);
		groundModel = glm::translate(groundModel, groundOffset);
		pyramidShaderProgram.setMat4("model", groundModel);

		// Bindowanie tekstury podłogi (jednostka 2) i ustawienie uniformu "tex0" na jednostkę 2
		groundSandTexture.texUnit(pyramidShaderProgram, "tex0"); // Ustawia sampler 'tex0' w shaderze na unit 2
		groundSandTexture.Bind(); // Aktywuje jednostkę 2 i binduje teksturę

		// Bindowanie VAO podłogi
		groundVAO.Bind();

		// === Ustawienie NISKIEJ siły specularnej dla podłogi ===
		// Wymaga uniformu u_specularStrength w default.frag
		pyramidShaderProgram.setFloat("u_specularStrength", 0.05f); // Bardzo niska wartość dla piasku
		// ======================================================

		// Rysowanie podłogi (używamy rozmiaru wektora indeksów)
		glDrawElements(GL_TRIANGLES, groundIndicesVec.size(), GL_UNSIGNED_INT, 0);

		// groundVAO.Unbind(); // Usunięto Unbind


		// --- renderowanie kaktusów ---
		// Aktywujemy już używany shader, ale zmieniamy teksturę i VAO
		// pyramidShaderProgram.Activate(); // Już aktywowany

		// Po narysowaniu podłogi, przełącz uniform "tex0" na jednostkę 3 dla kaktusów
		cactusTexture.texUnit(pyramidShaderProgram, "tex0"); // Ustawia sampler 'tex0' w shaderze na unit 3
		cactusTexture.Bind(); // Bindowanie tekstury kaktusa (jednostka 3)

		// === Bindowanie VAO sfery dla kaktusów ===
		cactusSphereVAO.Bind();

		// === Ustawienie ŚREDNIEJ siły specularnej dla kaktusów ===
		pyramidShaderProgram.setFloat("u_specularStrength", 0.2f); // Niska, ale nieco wyższa niż dla piasku (np. liście)
		// ======================================================

		// Rysowanie każdej instancji kaktusa
		// std::vector<Cactus> cacti zadeklarowany i wypełniony przed p�tl�
		// standardCactusPartsData zadeklarowany przed p�tlą
		for (const auto& cactus : cacti) // Iteracja przez wektor instancji kaktusów
		{
			// Metoda Draw klasy Cactus zajmie się ustawieniem macierzy modelu
			// dla każdej części i wywołaniem glDrawElements dla SFERY.
			// Przekazujemy shader, liczbę indeksów sfery i dane o częściach.
			cactus.Draw(pyramidShaderProgram, sphereIndexCount, standardCactusPartsData);
		}

		// cactusSphereVAO.Unbind(); // Usunięto Unbind


		// --- renderowanie piramidy ---
		// Po narysowaniu kaktusów, przywracamy uniform "tex0" na jednostkę 0 dla piramid
		pyramidTexture.texUnit(pyramidShaderProgram, "tex0"); // Ustawia sampler 'tex0' w shaderze z powrotem na unit 0
		pyramidTexture.Bind(); // Bindowanie tekstury piramidy (jednostka 0)

		// Bindowanie VAO piramidy
		pyramidVAO.Bind();

		// === Ustawienie WYŻSZEJ siły specularnej dla piramid ===
		pyramidShaderProgram.setFloat("u_specularStrength", 0.7f); // Wartość dla piramid (piaskowiec może się lekko błyszczeć)
		// =====================================================

		for (int i = 0; i < numPyramids; ++i)
		{
			glm::mat4 pyramidModel = glm::mat4(1.0f);
			//przeduniecie do pozycji
			pyramidModel = glm::translate(pyramidModel, pyramidPositions[i]);
			//obrocenie wokol osi y
			pyramidModel = glm::rotate(pyramidModel, glm::radians(pyramidYRotations[i]), glm::vec3(0.0f, 1.0f, 0.0f));
			//skalowanie
			pyramidModel = glm::scale(pyramidModel, glm::vec3(pyramidScales[i]));

			pyramidShaderProgram.setMat4("model", pyramidModel);
			glDrawElements(GL_TRIANGLES, sizeof(pyramidIndices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
		}

		// --- renderowanie s�o�ca (jako �wiec�cej sfery) ---
		// Po narysowaniu piramid, przełączamy shader i VAO na słońce
		// pyramidVAO.Unbind(); // Usunięto Unbind
		// pyramidShaderProgram.Deactivate(); // Usunięto Deactivate

		sunShaderProgram.Activate(); // Teraz aktywujemy shader słońca

		camera.Matrix(sunShaderProgram, "camMatrix");
		glm::mat4 sunModel = glm::mat4(1.0f);
		sunModel = glm::translate(sunModel, lightPos); //przesuni�cie slonca na pozycje swiatla
        sunModel = glm::scale(sunModel, glm::vec3(sunRadius/baseSphereRadius)); // Skalujemy bazową sferę do rozmiaru słońca
		sunShaderProgram.setMat4("model", sunModel);
		sunShaderProgram.setVec4("sunColor", sunTintColor);

		sunTexture.Bind();
		// === Bindowanie VAO sfery dla słońca ===
		sunVAO.Bind();
		glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);

		// sunVAO.Unbind(); // Usunięto Unbind
		// sunShaderProgram.Deactivate(); // Usunięto Deactivate - shaddery pozostają aktywne do końca ramki

		// Na końcu pętli renderowania, opcjonalnie możemy unbindować ostatnio używane VAO i shader,
		// choć nie jest to konieczne, bo będą zbindowane na nowo w następnej ramce.
		// glUseProgram(0);
		// glBindVertexArray(0);


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// === CZYSZCZENIE ZASOBÓW ===
	pyramidVAO.Delete(); pyramidVBO.Delete(); pyramidEBO.Delete();
	groundVAO.Delete(); groundVBO.Delete(); groundEBO.Delete();

	// === Czyszczenie zasobów sfery (VBO i EBO są współdzielone!) ===
	cactusSphereVAO.Delete(); // Usuwamy VAO dla kaktusów
	sunVAO.Delete();          // Usuwamy VAO dla słońca
    sphereVBO.Delete(); // Usuwamy VBO danych sfery (było jedno VBO używane przez dwa VAO)
    sphereEBO.Delete(); // Usuwamy EBO indeksów sfery (było jedno EBO używane przez dwa VAO)


	pyramidTexture.Delete();
	sunTexture.Delete();
	groundSandTexture.Delete();
	cactusTexture.Delete(); // Usunięcie tekstury kaktusa

	pyramidShaderProgram.Delete();
	sunShaderProgram.Delete();


	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
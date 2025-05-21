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

#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Camera.h"
#include "Texture.h"

//funkcja pomocnicza do generowania wierzcho�k�w sfery UV - zwraca wektor danych wierzcho�k�w (pozycja, normalna, texCoord) i wektor indeks�w
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
		//the first and last outSphereVertices have same position and normal, but different texCoords
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

//dane wierzcho�k�w (wspolrzedne, kolory, tekstury, normalne)
GLfloat pyramidVertices[] =
{ 
	-0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,  0.0f, 0.0f,   0.0f, -1.0f, 0.0f, //dol
	-0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,  0.0f, 5.0f,   0.0f, -1.0f, 0.0f,
	 0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,  5.0f, 5.0f,   0.0f, -1.0f, 0.0f,
	 0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,  5.0f, 0.0f,   0.0f, -1.0f, 0.0f,

	-0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,  0.0f, 0.0f,  -0.8f, 0.5f, 0.0f, //lewa
	-0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,  5.0f, 0.0f,  -0.8f, 0.5f, 0.0f,
	 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,  2.5f, 5.0f,  -0.8f, 0.5f, 0.0f,

	-0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,  5.0f, 0.0f,   0.0f, 0.5f,-0.8f, //tyl
	 0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,  0.0f, 0.0f,   0.0f, 0.5f,-0.8f,
	 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,  2.5f, 5.0f,   0.0f, 0.5f,-0.8f,

	 0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,  0.0f, 0.0f,   0.8f, 0.5f, 0.0f, //prawa
	 0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,  5.0f, 0.0f,   0.8f, 0.5f, 0.0f,
	 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,  2.5f, 5.0f,   0.8f, 0.5f, 0.0f,

	 0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,  5.0f, 0.0f,   0.0f, 0.5f, 0.8f, //przod
	-0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,  0.0f, 0.0f,   0.0f, 0.5f, 0.8f,
	 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,  2.5f, 5.0f,   0.0f, 0.5f, 0.8f
};

GLuint pyramidIndices[] =
{
	0, 1, 2, 0, 2, 3, //dol
	4, 6, 5,          //lewa
	7, 9, 8,          //tyl
	10, 12, 11,       //prawa
	13, 15, 14        //przod
};

//wierzcho�ki i indeksy dla p�aszczyzny pod�o�a
GLfloat groundVertices[] = {
	//wspolrzedne, kolory (tu nie uzywane bo tekstura), tekstura, normalne (zawsze w gore)
	 50.0f, 0.0f,  50.0f,  1.0f, 1.0f, 1.0f,  50.0f,  0.0f,  0.0f, 1.0f, 0.0f,
	 50.0f, 0.0f, -50.0f,  1.0f, 1.0f, 1.0f,  50.0f, 50.0f,  0.0f, 1.0f, 0.0f,
	-50.0f, 0.0f, -50.0f,  1.0f, 1.0f, 1.0f,   0.0f, 50.0f,  0.0f, 1.0f, 0.0f,
	-50.0f, 0.0f,  50.0f,  1.0f, 1.0f, 1.0f,   0.0f,  0.0f,  0.0f, 1.0f, 0.0f
};
GLuint groundIndices[] = {
	0, 1, 3, //pierwszy trojkat
	1, 2, 3  //drugi trojkat
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

	Camera camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 1.0f, 8.0f)); //lekko podniesiona kamera

	Shader pyramidShaderProgram("default.vert", "default.frag");
	if (pyramidShaderProgram.ID == 0) { glfwTerminate(); return -1; } //sprawdzenie, czy shader sie zaladowal

	Shader sunShaderProgram("sun.vert", "sun.frag"); //shader dla slonca
	if (sunShaderProgram.ID == 0) { pyramidShaderProgram.Delete(); glfwTerminate(); return -1; }

	//zaladowanie tekstury
	//jednostka teksturuj�ca 0, format GL_RGBA, typ GL_UNSIGNED_BYTE
	Texture pyramidTexture("sand_texture.png", GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE);
	if (pyramidTexture.ID == 0) { //sprawdzenie, czy tekstura sie zaladowala
		pyramidShaderProgram.Delete();
		sunShaderProgram.Delete();
		glfwTerminate();
		return -1;
	}
	//ustawienie uniformu samplera tex0 na jednostk� teksturuj�c� 0
	pyramidTexture.texUnit(pyramidShaderProgram, "tex0");

	Texture sunTexture("sun_texture.png", GL_TEXTURE_2D, 1, GL_RGBA, GL_UNSIGNED_BYTE);		//jednostka 1 dla slonca
	if (sunTexture.ID == 0) {
		pyramidShaderProgram.Delete();
		sunShaderProgram.Delete();
		pyramidTexture.Delete();
		glfwTerminate();
		return -1;
	}
	sunTexture.texUnit(sunShaderProgram, "sunTexture"); //sunTexture dla shadera s�o�ca

	//tekstura dla podloza z piasku
	Texture groundSandTexture("groundSand_texture.png", GL_TEXTURE_2D, 2, GL_RGBA, GL_UNSIGNED_BYTE);
	if (groundSandTexture.ID == 0) {
		pyramidShaderProgram.Delete(); sunShaderProgram.Delete(); pyramidTexture.Delete(); sunTexture.Delete();
		glfwTerminate();
		return -1;
	}

	//vao, vbo i ebo dla piramidy
	VAO pyramidVAO;
	pyramidVAO.Bind();
	VBO pyramidVBO(pyramidVertices, sizeof(pyramidVertices));
	EBO pyramidEBO(pyramidIndices, sizeof(pyramidIndices));
	//atrybuty: 0-pozycja, 1-kolor, 2-wsp. tekstury, 3-normalne
	//stride dla piramidy: 3(pos)+3(color)+2(tex)+3(normal)=11 floats
	pyramidVAO.LinkAttrib(pyramidVBO, 0, 3, GL_FLOAT, 11 * sizeof(float), (void*)0);
	pyramidVAO.LinkAttrib(pyramidVBO, 1, 3, GL_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	pyramidVAO.LinkAttrib(pyramidVBO, 2, 2, GL_FLOAT, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	pyramidVAO.LinkAttrib(pyramidVBO, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	pyramidVAO.Unbind(); // VBO i EBO s� odpinane przez VAO/EBO

	//vao, vbo, ebo dla podloza
	VAO groundVAO;
	VBO groundVBO(groundVertices, sizeof(groundVertices));
	EBO groundEBO(groundIndices, sizeof(groundIndices));
	groundVAO.Bind();
	//stride dla podloza taki sam jak dla piramidy - 11 floats
	groundVAO.LinkAttrib(groundVBO, 0, 3, GL_FLOAT, 11 * sizeof(float), (void*)0);
	groundVAO.LinkAttrib(groundVBO, 1, 3, GL_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	groundVAO.LinkAttrib(groundVBO, 2, 2, GL_FLOAT, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	groundVAO.LinkAttrib(groundVBO, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	groundVAO.Unbind();

	//generowanie danych dla sfery (s�o�ca)
	std::vector<GLfloat> sphereVertices;
	std::vector<GLuint> sphereIndices;
	float sunRadius = 0.1f; //rozmiar s�o�ca
	generateSphere(sunRadius, 36, 18, sphereVertices, sphereIndices); //36 sektor�w, 18 stos�w

	VAO sunVAO;
	sunVAO.Bind();
	VBO sunVBO(&sphereVertices[0], sphereVertices.size() * sizeof(GLfloat));
	EBO sunEBO(&sphereIndices[0], sphereIndices.size() * sizeof(GLuint));
	//stride dla sfery: 3 (pos) + 3 (normal) + 2 (texCoord) = 8 floats
	//layout: 0=pos, 1=normal, 2=texCoord (zgodnie z sun.vert)
	sunVAO.LinkAttrib(sunVBO, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
	sunVAO.LinkAttrib(sunVBO, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float))); //normalne
	sunVAO.LinkAttrib(sunVBO, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float))); //texCoords
	sunVAO.Unbind();

	//pozycje piramid
	glm::vec3 pyramidPositions[] = {
		glm::vec3(0.9f, 0.0f,  -0.3f), //piramida centralna
		glm::vec3(-0.7f, 0.0f, 0.0f), //piramida po lewej
		glm::vec3(0.2f, 0.0f, -1.5f), //piramida po prawej
		glm::vec3(-1.5f, 0.0f, -1.0f)  //piramida daleko z ty�u
	};

	//skale dla piramid - piramidy bedace bardziej z tylu sa mniejsze
	float pyramidScales[] = {
		1.1f,    //skala piramidy centralnej
		1.0f,    //skala piramidy po lewej
		0.85f,    //skala piramidy po prawej
		0.7f     //skala piramidy daleko z ty�u
	};

	//k�ty obrotu wok� osi Y dla piramid
	float pyramidYRotations[] = {
		-20.0f,          //piramida centralna bez dodatkowego obrotu
		25.0f,         //piramida po lewej obr�cona o 45 stopni
		5.0f,        //piramida po prawej obr�cona o -30 stopni
		45.0f          //piramida z ty�u
	};

	int numPyramids = sizeof(pyramidPositions) / sizeof(glm::vec3);

	//parametry dla ruchu wsch�d-zach�d
	float dayNightCycleSpeed = 0.05f; //jak szybko ma przebiega� cykl dnia i nocy
	float sunPathRadius = 5.0f;    //jak daleko od centrum sceny (na osi X lub Z) s�o�ce "wschodzi/zachodzi"
	float sunMaxHeight = 3.5f;     //maksymalna wysoko�� s�o�ca w zenicie
	float sunMinHeight = -0.5f;    //minimalna wysoko�� (mo�e by� lekko poni�ej horyzontu dla pe�nego zachodu)
	//s�o�ce porusza si� w p�aszczy�nie X-Y, Z pozostanie sta�e (lub odwrotnie)
	float sunPathDepth = -3.0f;    //sta�a g��boko�� Z dla �cie�ki s�o�ca (lub X, je�li ruch jest w Z-Y)


	while (!glfwWindowShouldClose(window))
	{
		float currentTime = (float)glfwGetTime();
		glClearColor(0.45f, 0.55f, 0.65f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera.Inputs(window);
		camera.updateMatrix(45.0f, 0.1f, 100.0f);

		glm::vec4 lightColor = glm::vec4(1.0f, 0.9f, 0.75f, 1.0f); //ciep�e ��te �wiat�o
		glm::vec4 sunTintColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); //kolor moduluj�cy tekstur� s�o�ca

		//obliczanie pozycji s�o�ca dla cyklu wsch�d-zach�d
	   //k�t zmienia si� od 0 do PI (wsch�d do zachodu) i potem resetuje lub idzie w drug� stron�
	   //dla uproszczenia, p�tla 0 -> PI -> 0 (dzie�)
		float angle = fmod(currentTime * dayNightCycleSpeed, (float)M_PI * 2.0f); //k�t od 0 do 2*PI

		float lightX, lightY, lightZ;

		//ruch po �uku od -sunPathRadius do +sunPathRadius na osi X
		//wysoko�� (Y) zmienia si� sinusoidalnie
		lightX = cos(angle) * sunPathRadius; //od sunPathRadius do -sunPathRadius i z powrotem
		lightY = sin(angle) * (sunMaxHeight - sunMinHeight) * 0.5f + (sunMaxHeight + sunMinHeight) * 0.5f;
		//Y >= sunMinHeight. sin(angle) da warto�ci od 0 (wsch�d/zach�d) do 1 (zenit) dla angle w [0, PI]
		//je�li angle jest w [0, 2PI], to sin(angle) b�dzie od -1 do 1
		//dla prostego �uku g�ra-d�:
		float normalizedTime = fmod(currentTime * dayNightCycleSpeed, 2.0f); //0 do 2.0 (0-1 wsch�d->zach�d, 1-2 zach�d->wsch�d)
		float pathParam;
		if (normalizedTime < 1.0f) { //wsch�d -> zenit -> zach�d
			pathParam = normalizedTime; //0 -> 1
		}
		else { //zach�d -> nadir -> wsch�d (pod horyzontem)
			pathParam = 2.0f - normalizedTime; //1 -> 0 (symetrycznie)
		}

		//pozycja x od -sunPathRadius do +sunPathRadius
		lightX = -sunPathRadius + (2.0f * sunPathRadius * pathParam);

		//pozycja Y jako parabola lub sinusoida od 0 do PI
		//k�t dla Y od 0 (wsch�d) do PI (zach�d)
		float angleY = pathParam * M_PI; //0 -> PI
		lightY = sin(angleY) * (sunMaxHeight - sunMinHeight) + sunMinHeight;

		lightZ = sunPathDepth; //sta�a g��boko�� Z

		glm::vec3 lightPos = glm::vec3(lightX, lightY, lightZ);

		//renderowanie piramidy
		pyramidShaderProgram.Activate();
		camera.Matrix(pyramidShaderProgram, "camMatrix"); //ustawienie macierzy kamery
		pyramidShaderProgram.setVec4("lightColor", lightColor);
		pyramidShaderProgram.setVec3("lightPos", lightPos);
		pyramidShaderProgram.setVec3("camPos", camera.Position);
		pyramidShaderProgram.setInt("u_lightingMode", currentLightingMode);

		glm::mat4 groundModel = glm::mat4(1.0f); //pod�o�ena y=0
		pyramidShaderProgram.setMat4("model", groundModel);

		groundSandTexture.texUnit(pyramidShaderProgram, "tex0"); //shader uzywa tekstury piasku dla 'tex0'
		groundSandTexture.Bind();

		groundVAO.Bind();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); //2 tr�jk�ty = 6 indeks�w

		pyramidTexture.texUnit(pyramidShaderProgram, "tex0"); //przywr�cenie tekstury piramidy dla tex0
		pyramidTexture.Bind();		//bindowanie tekstury - u�ycie zapisanej jednostki
		pyramidVAO.Bind();

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

		//renderowanie s�o�ca (jako �wiec�cej sfery)
		sunShaderProgram.Activate();
		camera.Matrix(sunShaderProgram, "camMatrix");
		glm::mat4 sunModel = glm::mat4(1.0f);
		sunModel = glm::translate(sunModel, lightPos); //przesuni�cie slonca na pozycje swiatla
		//nie potrzebne jest skalowanie, bo promie� jest ju� w generateSphere
		sunShaderProgram.setMat4("model", sunModel);
		sunShaderProgram.setVec4("sunColor", sunTintColor); //dla modulacji koloru tekstury s�o�ca
		//sunShaderProgram.setInt("sunTexture", 1); //ju� ustawione przez sunTexture.texUnit

		sunTexture.Bind();
		sunVAO.Bind();
		glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	pyramidVAO.Delete(); pyramidVBO.Delete(); pyramidEBO.Delete();
	groundVAO.Delete(); groundVBO.Delete(); groundEBO.Delete();
	sunVAO.Delete(); sunVBO.Delete(); sunEBO.Delete();
	pyramidTexture.Delete(); sunTexture.Delete();
	pyramidShaderProgram.Delete(); sunShaderProgram.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
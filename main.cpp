#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <string> 
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <random>
#include <ctime>

#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Camera.h"
#include "Texture.h"
#include "Cactus.h"
#include "Skybox.h" 


void generateSphere(float radius, int sectorCount, int stackCount,
    std::vector<GLfloat>& outSphereVertices, std::vector<GLuint>& outSphereIndices)
{
    outSphereVertices.clear();
    outSphereIndices.clear();

    float x, y, z, xy;                      //vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    //vertex normal
    float s, t;                                     //vertex texCoord

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;        //starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             //r * cos(u)
        z = radius * sinf(stackAngle);              //r * sin(u)

        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           //starting from 0 to 2pi

            x = xy * cosf(sectorAngle);             //r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             //r * cos(u) * sin(v)
            outSphereVertices.push_back(x);
            outSphereVertices.push_back(y);
            outSphereVertices.push_back(z);

            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            outSphereVertices.push_back(nx);
            outSphereVertices.push_back(ny);
            outSphereVertices.push_back(nz);

            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            outSphereVertices.push_back(s);
            outSphereVertices.push_back(t);
        }
    }

    int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     
        k2 = k1 + sectorCount + 1;      

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                outSphereIndices.push_back(k1);
                outSphereIndices.push_back(k2);
                outSphereIndices.push_back(k1 + 1);
            }
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

glm::vec3 calculateNormal(float x, float z, float epsilon, float amplitude, float frequency) {
    float y_center = getHeight(x, z, amplitude, frequency);
    float y_dx = getHeight(x + epsilon, z, amplitude, frequency);
    float y_dz = getHeight(x, z + epsilon, amplitude, frequency);
    glm::vec3 tangentX = glm::vec3(epsilon, y_dx - y_center, 0.0f);
    glm::vec3 tangentZ = glm::vec3(0.0f, y_dz - y_center, epsilon);
    glm::vec3 normal = glm::normalize(glm::cross(tangentZ, tangentX));
    return normal;
}

void generateWavyGround(int segmentsX, int segmentsZ, float totalWidth, float totalDepth,
    float waveAmplitude, float waveFrequency, float textureTiling,
    std::vector<GLfloat>& outGroundVertices, std::vector<GLuint>& outGroundIndices)
{
    outGroundVertices.clear();
    outGroundIndices.clear();
    float segmentWidth = totalWidth / segmentsX;
    float segmentDepth = totalDepth / segmentsZ;
    float epsilon = 0.005f;
    for (int i = 0; i <= segmentsZ; ++i) {
        for (int j = 0; j <= segmentsX; ++j) {
            float x = (float)j * segmentWidth - totalWidth * 0.5f;
            float z = (float)i * segmentDepth - totalDepth * 0.5f;
            float y = getHeight(x, z, waveAmplitude, waveFrequency);
            float r = 1.0f, g = 1.0f, b = 1.0f; // Dummy color
            float s = (float)j / segmentsX * textureTiling;
            float t = (float)i / segmentsZ * textureTiling;
            glm::vec3 normal = calculateNormal(x, z, epsilon, waveAmplitude, waveFrequency);
            outGroundVertices.push_back(x); outGroundVertices.push_back(y); outGroundVertices.push_back(z);
            outGroundVertices.push_back(r); outGroundVertices.push_back(g); outGroundVertices.push_back(b);
            outGroundVertices.push_back(s); outGroundVertices.push_back(t);
            outGroundVertices.push_back(normal.x); outGroundVertices.push_back(normal.y); outGroundVertices.push_back(normal.z);
        }
    }
    int verticesPerSegmentRow = segmentsX + 1;
    for (int i = 0; i < segmentsZ; ++i) {
        for (int j = 0; j < segmentsX; ++j) {
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
// --- Koniec funkcji pomocniczych ---

static int currentLightingMode = 3;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_1) { currentLightingMode = 0; std::cout << "Tryb: Ambient" << std::endl; }
        else if (key == GLFW_KEY_2) { currentLightingMode = 1; std::cout << "Tryb: Diffuse" << std::endl; }
        else if (key == GLFW_KEY_3) { currentLightingMode = 2; std::cout << "Tryb: Specular (+Ambient)" << std::endl; }
        else if (key == GLFW_KEY_4) { currentLightingMode = 3; std::cout << "Tryb: Pełne (ADS)" << std::endl; }
        else if (key == GLFW_KEY_L) {
            currentLightingMode = (currentLightingMode + 1) % 4;
            if (currentLightingMode == 0) std::cout << "Tryb: Ambient" << std::endl;
            else if (currentLightingMode == 1) std::cout << "Tryb: Diffuse" << std::endl;
            else if (currentLightingMode == 2) std::cout << "Tryb: Specular (+Ambient)" << std::endl;
            else if (currentLightingMode == 3) std::cout << "Tryb: Pełne (ADS)" << std::endl;
        }
    }
}

GLfloat pyramidVertices[] = { 
    -0.5f, 0.0f,  0.5f,    0.83f, 0.70f, 0.44f,  0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
    -0.5f, 0.0f, -0.5f,    0.83f, 0.70f, 0.44f,  0.0f, 5.0f,   0.0f, -1.0f, 0.0f,
     0.5f, 0.0f, -0.5f,    0.83f, 0.70f, 0.44f,  5.0f, 5.0f,   0.0f, -1.0f, 0.0f,
     0.5f, 0.0f,  0.5f,    0.83f, 0.70f, 0.44f,  5.0f, 0.0f,   0.0f, -1.0f, 0.0f,
    -0.5f, 0.0f,  0.5f,    0.83f, 0.70f, 0.44f,  0.0f, 0.0f,  -0.8f, 0.5f, 0.0f,
    -0.5f, 0.0f, -0.5f,    0.83f, 0.70f, 0.44f,  5.0f, 0.0f,  -0.8f, 0.5f, 0.0f,
     0.0f, 0.8f,  0.0f,    0.92f, 0.86f, 0.76f,  2.5f, 5.0f,  -0.8f, 0.5f, 0.0f,
    -0.5f, 0.0f, -0.5f,    0.83f, 0.70f, 0.44f,  5.0f, 0.0f,   0.0f, 0.5f,-0.8f,
     0.5f, 0.0f, -0.5f,    0.83f, 0.70f, 0.44f,  0.0f, 0.0f,   0.0f, 0.5f,-0.8f,
     0.0f, 0.8f,  0.0f,    0.92f, 0.86f, 0.76f,  2.5f, 5.0f,   0.0f, 0.5f,-0.8f,
     0.5f, 0.0f, -0.5f,    0.83f, 0.70f, 0.44f,  0.0f, 0.0f,   0.8f, 0.5f, 0.0f,
     0.5f, 0.0f,  0.5f,    0.83f, 0.70f, 0.44f,  5.0f, 0.0f,   0.8f, 0.5f, 0.0f,
     0.0f, 0.8f,  0.0f,    0.92f, 0.86f, 0.76f,  2.5f, 5.0f,   0.8f, 0.5f, 0.0f,
     0.5f, 0.0f,  0.5f,    0.83f, 0.70f, 0.44f,  5.0f, 0.0f,   0.0f, 0.5f, 0.8f,
    -0.5f, 0.0f,  0.5f,    0.83f, 0.70f, 0.44f,  0.0f, 0.0f,   0.0f, 0.5f, 0.8f,
     0.0f, 0.8f,  0.0f,    0.92f, 0.86f, 0.76f,  2.5f, 5.0f,   0.0f, 0.5f, 0.8f
};
GLuint pyramidIndices[] = { 
    0, 1, 2, 0, 2, 3,
    4, 6, 5,
    7, 9, 8,
    10, 12, 11,
    13, 15, 14
};

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// ------ Konfiguracja tekstur Skyboxa ------
std::vector<std::string> skyboxFaces = {
    "textures/skybox/Right.png", "textures/skybox/Left.png",
    "textures/skybox/Top.png",   "textures/skybox/Bottom.png",
    "textures/skybox/Front.png", "textures/skybox/Back.png"
};
// -----------------------------------------

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Projekt OpenGL + Skybox", NULL, NULL);
    if (window == NULL) { std::cout << "Nie udało się utworzyć okna GLFW" << std::endl; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cout << "Nie udało się zainicjalizować GLAD" << std::endl; return -1; }

    glEnable(GL_DEPTH_TEST); // Włączone globalnie
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glfwSetKeyCallback(window, key_callback);

    std::srand(static_cast<unsigned int>(std::time(0)));
    std::mt19937 rng(static_cast<unsigned int>(std::time(0)));
    std::uniform_real_distribution<float> dist(0.0f, 360.0f);

    Camera camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 2.0f, 10.0f));
    Shader pyramidShaderProgram("default.vert", "default.frag"); 
    Shader sunShaderProgram("sun.vert", "sun.frag");        

    
    if (pyramidShaderProgram.ID == 0) { std::cerr << "Shader 'default' nie załadowany." << std::endl; return -1; }
    if (sunShaderProgram.ID == 0) { std::cerr << "Shader 'sun' nie załadowany." << std::endl; return -1; }

    
    Skybox skybox("skybox.vert", "skybox.frag"); 
    if (!skybox.loadCubemap(skyboxFaces)) {
        std::cerr << "Nie udało się załadować tekstur skyboxa." << std::endl;
        
        return -1;
    }

    Texture pyramidTexture("sand_texture.png", GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE);
    Texture sunTexture("sun_texture.png", GL_TEXTURE_2D, 1, GL_RGBA, GL_UNSIGNED_BYTE);
    Texture groundSandTexture("groundSand_texture.png", GL_TEXTURE_2D, 2, GL_RGBA, GL_UNSIGNED_BYTE);
    Texture cactusTexture("cactus_texture.jpg", GL_TEXTURE_2D, 3, GL_RGBA, GL_UNSIGNED_BYTE);
    


    std::vector<GLfloat> groundVerticesVec;
    std::vector<GLuint> groundIndicesVec;
    int segmentsX = 60; int segmentsZ = 60; float totalGroundWidth = 6.0f; float totalGroundDepth = 6.0f;
    float waveAmplitude = 0.25f; float waveFrequency = 0.8f; float textureTiling = 8.0f;
    generateWavyGround(segmentsX, segmentsZ, totalGroundWidth, totalGroundDepth, waveAmplitude, waveFrequency, textureTiling, groundVerticesVec, groundIndicesVec);
    float groundGenWaveAmplitude = waveAmplitude; float groundGenWaveFrequency = waveFrequency;

    VAO groundVAO; groundVAO.Bind();
    VBO groundVBO(groundVerticesVec.data(), groundVerticesVec.size() * sizeof(GLfloat));
    EBO groundEBO(groundIndicesVec.data(), groundIndicesVec.size() * sizeof(GLuint));
    groundVAO.LinkAttrib(groundVBO, 0, 3, GL_FLOAT, 11 * sizeof(float), (void*)0); // aPos
    groundVAO.LinkAttrib(groundVBO, 1, 3, GL_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float))); // aColor
    groundVAO.LinkAttrib(groundVBO, 2, 2, GL_FLOAT, 11 * sizeof(float), (void*)(6 * sizeof(float))); // aTex
    groundVAO.LinkAttrib(groundVBO, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float))); // aNormal

    VAO pyramidVAO; pyramidVAO.Bind();
    VBO pyramidVBO(pyramidVertices, sizeof(pyramidVertices));
    EBO pyramidEBO(pyramidIndices, sizeof(pyramidIndices));
    pyramidVAO.LinkAttrib(pyramidVBO, 0, 3, GL_FLOAT, 11 * sizeof(float), (void*)0);
    pyramidVAO.LinkAttrib(pyramidVBO, 1, 3, GL_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    pyramidVAO.LinkAttrib(pyramidVBO, 2, 2, GL_FLOAT, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    pyramidVAO.LinkAttrib(pyramidVBO, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float)));

    std::vector<GLfloat> sphereVertices; std::vector<GLuint> sphereIndices;
    float baseSphereRadius = 0.5f;
    generateSphere(baseSphereRadius, 36, 18, sphereVertices, sphereIndices);
    GLsizei sphereIndexCount = sphereIndices.size();

    VAO cactusSphereVAO; cactusSphereVAO.Bind();
    VBO sphereVBO(sphereVertices.data(), sphereVertices.size() * sizeof(GLfloat));
    EBO sphereEBO(sphereIndices.data(), sphereIndices.size() * sizeof(GLuint));
    cactusSphereVAO.LinkAttrib(sphereVBO, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
    cactusSphereVAO.LinkAttrib(sphereVBO, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float))); // dummy color
    cactusSphereVAO.LinkAttrib(sphereVBO, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    cactusSphereVAO.LinkAttrib(sphereVBO, 3, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float))); // normal

    VAO sunVAO; sunVAO.Bind();
    sphereVBO.Bind(); sphereEBO.Bind(); // Re-bind shared VBO/EBO
    sunVAO.LinkAttrib(sphereVBO, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
    sunVAO.LinkAttrib(sphereVBO, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    sunVAO.LinkAttrib(sphereVBO, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    // Pozycje, skale, rotacje piramid 
    glm::vec3 pyramidPositions[] = { /* ... */ glm::vec3(0.9f, 0.0f, -0.3f), glm::vec3(-0.7f, 0.0f, 0.0f), glm::vec3(0.2f, 0.0f, -1.5f), glm::vec3(-1.5f, 0.0f, -1.0f) };
    float pyramidScales[] = { /* ... */ 1.1f, 1.0f, 0.85f, 0.7f };
    float pyramidYRotations[] = { /* ... */ -20.0f, 25.0f, 5.0f, 45.0f };
    int numPyramids = sizeof(pyramidPositions) / sizeof(glm::vec3);

    const std::vector<CactusPart> standardCactusPartsData = {
        { glm::vec3(0.0f, 0.5f * 0.5f, 0.0f), glm::vec3(0.15f, 0.5f, 0.15f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f }
    };
    glm::vec3 cactusPositionsXZ[] = { /* ... */ glm::vec3(1.5f, 0.0f, 0.5f), glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(-2.0f, 0.0f, -1.5f), glm::vec3(2.0f, 0.0f, 1.5f) };
    int numCacti = sizeof(cactusPositionsXZ) / sizeof(glm::vec3);
    std::vector<Cactus> cacti;
    for (int i = 0; i < numCacti; ++i) {
        glm::vec3 posXZ = cactusPositionsXZ[i];
        float groundHeight = getHeight(posXZ.x, posXZ.z, groundGenWaveAmplitude, groundGenWaveFrequency);
        float randomYRotation = dist(rng);
        cacti.push_back(Cactus(glm::vec3(posXZ.x, groundHeight, posXZ.z), randomYRotation));
    }

    float dayNightCycleSpeed = 0.05f; float sunPathRadius = 5.0f; float sunMaxHeight = 3.5f;
    float sunMinHeight = -0.5f; float sunPathDepth = -3.0f; float sunRadius = 0.05f;
    glm::vec3 pyramidCenter(0.0f); if (numPyramids > 0) { for (int i = 0; i < numPyramids; ++i) { pyramidCenter += pyramidPositions[i]; } pyramidCenter /= numPyramids; }
    glm::vec3 groundOffset = glm::vec3(pyramidCenter.x, 0.0f, pyramidCenter.z);

    // Pętla renderowania
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        glClearColor(0.45f, 0.55f, 0.65f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.Inputs(window);
       
        float FOV = 45.0f;
        float nearPlane = 0.1f;
        float farPlane = 100.0f; 
        glm::mat4 currentViewMatrix = glm::lookAt(camera.Position, camera.Position + camera.Orientation, camera.Up);
        glm::mat4 currentProjectionMatrix = glm::perspective(glm::radians(FOV), (float)SCR_WIDTH / (float)SCR_HEIGHT, nearPlane, farPlane);
        glm::mat4 combinedCamMatrix = currentProjectionMatrix * currentViewMatrix; 

        glm::vec4 lightColor = glm::vec4(1.0f, 0.9f, 0.75f, 1.0f);
        glm::vec4 sunTintColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        float normalizedTime = fmod(currentTime * dayNightCycleSpeed, 2.0f);
        float pathParam = (normalizedTime < 1.0f) ? normalizedTime : (2.0f - normalizedTime);
        float lightX = -sunPathRadius + (2.0f * sunPathRadius * pathParam);
        float angleY = pathParam * M_PI;
        float lightY = sin(angleY) * (sunMaxHeight - sunMinHeight) + sunMinHeight;
        float lightZ = sunPathDepth;
        glm::vec3 lightPos = glm::vec3(lightX, lightY, lightZ);

        pyramidShaderProgram.Activate();
        // camera.Matrix(pyramidShaderProgram, "camMatrix"); 
        pyramidShaderProgram.setMat4("camMatrix", combinedCamMatrix);
        pyramidShaderProgram.setVec4("lightColor", lightColor);
        pyramidShaderProgram.setVec3("lightPos", lightPos);
        pyramidShaderProgram.setVec3("camPos", camera.Position);
        pyramidShaderProgram.setInt("u_lightingMode", currentLightingMode);
        glm::mat4 groundModel = glm::translate(glm::mat4(1.0f), groundOffset);
        pyramidShaderProgram.setMat4("model", groundModel);
        groundSandTexture.texUnit(pyramidShaderProgram, "tex0");
        groundSandTexture.Bind();
        groundVAO.Bind();
        pyramidShaderProgram.setFloat("u_specularStrength", 0.05f);
        glDrawElements(GL_TRIANGLES, groundIndicesVec.size(), GL_UNSIGNED_INT, 0);

        cactusTexture.texUnit(pyramidShaderProgram, "tex0");
        cactusTexture.Bind();
        cactusSphereVAO.Bind();
        pyramidShaderProgram.setFloat("u_specularStrength", 0.2f);
        for (const auto& cactus_instance : cacti) { 
            cactus_instance.Draw(pyramidShaderProgram, sphereIndexCount, standardCactusPartsData);
        }

        
        pyramidTexture.texUnit(pyramidShaderProgram, "tex0");
        pyramidTexture.Bind();
        pyramidVAO.Bind();
        pyramidShaderProgram.setFloat("u_specularStrength", 0.7f);
        for (int i = 0; i < numPyramids; ++i) {
            glm::mat4 pyramidModel_instance = glm::mat4(1.0f); 
            pyramidModel_instance = glm::translate(pyramidModel_instance, pyramidPositions[i]);
            pyramidModel_instance = glm::rotate(pyramidModel_instance, glm::radians(pyramidYRotations[i]), glm::vec3(0.0f, 1.0f, 0.0f));
            pyramidModel_instance = glm::scale(pyramidModel_instance, glm::vec3(pyramidScales[i]));
            pyramidShaderProgram.setMat4("model", pyramidModel_instance);
            glDrawElements(GL_TRIANGLES, sizeof(pyramidIndices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
        }

        
        sunShaderProgram.Activate();
        
        sunShaderProgram.setMat4("camMatrix", combinedCamMatrix);
        glm::mat4 sunModel_instance = glm::mat4(1.0f); 
        sunModel_instance = glm::translate(sunModel_instance, lightPos);
        sunModel_instance = glm::scale(sunModel_instance, glm::vec3(sunRadius / baseSphereRadius));
        sunShaderProgram.setMat4("model", sunModel_instance);
        sunShaderProgram.setVec4("sunColor", sunTintColor); 
        sunTexture.Bind(); 
        sunVAO.Bind();
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
        glDepthFunc(GL_LEQUAL); 
        skybox.Draw(currentViewMatrix, currentProjectionMatrix);
        glDepthFunc(GL_LESS); //  domyślna funkcję głębokości

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    
    pyramidVAO.Delete(); pyramidVBO.Delete(); pyramidEBO.Delete();
    groundVAO.Delete(); groundVBO.Delete(); groundEBO.Delete();
    cactusSphereVAO.Delete(); sunVAO.Delete();
    sphereVBO.Delete(); sphereEBO.Delete(); // Współdzielone VBO/EBO usuwane raz
    pyramidTexture.Delete(); sunTexture.Delete(); groundSandTexture.Delete(); cactusTexture.Delete();
    pyramidShaderProgram.Delete(); sunShaderProgram.Delete();
    

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

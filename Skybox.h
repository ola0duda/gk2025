#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include "shaderClass.h" // Twoja klasa do obs³ugi shaderów

// stb_image.h zostanie do³¹czony przez Skybox.cpp

class Skybox
{
public:
    // Konstruktor przyjmuje œcie¿ki do plików shaderów skyboxa
    Skybox(const char* vertexPath, const char* fragmentPath);
    ~Skybox();

    // £aduje 6 tekstur œcian cubemapy.
    // Oczekiwana kolejnoœæ tekstur w wektorze 'faces':
    // 1. Prawo (+X)
    // 2. Lewo (-X)
    // 3. Góra (+Y)
    // 4. Dó³ (-Y)
    // 5. Przód (+Z w koordynatach tekstury, co odpowiada kierunkowi widoku -Z, jeœli na niego patrzymy)
    // 6. Ty³ (-Z w koordynatach tekstury, co odpowiada kierunkowi widoku +Z, jeœli na niego patrzymy)
    // Podaj pe³ne œcie¿ki do tekstur lub upewnij siê, ¿e znajduj¹ siê w katalogu roboczym.
    bool loadCubemap(std::vector<std::string> faces);

    // Rysuje skybox
    void Draw(const glm::mat4& view, const glm::mat4& projection);

private:
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int cubemapTextureID;
    Shader skyboxShader; // Skybox zarz¹dza w³asnym shaderem

    void setupSkybox(); // Prywatna metoda do konfiguracji VAO/VBO
};

#endif // SKYBOX_H
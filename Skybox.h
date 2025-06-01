#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include "shaderClass.h" // Twoja klasa do obs�ugi shader�w

// stb_image.h zostanie do��czony przez Skybox.cpp

class Skybox
{
public:
    // Konstruktor przyjmuje �cie�ki do plik�w shader�w skyboxa
    Skybox(const char* vertexPath, const char* fragmentPath);
    ~Skybox();

    // �aduje 6 tekstur �cian cubemapy.
    // Oczekiwana kolejno�� tekstur w wektorze 'faces':
    // 1. Prawo (+X)
    // 2. Lewo (-X)
    // 3. G�ra (+Y)
    // 4. D� (-Y)
    // 5. Prz�d (+Z w koordynatach tekstury, co odpowiada kierunkowi widoku -Z, je�li na niego patrzymy)
    // 6. Ty� (-Z w koordynatach tekstury, co odpowiada kierunkowi widoku +Z, je�li na niego patrzymy)
    // Podaj pe�ne �cie�ki do tekstur lub upewnij si�, �e znajduj� si� w katalogu roboczym.
    bool loadCubemap(std::vector<std::string> faces);

    // Rysuje skybox
    void Draw(const glm::mat4& view, const glm::mat4& projection);

private:
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int cubemapTextureID;
    Shader skyboxShader; // Skybox zarz�dza w�asnym shaderem

    void setupSkybox(); // Prywatna metoda do konfiguracji VAO/VBO
};

#endif // SKYBOX_H
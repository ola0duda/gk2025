#include "Skybox.h"
#include <iostream>

// Upewnij si�, �e STB_IMAGE_IMPLEMENTATION jest zdefiniowane tylko raz w projekcie.
// Obecnie znajduje si� w Texture.cpp.
// #define STB_IMAGE_IMPLEMENTATION // Jest ju� w Texture.cpp
#include "stb_image.h" // Do �adowania tekstur

// Wierzcho�ki skyboxa (tylko pozycje)
float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f
};

Skybox::Skybox(const char* vertexPath, const char* fragmentPath)
    : skyboxShader(vertexPath, fragmentPath), cubemapTextureID(0), skyboxVAO(0), skyboxVBO(0) {
    setupSkybox();
}

Skybox::~Skybox() {
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    if (cubemapTextureID != 0) {
        glDeleteTextures(1, &cubemapTextureID);
    }
    // Destruktor obiektu Shader powinien zaj�� si� zwolnieniem programu shadera (skyboxShader.Delete())
}

void Skybox::setupSkybox() {
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Pozycja wierzcho�ka
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0); // Od��cz VAO
}

// Kolejno�� �cianek tekstury: Prawo, Lewo, G�ra, D�, Prz�d(+Z), Ty�(-Z)
// Odpowiada to:
// GL_TEXTURE_CUBE_MAP_POSITIVE_X
// GL_TEXTURE_CUBE_MAP_NEGATIVE_X
// GL_TEXTURE_CUBE_MAP_POSITIVE_Y
// GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
// GL_TEXTURE_CUBE_MAP_POSITIVE_Z
// GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
bool Skybox::loadCubemap(std::vector<std::string> faces) {
    if (faces.size() != 6) {
        std::cerr << "ERROR::SKYBOX::LOAD_CUBEMAP::Oczekiwano 6 tekstur, otrzymano " << faces.size() << std::endl;
        return false;
    }

    glGenTextures(1, &cubemapTextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);

    // Cubemapy cz�sto nie wymagaj� odwracania wertykalnego,
    // lub zale�y to od �r�d�a tekstur. W razie problem�w mo�na zmieni� na true.
    // Twoja klasa Texture.cpp u�ywa stbi_set_flip_vertically_on_load(true) dla tekstur 2D.
    stbi_set_flip_vertically_on_load(false);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = GL_RGB; // Domy�lny format
            if (nrChannels == 1) format = GL_RED;
            else if (nrChannels == 3) format = GL_RGB;
            else if (nrChannels == 4) format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cerr << "ERROR::SKYBOX::LOAD_CUBEMAP::Nie uda�o si� za�adowa� tekstury cubemapy: " << faces[i] << std::endl;
            std::cerr << "STB Reason: " << stbi_failure_reason() << std::endl;
            // stbi_image_free(data); // Bezpieczne, nawet je�li data jest null
            glDeleteTextures(1, &cubemapTextureID); // Posprz�taj
            cubemapTextureID = 0;
            stbi_set_flip_vertically_on_load(true); // Przywr�� domy�lne ustawienie dla innych tekstur, je�li to konieczne
            return false;
        }
    }
    stbi_set_flip_vertically_on_load(true); // Przywr�� domy�lne ustawienie dla innych tekstur (je�li Texture.cpp tego oczekuje)

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // Od��cz tekstur�
    return true;
}

void Skybox::Draw(const glm::mat4& view, const glm::mat4& projection) {
    if (cubemapTextureID == 0 || skyboxVAO == 0) {
        // Skybox nie jest za�adowany lub skonfigurowany
        return;
    }

    // Zmie� funkcj� g��bi, aby test g��bi przechodzi�, gdy warto�ci s� r�wne zawarto�ci bufora g��bi.
    // Skybox powinien by� rysowany, je�li jego g��bia jest <= istniej�cej g��bi (kt�ra wyniesie 1.0 dzi�ki shaderowi).
    glDepthFunc(GL_LEQUAL);

    skyboxShader.Activate();
    // Macierz widoku dla skyboxa nie powinna zawiera� translacji.
    // Vertex shader (`skybox.vert`) zajmuje si� usuni�ciem translacji z macierzy 'view'.
    skyboxShader.setMat4("view", view);
    skyboxShader.setMat4("projection", projection);
    skyboxShader.setInt("skyboxTexture", 0); // Ustaw uniform samplera cubemapy

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0); // Aktywuj jednostk� teksturuj�c� 0
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);

    glDrawArrays(GL_TRIANGLES, 0, 36); // Rysuj sze�cian skyboxa

    glBindVertexArray(0); // Od��cz VAO
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // Od��cz tekstur� cubemapy
    glDepthFunc(GL_LESS); // Przywr�� domy�ln� funkcj� testu g��bi
}
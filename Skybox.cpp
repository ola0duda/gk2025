#include "Skybox.h"
#include <iostream>

// Upewnij siê, ¿e STB_IMAGE_IMPLEMENTATION jest zdefiniowane tylko raz w projekcie.
// Obecnie znajduje siê w Texture.cpp.
// #define STB_IMAGE_IMPLEMENTATION // Jest ju¿ w Texture.cpp
#include "stb_image.h" // Do ³adowania tekstur

// Wierzcho³ki skyboxa (tylko pozycje)
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
    // Destruktor obiektu Shader powinien zaj¹æ siê zwolnieniem programu shadera (skyboxShader.Delete())
}

void Skybox::setupSkybox() {
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Pozycja wierzcho³ka
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0); // Od³¹cz VAO
}

// Kolejnoœæ œcianek tekstury: Prawo, Lewo, Góra, Dó³, Przód(+Z), Ty³(-Z)
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

    // Cubemapy czêsto nie wymagaj¹ odwracania wertykalnego,
    // lub zale¿y to od Ÿród³a tekstur. W razie problemów mo¿na zmieniæ na true.
    // Twoja klasa Texture.cpp u¿ywa stbi_set_flip_vertically_on_load(true) dla tekstur 2D.
    stbi_set_flip_vertically_on_load(false);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = GL_RGB; // Domyœlny format
            if (nrChannels == 1) format = GL_RED;
            else if (nrChannels == 3) format = GL_RGB;
            else if (nrChannels == 4) format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cerr << "ERROR::SKYBOX::LOAD_CUBEMAP::Nie uda³o siê za³adowaæ tekstury cubemapy: " << faces[i] << std::endl;
            std::cerr << "STB Reason: " << stbi_failure_reason() << std::endl;
            // stbi_image_free(data); // Bezpieczne, nawet jeœli data jest null
            glDeleteTextures(1, &cubemapTextureID); // Posprz¹taj
            cubemapTextureID = 0;
            stbi_set_flip_vertically_on_load(true); // Przywróæ domyœlne ustawienie dla innych tekstur, jeœli to konieczne
            return false;
        }
    }
    stbi_set_flip_vertically_on_load(true); // Przywróæ domyœlne ustawienie dla innych tekstur (jeœli Texture.cpp tego oczekuje)

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // Od³¹cz teksturê
    return true;
}

void Skybox::Draw(const glm::mat4& view, const glm::mat4& projection) {
    if (cubemapTextureID == 0 || skyboxVAO == 0) {
        // Skybox nie jest za³adowany lub skonfigurowany
        return;
    }

    // Zmieñ funkcjê g³êbi, aby test g³êbi przechodzi³, gdy wartoœci s¹ równe zawartoœci bufora g³êbi.
    // Skybox powinien byæ rysowany, jeœli jego g³êbia jest <= istniej¹cej g³êbi (która wyniesie 1.0 dziêki shaderowi).
    glDepthFunc(GL_LEQUAL);

    skyboxShader.Activate();
    // Macierz widoku dla skyboxa nie powinna zawieraæ translacji.
    // Vertex shader (`skybox.vert`) zajmuje siê usuniêciem translacji z macierzy 'view'.
    skyboxShader.setMat4("view", view);
    skyboxShader.setMat4("projection", projection);
    skyboxShader.setInt("skyboxTexture", 0); // Ustaw uniform samplera cubemapy

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0); // Aktywuj jednostkê teksturuj¹c¹ 0
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);

    glDrawArrays(GL_TRIANGLES, 0, 36); // Rysuj szeœcian skyboxa

    glBindVertexArray(0); // Od³¹cz VAO
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // Od³¹cz teksturê cubemapy
    glDepthFunc(GL_LESS); // Przywróæ domyœln¹ funkcjê testu g³êbi
}
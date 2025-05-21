#include "Texture.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const char* image, GLenum texType, GLuint slot, GLenum format, GLenum pixelType)
{
    type = texType;
    unit = slot; //zapisanie jednostki teksturuj¹cej
    ID = 0;      //inicjowanie ID na 0

    int widthImg, heightImg, numColCh;
    stbi_set_flip_vertically_on_load(true); //odwrocenie obrazka - standard dla opengl
    //dla spojnosci wymuszenie ladowania rgba
    unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 4);

    if (!bytes)
    {
        std::cerr << "nie udalo siê za³adowaæ tekstury: " << image << ". Powod: " << stbi_failure_reason() << std::endl;
        return; //ID pozostaje 0
    }
    else
    {
        std::cout << "Tekstura '" << image << "' zaladowana pomyslnie. Wymiary: " << widthImg << "x" << heightImg << std::endl;
    }

    glGenTextures(1, &ID);
    glActiveTexture(GL_TEXTURE0 + unit); //aktywowanie jednostki teksturuj¹cej
    glBindTexture(texType, ID);

    //parametry owijania i teksturowania
    glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //mipmapy
    glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //za³adowanie danych obrazu do tekstury
    glTexImage2D(texType, 0, GL_RGBA, widthImg, heightImg, 0, GL_RGBA, pixelType, bytes);
    glGenerateMipmap(texType); //generowanie mipmapy

    stbi_image_free(bytes);    //zwolnienie pamieci obrazu
    glBindTexture(texType, 0);
}

void Texture::texUnit(Shader& shader, const char* uniform)
{
    if (ID == 0) {
        std::cerr << "wywolanie texUnit na nieprawidlowym obiekcie tekstury (ID=0)" << std::endl;
        return;
    }
    shader.Activate(); //aktywowanie shadera przed ustawieniami uniformu
    //pobranie lokalizacji uniformu samplera
    GLint texUniLoc = glGetUniformLocation(shader.ID, uniform);
    if (texUniLoc == -1) {
        std::cerr << "uniform sampler2D '" << uniform << "' nie znaleziony w shaderze (ID: " << shader.ID << ")!" << std::endl;
    }
    //ustawienie uniformu samplera na numer jednostki teksturuj¹cej
    glUniform1i(texUniLoc, unit);
}

void Texture::Bind()
{
    if (ID == 0) return;
    glActiveTexture(GL_TEXTURE0 + unit); //aktywowanie przypisanej jednostki steruj¹cej
    glBindTexture(type, ID);
}

void Texture::Unbind()
{
    if (ID == 0) return;
    glBindTexture(type, 0);
}

void Texture::Delete()
{
    if (ID == 0) return;
    glDeleteTextures(1, &ID);
    ID = 0;
}
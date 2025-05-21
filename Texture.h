#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include <glad/glad.h>
#include "shaderClass.h"
#include <string>
// stb_image.h w Texture.cpp

class Texture
{
public:
    GLuint ID;
    GLenum type;
    GLuint unit; //jednostka teksturuj�ca, z kt�r� ta tekstura jest powi�zana

    //Konstruktor
    //image: �cie�ka do pliku obrazu
    //texType: typ tekstury (np. GL_TEXTURE_2D)
    //slot: numer jednostki teksturuj�cej (np. 0 dla GL_TEXTURE0)
    //format: format danych obrazu (np. GL_RGBA)
    //pixelType: typ danych pikseli (np. GL_UNSIGNED_BYTE)
    Texture(const char* image, GLenum texType, GLuint slot, GLenum format, GLenum pixelType);

    //ustawia uniform samplera w shaderze
    void texUnit(Shader& shader, const char* uniform);
    //aktywuje jednostk� teksturuj�c� i binduje tekstur�
    void Bind();
    //unbinduje tekstur�
    void Unbind();
    //usuwa tekstur�
    void Delete();
};

#endif
#ifndef CACTUS_CLASS_H
#define CACTUS_CLASS_H

#include <glm/glm.hpp>
#include <vector>
#include "shaderClass.h"
#include "VAO.h" // Nadal potrzebujemy VAO, �eby Draw mog�o z nim pracowa� (chocia� go nie przyjmuje jako parametr)

// Struktura definiuj�ca pojedyncz� cz�� kaktusa (np. pie�, rami�)
// B�dzie ona statyczna i wsp�dzielona przez wszystkie instancje kaktus�w
struct CactusPart {
    glm::vec3 relativePosition; // Pozycja �rodka tej cz�ci wzgl�dem punktu (0,0,0) instancji kaktusa
    glm::vec3 scale;            // Skala sfery dla tej cz�ci
    glm::vec3 rotationAxis;     // O� obrotu (dla rotacji relatywnej cz�ci)
    float rotationAngle;        // K�t obrotu w stopniach (dla rotacji relatywnej cz�ci)
};

// Klasa reprezentuj�ca pojedyncz� instancj� kaktusa w �wiecie
class Cactus
{
public:
    glm::vec3 Position; // Pozycja tej konkretnej instancji kaktusa (X, Z oraz Y - wysoko�� na pod�o�u)
    float yRotation;    // Losowy obr�t wok� osi Y dla tej instancji kaktusa

    // Konstruktor
    Cactus(glm::vec3 pos, float rotationY = 0.0f); // Dodajemy argument dla losowego obrotu

    // Metoda do rysowania tej instancji kaktusa
    // Przyjmuje shader, liczb� indeks�w sfery i wsp�dzielone dane o cz�ciach kaktusa.
    // VAO sfery dla kaktus�w MUSI by� zbindowane ZEWN�TRZNIE przed wywo�aniem tej metody.
    void Draw(Shader& shader, GLsizei sphereIndexCount, const std::vector<CactusPart>& partsData) const;

    // Uwaga: Usuwanie zasob�w OpenGL (VAO, Texture) nie powinno by� w tej klasie,
    // poniewa� s� one wsp�dzielone. Zarz�dzanie nimi pozostaje w main.cpp.
};

#endif // CACTUS_CLASS_H
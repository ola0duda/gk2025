#ifndef CACTUS_CLASS_H
#define CACTUS_CLASS_H

#include <glm/glm.hpp>
#include <vector>
#include "shaderClass.h"
#include "VAO.h" // Nadal potrzebujemy VAO, ¿eby Draw mog³o z nim pracowaæ (chocia¿ go nie przyjmuje jako parametr)

// Struktura definiuj¹ca pojedyncz¹ czêœæ kaktusa (np. pieñ, ramiê)
// Bêdzie ona statyczna i wspó³dzielona przez wszystkie instancje kaktusów
struct CactusPart {
    glm::vec3 relativePosition; // Pozycja œrodka tej czêœci wzglêdem punktu (0,0,0) instancji kaktusa
    glm::vec3 scale;            // Skala sfery dla tej czêœci
    glm::vec3 rotationAxis;     // Oœ obrotu (dla rotacji relatywnej czêœci)
    float rotationAngle;        // K¹t obrotu w stopniach (dla rotacji relatywnej czêœci)
};

// Klasa reprezentuj¹ca pojedyncz¹ instancjê kaktusa w œwiecie
class Cactus
{
public:
    glm::vec3 Position; // Pozycja tej konkretnej instancji kaktusa (X, Z oraz Y - wysokoœæ na pod³o¿u)
    float yRotation;    // Losowy obrót wokó³ osi Y dla tej instancji kaktusa

    // Konstruktor
    Cactus(glm::vec3 pos, float rotationY = 0.0f); // Dodajemy argument dla losowego obrotu

    // Metoda do rysowania tej instancji kaktusa
    // Przyjmuje shader, liczbê indeksów sfery i wspó³dzielone dane o czêœciach kaktusa.
    // VAO sfery dla kaktusów MUSI byæ zbindowane ZEWNÊTRZNIE przed wywo³aniem tej metody.
    void Draw(Shader& shader, GLsizei sphereIndexCount, const std::vector<CactusPart>& partsData) const;

    // Uwaga: Usuwanie zasobów OpenGL (VAO, Texture) nie powinno byæ w tej klasie,
    // poniewa¿ s¹ one wspó³dzielone. Zarz¹dzanie nimi pozostaje w main.cpp.
};

#endif // CACTUS_CLASS_H
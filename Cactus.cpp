#include "Cactus.h"
#include <glm/gtc/matrix_transform.hpp> // Potrzebujemy tego do transformacji macierzy
#include <glad/glad.h>                  // Potrzebujemy OpenGL do glDrawElements
#include <vector>                       // Potrzebujemy wektora dla struktury części danych

// Konstruktor klasy Cactus - ustawia pozycję i obrót instancji
Cactus::Cactus(glm::vec3 pos, float rotationY)
    : Position(pos), yRotation(rotationY) // Lista inicjalizacyjna
{
    // Brak dodatkowej logiki w konstruktorze dla tego prostego przypadku
}

// Metoda do rysowania pojedynczej instancji kaktusa
// Przyjmuje shader, liczbę indeksów sfery i współdzielone dane o częściach kaktusa.
// VAO sfery dla kaktusów (np. cactusSphereVAO z main.cpp) MUSI być zbindowane ZEWNĘTRZNIE przed wywołaniem tej metody.
void Cactus::Draw(Shader& shader, GLsizei sphereIndexCount, const std::vector<CactusPart>& partsData) const
{
    // Shader i Tekstura kaktusa powinny być ZBINDOWANE ZEWNĘTRZNIE (w main)
    // VAO sfery dla kaktusów (np. cactusSphereVAO) powinno być ZBINDOWANE ZEWNĘTRZNIE (w main)
    // Uniformy kamery, światła, trybu oświetlenia, specular strength powinny być ustawione zewnętrznie.

    // Pętla przez wszystkie części składowe standardowego kaktusa
    for (const auto& part : partsData)
    {
        // 1. Oblicz macierz modelu dla tej konkretnej części, dla tej konkretnej instancji kaktusa.
        // Zaczynamy od macierzy jednostkowej
        glm::mat4 cactusPartModel = glm::mat4(1.0f);

        // Zastosuj GLOBALNĄ transformację instancji: Przesuń do jej pozycji, a następnie obróć wokół osi Y
        cactusPartModel = glm::translate(cactusPartModel, Position); // Przesuń do pozycji instancji
        cactusPartModel = glm::rotate(cactusPartModel, glm::radians(yRotation), glm::vec3(0.0f, 1.0f, 0.0f)); // Obróć instancję wokół Y

        

        glm::mat4 partTransformation = glm::mat4(1.0f);
        // Najpierw skaluj
        partTransformation = glm::scale(partTransformation, part.scale);
        // Potem obróć relatywnie
        partTransformation = glm::rotate(partTransformation, glm::radians(part.rotationAngle), part.rotationAxis);
        // Na końcu przesuń środek do part.relativePosition (WAŻNE: relativePosition jest po skalowaniu i obrocie relatywnym)
        partTransformation = glm::translate(partTransformation, part.relativePosition);


        // Połącz macierz instancji z macierzą części (w odpowiedniej kolejności mnożenia GLM)
        // Macierz_Instancji * Macierz_Części
        cactusPartModel = cactusPartModel * partTransformation;


        // 2. Ustaw macierz modelu w shaderze
        shader.setMat4("model", cactusPartModel);

        // 3. Rysuj bazową geometrię SFERY (VAO/VBO/EBO dla sfery muszą być zbindowane zewnętrznie w main)
        // Używamy liczby indeksów sfery przekazanej jako argument
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
    }
}

#include "Cactus.h"
#include <glm/gtc/matrix_transform.hpp> // Potrzebujemy tego do transformacji macierzy
#include <glad/glad.h>                  // Potrzebujemy OpenGL do glDrawElements
#include <vector>                       // Potrzebujemy wektora dla struktury czêœci danych

// Konstruktor klasy Cactus - ustawia pozycjê i obrót instancji
Cactus::Cactus(glm::vec3 pos, float rotationY)
    : Position(pos), yRotation(rotationY) // Lista inicjalizacyjna
{
    // Brak dodatkowej logiki w konstruktorze dla tego prostego przypadku
}

// Metoda do rysowania pojedynczej instancji kaktusa
// Przyjmuje shader, liczbê indeksów sfery i wspó³dzielone dane o czêœciach kaktusa.
// VAO sfery dla kaktusów (np. cactusSphereVAO z main.cpp) MUSI byæ zbindowane ZEWNÊTRZNIE przed wywo³aniem tej metody.
void Cactus::Draw(Shader& shader, GLsizei sphereIndexCount, const std::vector<CactusPart>& partsData) const
{
    // Shader i Tekstura kaktusa powinny byæ ZBINDOWANE ZEWNÊTRZNIE (w main)
    // VAO sfery dla kaktusów (np. cactusSphereVAO) powinno byæ ZBINDOWANE ZEWNÊTRZNIE (w main)
    // Uniformy kamery, œwiat³a, trybu oœwietlenia, specular strength powinny byæ ustawione zewnêtrznie.

    // Pêtla przez wszystkie czêœci sk³adowe standardowego kaktusa
    for (const auto& part : partsData)
    {
        // 1. Oblicz macierz modelu dla tej konkretnej czêœci, dla tej konkretnej instancji kaktusa.
        // Zaczynamy od macierzy jednostkowej
        glm::mat4 cactusPartModel = glm::mat4(1.0f);

        // Zastosuj GLOBALN¥ transformacjê instancji: Przesuñ do jej pozycji, a nastêpnie obróæ wokó³ osi Y
        cactusPartModel = glm::translate(cactusPartModel, Position); // Przesuñ do pozycji instancji
        cactusPartModel = glm::rotate(cactusPartModel, glm::radians(yRotation), glm::vec3(0.0f, 1.0f, 0.0f)); // Obróæ instancjê wokó³ Y

        // Teraz zastosuj transformacje RELATYWNE dla tej czêœci kaktusa (przesuniêcie, obrót relatywny, skalowanie)
        // Kolejnoœæ transformacji ma znaczenie! Skalowanie -> Obrót relatywny -> Przesuniêcie relatywne
        // Wektor relativePosition reprezentuje œrodek SFERY PO SKALOWANIU I OBRÓCIE RELATYWNYM, przesuniêty wzglêdem PUNKTU ODNIESIENIA INSTANCJI.
        // Poprawna kolejnoœæ dla czêœci: Najpierw skaluj bazow¹ sferê (œrodek w 0,0,0), potem obróæ j¹ relatywnie, na koñcu przesuñ jej œrodek do part.relativePosition.
        // Stosujemy to do macierzy, która JU¯ zawiera globalne transformacje instancji (Position, yRotation).
        // Czyli: Globalna(T * R_y) * Relatywna(T_part * R_part * S_part)
        // Ale w glm mno¿ymy macierze w odwrotnej kolejnoœci, wiêc: S_part * R_part * T_part * R_y * T
        // Nasza obecna macierz cactusPartModel zawiera T * R_y.
        // Musimy zastosowaæ do niej relatywne transformacje: T_part * R_part * S_part
        // Czyli: cactusPartModel = cactusPartModel * T_part * R_part * S_part; (mno¿enie od prawej)
        // Ale wygodniej jest myœleæ o tym jako o tworzeniu macierzy transformacji DLA CZÊŒCI i mno¿eniu przez macierz INSTANCJI.
        // Macierz transformacji dla czêœci: T_part * R_part * S_part
        // Macierz koñcowa dla czêœci: Macierz_Instancji * Macierz_Czêœci
        // Macierz_Instancji = Translate(Position) * RotateY(yRotation)

        glm::mat4 partTransformation = glm::mat4(1.0f);
        // Najpierw skaluj
        partTransformation = glm::scale(partTransformation, part.scale);
        // Potem obróæ relatywnie
        partTransformation = glm::rotate(partTransformation, glm::radians(part.rotationAngle), part.rotationAxis);
        // Na koñcu przesuñ œrodek do part.relativePosition (WA¯NE: relativePosition jest po skalowaniu i obrocie relatywnym)
        partTransformation = glm::translate(partTransformation, part.relativePosition);


        // Po³¹cz macierz instancji z macierz¹ czêœci (w odpowiedniej kolejnoœci mno¿enia GLM)
        // Macierz_Instancji * Macierz_Czêœci
        cactusPartModel = cactusPartModel * partTransformation;


        // 2. Ustaw macierz modelu w shaderze
        shader.setMat4("model", cactusPartModel);

        // 3. Rysuj bazow¹ geometriê SFERY (VAO/VBO/EBO dla sfery musz¹ byæ zbindowane zewnêtrznie w main)
        // U¿ywamy liczby indeksów sfery przekazanej jako argument
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
    }
}
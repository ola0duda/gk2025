#include "Cactus.h"
#include <glm/gtc/matrix_transform.hpp> // Potrzebujemy tego do transformacji macierzy
#include <glad/glad.h>                  // Potrzebujemy OpenGL do glDrawElements
#include <vector>                       // Potrzebujemy wektora dla struktury cz�ci danych

// Konstruktor klasy Cactus - ustawia pozycj� i obr�t instancji
Cactus::Cactus(glm::vec3 pos, float rotationY)
    : Position(pos), yRotation(rotationY) // Lista inicjalizacyjna
{
    // Brak dodatkowej logiki w konstruktorze dla tego prostego przypadku
}

// Metoda do rysowania pojedynczej instancji kaktusa
// Przyjmuje shader, liczb� indeks�w sfery i wsp�dzielone dane o cz�ciach kaktusa.
// VAO sfery dla kaktus�w (np. cactusSphereVAO z main.cpp) MUSI by� zbindowane ZEWN�TRZNIE przed wywo�aniem tej metody.
void Cactus::Draw(Shader& shader, GLsizei sphereIndexCount, const std::vector<CactusPart>& partsData) const
{
    // Shader i Tekstura kaktusa powinny by� ZBINDOWANE ZEWN�TRZNIE (w main)
    // VAO sfery dla kaktus�w (np. cactusSphereVAO) powinno by� ZBINDOWANE ZEWN�TRZNIE (w main)
    // Uniformy kamery, �wiat�a, trybu o�wietlenia, specular strength powinny by� ustawione zewn�trznie.

    // P�tla przez wszystkie cz�ci sk�adowe standardowego kaktusa
    for (const auto& part : partsData)
    {
        // 1. Oblicz macierz modelu dla tej konkretnej cz�ci, dla tej konkretnej instancji kaktusa.
        // Zaczynamy od macierzy jednostkowej
        glm::mat4 cactusPartModel = glm::mat4(1.0f);

        // Zastosuj GLOBALN� transformacj� instancji: Przesu� do jej pozycji, a nast�pnie obr�� wok� osi Y
        cactusPartModel = glm::translate(cactusPartModel, Position); // Przesu� do pozycji instancji
        cactusPartModel = glm::rotate(cactusPartModel, glm::radians(yRotation), glm::vec3(0.0f, 1.0f, 0.0f)); // Obr�� instancj� wok� Y

        // Teraz zastosuj transformacje RELATYWNE dla tej cz�ci kaktusa (przesuni�cie, obr�t relatywny, skalowanie)
        // Kolejno�� transformacji ma znaczenie! Skalowanie -> Obr�t relatywny -> Przesuni�cie relatywne
        // Wektor relativePosition reprezentuje �rodek SFERY PO SKALOWANIU I OBR�CIE RELATYWNYM, przesuni�ty wzgl�dem PUNKTU ODNIESIENIA INSTANCJI.
        // Poprawna kolejno�� dla cz�ci: Najpierw skaluj bazow� sfer� (�rodek w 0,0,0), potem obr�� j� relatywnie, na ko�cu przesu� jej �rodek do part.relativePosition.
        // Stosujemy to do macierzy, kt�ra JU� zawiera globalne transformacje instancji (Position, yRotation).
        // Czyli: Globalna(T * R_y) * Relatywna(T_part * R_part * S_part)
        // Ale w glm mno�ymy macierze w odwrotnej kolejno�ci, wi�c: S_part * R_part * T_part * R_y * T
        // Nasza obecna macierz cactusPartModel zawiera T * R_y.
        // Musimy zastosowa� do niej relatywne transformacje: T_part * R_part * S_part
        // Czyli: cactusPartModel = cactusPartModel * T_part * R_part * S_part; (mno�enie od prawej)
        // Ale wygodniej jest my�le� o tym jako o tworzeniu macierzy transformacji DLA CZʌCI i mno�eniu przez macierz INSTANCJI.
        // Macierz transformacji dla cz�ci: T_part * R_part * S_part
        // Macierz ko�cowa dla cz�ci: Macierz_Instancji * Macierz_Cz�ci
        // Macierz_Instancji = Translate(Position) * RotateY(yRotation)

        glm::mat4 partTransformation = glm::mat4(1.0f);
        // Najpierw skaluj
        partTransformation = glm::scale(partTransformation, part.scale);
        // Potem obr�� relatywnie
        partTransformation = glm::rotate(partTransformation, glm::radians(part.rotationAngle), part.rotationAxis);
        // Na ko�cu przesu� �rodek do part.relativePosition (WA�NE: relativePosition jest po skalowaniu i obrocie relatywnym)
        partTransformation = glm::translate(partTransformation, part.relativePosition);


        // Po��cz macierz instancji z macierz� cz�ci (w odpowiedniej kolejno�ci mno�enia GLM)
        // Macierz_Instancji * Macierz_Cz�ci
        cactusPartModel = cactusPartModel * partTransformation;


        // 2. Ustaw macierz modelu w shaderze
        shader.setMat4("model", cactusPartModel);

        // 3. Rysuj bazow� geometri� SFERY (VAO/VBO/EBO dla sfery musz� by� zbindowane zewn�trznie w main)
        // U�ywamy liczby indeks�w sfery przekazanej jako argument
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
    }
}
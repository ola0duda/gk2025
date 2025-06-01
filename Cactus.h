#ifndef CACTUS_CLASS_H
#define CACTUS_CLASS_H

#include <glm/glm.hpp>
#include <vector>
#include "shaderClass.h"
#include "VAO.h" 


struct CactusPart {
    glm::vec3 relativePosition; 
    glm::vec3 scale;            
    glm::vec3 rotationAxis;     
    float rotationAngle;        
};


class Cactus
{
public:
    glm::vec3 Position; 
    float yRotation;    


    Cactus(glm::vec3 pos, float rotationY = 0.0f); 


    void Draw(Shader& shader, GLsizei sphereIndexCount, const std::vector<CactusPart>& partsData) const;

 
};

#endif 

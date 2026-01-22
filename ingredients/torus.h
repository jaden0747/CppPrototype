#pragma once

#include "cookbookogl.h"
#include "trianglemesh.h"

class Torus : public TriangleMesh
{
public:
    Torus(GLfloat outerRadius, GLfloat innerRadius, GLuint nsides, GLuint nrings);
};

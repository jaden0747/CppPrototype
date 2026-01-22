#pragma once

#include "cookbookogl.h"
#include "trianglemesh.h"

class Sphere : public TriangleMesh
{
public:
    Sphere(float rad, GLuint sl, GLuint st);
};

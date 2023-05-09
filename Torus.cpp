/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The sphere class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Torus.h"
#include "Solver.h"
#include <math.h>

/**
* Sphere's intersection method.  The input is a ray. 
*/
bool Torus::preIntersect(glm::vec3 p0, glm::vec3 dir)
{
    glm::vec3 vdif = p0 - center;   //Vector s (see Slide 28)
    float b = glm::dot(dir, vdif);
    float len = glm::length(vdif);
    float radius = tubeRadius + sweptRadius;
    float c = len*len - radius*radius;
    float delta = b*b - c;
   
    if(delta < 0.001) return true;    //includes zero and negative values

    float t1 = -b - sqrt(delta);
    float t2 = -b + sqrt(delta);

	if (t1 < 0)
	{
        return (t2 > 0) ? false : true;
	}
    else return false;
}



/**
* code based of http://blog.marcinchwedczuk.pl/ray-tracing-torus
*/
float Torus::intersect(glm::vec3 p0, glm::vec3 dir)
{
    if (preIntersect(p0,dir)) {
        return -1;
    }
    const glm::vec3 p = p0 - center;

    float ox = p.x;
    float oy = p.y;
    float oz = p.z;

    float dx = dir.x;
    float dy = dir.y;
    float dz = dir.z;

    float sum_d_sqrd = dx * dx + dy * dy + dz * dz;
    float e = ox * ox + oy * oy + oz * oz -  sweptRadius * sweptRadius - tubeRadius * tubeRadius;
    float f = ox * dx + oy * dy + oz * dz;
    float four_a_sqrd = 4.0 * sweptRadius * sweptRadius;

    return getSmallistSol(
                e * e - four_a_sqrd * (tubeRadius*tubeRadius - oy * oy),
                4.0 * f * e + 2.0 * four_a_sqrd * oy * dy,
                2.0 * sum_d_sqrd * e + 4.0 * f * f + four_a_sqrd * dy * dy,
                4.0 * sum_d_sqrd * f,
                sum_d_sqrd * sum_d_sqrd);
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
glm::vec3 Torus::normal(glm::vec3 p)
{
    const glm::vec3 p0 = p - center;
    float x = p0.x;
    float y = p0.y;
    float z = p0.z;

    float sumSquared = x * x + y * y + z * z;

    const float paramSquared = sweptRadius*sweptRadius + tubeRadius* tubeRadius;

    glm::vec3 temp = glm::vec3(
                4.0 * x * (sumSquared - paramSquared),
                4.0 * y * (sumSquared - paramSquared + 2.0*sweptRadius*sweptRadius),
                4.0 * z * (sumSquared - paramSquared)
                );
    return glm::normalize(temp);
}

/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The sphere class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Solver.h"
#include <float.h>
#include "Roots3And4.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <list>
using namespace std;

float getSmallistSol(float c0, float c1, float c2, float c3, float c4) {
    float c[] = {c0,c1,c2,c3,c4};
    float s[4];
    int count = SolveQuartic(c,s);

    if (count == 0) {
        return -1;
    }

    float min = FLT_MAX;
    bool noMin = true;

    for (int i = 0; i < 4; i++) {
        float f = s[i];
        if (f > 0.1 && f < min) {
            min = f;
            noMin = false;
        }
    }
    if (noMin) {
        return -1.0f;
    } else {
        return min;
    }


}

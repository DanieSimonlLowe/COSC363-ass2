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

float EQN_ESP = 0.1;
/*
bool isZero(float x) {
    return (x > -EQN_ESP && x < EQN_ESP);
}


float func(float c0, float c1, float c2, float c3, float c4, float x) {
    return c0 * pow(x,4) + c1 * pow(x,3) + c2 * x * x + c3 * x + c4;
}

float changeVal(float c0, float c1, float c2, float c3, float c4, float x, float f) {
    float f1 = 4 * c0 * pow(x,3) + 3 * c1 * x * x + 2 * c2 * x + c3;
    return -f/f1;
}*/

float getSmallistSol(float c0, float c1, float c2, float c3, float c4) {
    double c[] = {c0,c1,c2,c3,c4};
    double s[4];
    int count = SolveQuartic(c,s);

    if (count == 0) {
        return -1;
    }

    float min = FLT_MAX;
    bool noMin = true;

    for (int i = 0; i < count; i++) {
        float f = s[i];
        if (f > 0 && f < min) {
            min = f;
            noMin = false;
        }
    }
    if (noMin) {
        return -1.0f;
    } else {
        return min;
    }


    /*float x = 0.1;
    unsigned int count = 0;
    float f;
    f = func(c0,c1,c2,c3,c4,x);
    do {
        x += changeVal(c0,c1,c2,c3,c4,x,f);
        if (x < 0) {
            return -1;
        }
        count += 1;
        if (count == 100) {
            return -1;
        }
        f = func(c0,c1,c2,c3,c4,x);
    } while (!isZero(f));
    cout << x << endl;
    return x;*/
}

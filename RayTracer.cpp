/*==================================================================================
* COSC 363  Computer Graphics (2023)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab06.pdf   for details.
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "Torus.h"
#include "SceneObject.h"
#include "Ray.h"
#include "Plane.h"
#include <GL/freeglut.h>
using namespace std;


const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;
const float ZERO = 0.001;


vector<SceneObject*> sceneObjects;
const glm::vec3 lightPos(15, 20, 30);					//Light's position


glm::vec3 trace(Ray ray, int step);

//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step, bool hasRefrac)
{

    const glm::vec3 fogColor(0.36,0.73,0.16);
    const float fogDist = 220;
    const float diffZ = 20;
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
    if (step >= MAX_STEPS) {
        return backgroundCol; //defult color
    }

	glm::vec3 color(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return fogColor;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found
    if (ray.index == 4)
    {
        const int tileWidth = 5;
        int iz = (ray.hit.z / tileWidth) + 300;
        int ix = (ray.hit.x / tileWidth) + 300;
        int k = (iz+ix) % 2;
        if (k == 0) {
            color = glm::vec3(1,1,1);
        } else {
            color = glm::vec3(0.5,0.5,0.5);
        }
        obj->setColor(color);
    } else if (ray.index == 5) {
        float fogCof = (abs(ray.hit.z - diffZ) / fogDist);
        return fogCof * fogColor + (1.0f-fogCof) * obj->getColor();
    } else if (ray.index == 6) {
        const float centerX = 5;
        const float centerZ = -100;
        const float intensity = 0.2;
        const float minIntensity = 1;
        const int tileSize = 3;
        float dist = sqrt(pow(ray.hit.x - centerX,2) + pow(ray.hit.z - centerZ,2));
        int temp = dist/tileSize;
        int k = temp % 3;
        float ram = (dist / (float)tileSize) - temp;
        if (k == 0) {
            color = (1-ram) * glm::vec3(1,0.0,0.0) + (ram) * glm::vec3(0.0,1,0.0);
        } else if (k==1) {
            color = (1-ram) * glm::vec3(0.0,1,0.0) + (ram) * glm::vec3(0.0,0.0,1);
        } else {
            color = (1-ram) * glm::vec3(0.0,0.0,1) + (ram) * glm::vec3(1,0.0,0.0);
        }
        obj->setColor((intensity * dist + minIntensity) * color);
    }

    //(glm::vec3 lightPos, glm::vec3 viewVec, glm::vec3 hit)
    glm::vec3 viewVec = ray.dir * -1.0f;
    color = obj->lighting(lightPos,viewVec,ray.hit);						//Object's colour

    if (obj->isTransparent()) {
        Ray tranRay(ray.hit+0.1f*ray.dir,ray.dir);
        float coeff = obj->getTransparencyCoeff();
        glm::vec3 colorTrans = coeff*trace(tranRay,step + 1);
        color = color+colorTrans;
    }

    if (obj->isReflective()) {
        glm::vec3 relDir = glm::reflect(ray.dir,obj->normal(ray.hit));
        Ray relRay(ray.hit,relDir);
        glm::vec3 colorRel = obj->getReflectionCoeff()*trace(relRay,step + 1);
        color = color + colorRel;
    }

    if (obj->isRefractive()) {
        glm::vec3 refDir;
        if (hasRefrac) {
            refDir = glm::refract(ray.dir,-1.0f*obj->normal(ray.hit),1.0f/obj->getRefractiveIndex());
        } else {
            refDir = glm::refract(ray.dir,obj->normal(ray.hit),obj->getRefractiveIndex());
        }
        Ray refRay(ray.hit,refDir);
        glm::vec3 colorRel = obj->getReflectionCoeff()*trace(refRay,step + 1, !hasRefrac);
        color = color + colorRel;
    }

    glm::vec3 lightVec = lightPos - ray.hit;
    Ray shadowRay(ray.hit,lightVec);
    shadowRay.closestPt(sceneObjects);
    if (shadowRay.index != 5) {
        float mult = 0.2;
        SceneObject* shObj;
        shObj = sceneObjects[shadowRay.index];
        if (shObj->isTransparent()) { // do refraction
            mult += (0.8-mult)*shObj->getTransparencyCoeff();
        }
        if (shObj->isRefractive()) { // do refraction
            mult += (0.8-mult)*shObj->getRefractionCoeff();
        }
        color = color * mult;
    }

    float fogCof = (abs(ray.hit.z - diffZ) / fogDist);
    color = fogCof * fogColor + (1.0f-fogCof) * color;


	return color;
}

glm::vec3 trace(Ray ray, int step) {
    return trace(ray,step,false);
}

float calculateColorDiffSqear(glm::vec3 colorA, glm::vec3 colorB) {
    float diffR = colorA.r - colorB.r;
    float diffB = colorA.b - colorB.b;
    float diffG = colorA.g - colorB.g;
    return diffR * diffR + diffB * diffB + diffG * diffG;
}

const float minSqearDiff = 0.01;
bool shouldAntiAlias(glm::vec3 cols[NUMDIV][NUMDIV], int i, int j) {

    glm::vec3 color = cols[i][j];
    if (i > 0 && calculateColorDiffSqear(color,cols[i-1][j]) > minSqearDiff) {
        return true;
    }
    if (i < NUMDIV && calculateColorDiffSqear(color,cols[i+1][j]) > minSqearDiff) {
        return true;
    }
    if (j > 0 && calculateColorDiffSqear(color,cols[i][j-1]) > minSqearDiff) {
        return true;
    }
    if (j < NUMDIV && calculateColorDiffSqear(color,cols[i][j+1]) > minSqearDiff) {
        return true;
    }

    if (i > 0 && j > 0 && calculateColorDiffSqear(color,cols[i-1][j-1]) > minSqearDiff) {
        return true;
    }
    if (i > 0 && j < NUMDIV && calculateColorDiffSqear(color,cols[i-1][j+1]) > minSqearDiff) {
        return true;
    }
    if (i < NUMDIV && j > 0 && calculateColorDiffSqear(color,cols[i+1][j-1]) > minSqearDiff) {
        return true;
    }
    if (i < NUMDIV && j < NUMDIV && calculateColorDiffSqear(color,cols[i+1][j+1]) > minSqearDiff) {
        return true;
    }
    return false;
}

bool shouldSubDivide(glm::vec3 color[4], int n) {
    for (int i = 0; i < 4; i++) {
        if (i != n && calculateColorDiffSqear(color[n],color[i]) < minSqearDiff) {
            return false;
        }
    }
    return true;
}

glm::vec3 getAvgColor(glm::vec3 colors[4]) {
    float sumR = 0;
    float sumG = 0;
    float sumB = 0;
    for (int i = 0; i < 4; i++) {
        sumR += colors[i].r;
        sumB += colors[i].b;
        sumG += colors[i].g;
    }
    glm::vec3 color(sumR/4,sumG/4,sumB/4);
    return color;
}

const int MAX_SUBDIV_STEP = 5;

glm::vec3 subDivide(float cellX, float cellY, float xMin, float yMin, int step, glm::vec3 eye) {
    float minDiff = 0.1;
    glm::vec3 colors[4];
    for (int k = 0; k < 2; k++) {
        float xp = xMin + cellX + k * cellX*0.5;
        for (int l = 0; l < 2; l++) {
            float yp = yMin + cellY + l * cellY*0.5;
            glm::vec3 dir(xp + 0.25 * cellX, yp + 0.25 * cellY, -EDIST);	//direction of the primary ray

            Ray ray = Ray(eye, dir);
            colors[k*2+l] = trace(ray, 1);
        }
    }
    if (step >= MAX_SUBDIV_STEP) {
        return getAvgColor(colors);
    }
    glm::vec3 newColors[4];

    if (shouldSubDivide(colors,0)) { // y=x=0
        newColors[0] = subDivide(cellX/2,cellY/2,xMin,yMin,step+1,eye);
    } else {
        newColors[0] = colors[0];
    }
    if (shouldSubDivide(colors,1)) { // y=1
        newColors[1] = subDivide(cellX/2,cellY/2,xMin,yMin+cellY,step+1,eye);
    } else {
        newColors[1] = colors[1];
    }
    if (shouldSubDivide(colors,2)) { // x=1 y=0
        newColors[2] = subDivide(cellX/2,cellY/2,xMin+cellX,yMin,step+1,eye);
    } else {
        newColors[2] = colors[2];
    }
    if (shouldSubDivide(colors,2)) { // x=1 y=1
        newColors[3] = subDivide(cellX/2,cellY/2,xMin+cellX,yMin+cellY,step+1,eye);
    } else {
        newColors[3] = colors[3];
    }

    return getAvgColor(newColors);
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX - XMIN) / NUMDIV;  //cell width
	float cellY = (YMAX - YMIN) / NUMDIV;  //cell height
    glm::vec3 eye(5, 0., 20);

    glm::vec3 cols[NUMDIV][NUMDIV] = {};


    //get intal colors;
	for (int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
	{
		xp = XMIN + i * cellX;
		for (int j = 0; j < NUMDIV; j++)
        {
            yp = YMIN + j * cellY;

            glm::vec3 dir(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);	//direction of the primary ray

			Ray ray = Ray(eye, dir);

            cols[i][j] = trace(ray, 1); //Trace the primary ray and get the colour value

		}
	}

    glm::vec3 disCols[NUMDIV][NUMDIV] = {};
    for (int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
    {
        xp = XMIN + i * cellX;
        for (int j = 0; j < NUMDIV; j++)
        {

            if (shouldAntiAlias(cols,i,j)) {
                yp = YMIN + j * cellY;
                disCols[i][j] = subDivide(cellX,cellY,xp,yp,1,eye);

            } else {
                disCols[i][j] = cols[i][j];
            }

        }
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);  //Each cell is a tiny quad.
    for (int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
    {
        xp = XMIN + i * cellX;
        for (int j = 0; j < NUMDIV; j++)
        {
            yp = YMIN + j * cellY;
            glColor3f(disCols[i][j].r, disCols[i][j].g, disCols[i][j].b);
            glVertex2f(xp, yp);				//Draw each cell with its color value
            glVertex2f(xp + cellX, yp);
            glVertex2f(xp + cellX, yp + cellY);
            glVertex2f(xp, yp + cellY);
        }
    }

    glEnd();
    glFlush();
}



//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL 2D orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

    glClearColor(0, 0, 0, 1);

    Sphere *spherer = new Sphere(glm::vec3(7.0, 5.0, -70.0), 3.0);
    spherer->setColor(glm::vec3(1, 0, 0));   //Set colour to red
    spherer->setTransparency(true,0.8f);
    sceneObjects.push_back(spherer);		 //Add sphere to scene objects

    Sphere *sphereg = new Sphere(glm::vec3(7.0, -9.0, -70.0), 6.5);
    sphereg->setColor(glm::vec3(0.25, 0.25, 0.25));   //Set colour to green
    sphereg->setRefractivity(true,1.0,0.98);
    sceneObjects.push_back(sphereg);		 //Add sphere to scene objects


    Torus *sphereb = new Torus(glm::vec3(0.0, 13.0, -70.0), 15.0, 1.0);
    sphereb->setColor(glm::vec3(0.6, 0.95, 1));   //Set colour to white
    sphereb->setShininess(5);
    sceneObjects.push_back(sphereb);		 //Add sphere to scene objects

    Sphere *sphere1 = new Sphere(glm::vec3(-5.0, 0.0, -90.0), 12.0);
    sphere1->setColor(glm::vec3(0, 0, 1));   //Set colour to blue
    sphere1->setReflectivity(true,0.5);
    sceneObjects.push_back(sphere1);		 //Add sphere to scene objects

    Plane *floorPlane = new Plane(glm::vec3(-20,-15,40),
                             glm::vec3(20,-15,40),
                             glm::vec3(20,-15,-200),
                             glm::vec3(-20,-15,-200));
    floorPlane->setColor(glm::vec3(0.8,0.8,0));
    sceneObjects.push_back(floorPlane);

    Sphere *light = new Sphere(lightPos, 5);
    light->setColor(glm::vec3(1,1,1));
    sceneObjects.push_back(light);

    Plane *ceilingPlane = new Plane(glm::vec3(-20,20,40),
                             glm::vec3(20,20,40),
                             glm::vec3(20,20,-200),
                             glm::vec3(-20,20,-200));
    ceilingPlane->setColor(glm::vec3(0.8,0.8,0));
    sceneObjects.push_back(ceilingPlane);

    Plane *leftPlane = new Plane(glm::vec3(-20,20,40),
                             glm::vec3(-20,-15,40),
                             glm::vec3(-20,-15,-200),
                             glm::vec3(-20,20,-200));
    leftPlane->setColor(glm::vec3(0,0,0));
    leftPlane->setReflectivity(true,1.0);
    sceneObjects.push_back(leftPlane);

    Plane *rightPlane = new Plane(glm::vec3(20,20,40),
                             glm::vec3(20,20,-200),
                             glm::vec3(20,-15,-200),
                             glm::vec3(20,-15,40)
                                  );
    rightPlane->setColor(glm::vec3(0,0,0));
    rightPlane->setReflectivity(true,1.0);
    sceneObjects.push_back(rightPlane);


    Plane *farPlane = new Plane(
                glm::vec3(-20,-15,-200),
                glm::vec3(20,-15,-200),
                glm::vec3(20,20,-200),
                glm::vec3(-20,20,-200)
                                );
    farPlane->setColor(glm::vec3(0.5,0.5,0.5));
    farPlane->setSpecularity(false);
    sceneObjects.push_back(farPlane);

    Plane *backPlane = new Plane(
                glm::vec3(-20,20,40),
                glm::vec3(20,20,40),
                glm::vec3(20,-15,40),
                glm::vec3(-20,-15,40));
    backPlane->setColor(glm::vec3(0.5,0.5,0.5));
    backPlane->setSpecularity(false);
    sceneObjects.push_back(backPlane);


}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}

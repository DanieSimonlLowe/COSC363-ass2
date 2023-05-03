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


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
    if (step >= MAX_STEPS) {
        return backgroundCol; //defult color
    }
	glm::vec3 lightPos(10, 40, -3);					//Light's position
	glm::vec3 color(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
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
    }

    //(glm::vec3 lightPos, glm::vec3 viewVec, glm::vec3 hit)
    glm::vec3 viewVec = ray.dir * -1.0f;
    color = obj->lighting(lightPos,viewVec,ray.hit);						//Object's colour

    if (obj->isTransparent()) {
        glm::vec3 tranVec = ray.hit + ray.dir;
        Ray tranRay(tranVec,ray.dir);
        glm::vec3 colorTrans = obj->getTransparencyCoeff()*trace(tranRay,step + 1);
        color = color+colorTrans;
    }

    if (obj->isReflective()) {
        //glm::vec3 relVec = ray.hit;
        glm::vec3 relDir = glm::reflect(ray.dir,obj->normal(ray.hit));
        Ray relRay(ray.hit,relDir);
        glm::vec3 colorRel = obj->getReflectionCoeff()*trace(relRay,step + 1);
        color = color + colorRel;
    }

    if (obj->isRefractive()) {
        glm::vec3 refVec = ray.hit - 0.1f * ray.dir;
        glm::vec3 refDir = glm::refract(ray.dir,obj->normal(ray.hit),obj->getRefractiveIndex());
        Ray refRay(refVec,refDir);
        glm::vec3 colorRel = obj->getReflectionCoeff()*trace(refRay,step + 1);
        color = color + colorRel;
    }

    glm::vec3 lightVec = lightPos - ray.hit;
    Ray shadowRay(ray.hit,lightVec);
    shadowRay.closestPt(sceneObjects);
    if (shadowRay.index != -1) {
        if (sceneObjects[shadowRay.index]->isTransparent()) { //TODO add trans shadows recursion
            float coefficant = 0.2f + 0.8f * obj->getTransparencyCoeff();
            color = coefficant * color;
        } else {
            color = 0.2f * color;
        }
    }

	return color;
}

float calculateColorDiffSqear(glm::vec3 colorA, glm::vec3 colorB) {
    float diffR = colorA.r - colorB.r;
    float diffB = colorA.b - colorB.b;
    float diffG = colorA.g - colorB.g;
    return diffR * diffR + diffB * diffB + diffG * diffG;
}

bool shouldAntiAlias(glm::vec3 cols[NUMDIV][NUMDIV], int i, int j) {
    const float minSqearDiff = 0.01;
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

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX - XMIN) / NUMDIV;  //cell width
	float cellY = (YMAX - YMIN) / NUMDIV;  //cell height
	glm::vec3 eye(0., 0., 0.);

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
        for (int j = 0; j < NUMDIV; j++)
        {
            if (shouldAntiAlias(cols,i,j)) {
                float sumR = cols[i][j].r;
                float sumG = cols[i][j].g;
                float sumB = cols[i][j].b;
                for (int k = 0; k < 2; k++) {
                    xp = XMIN + i * cellX + k * cellX*0.5;
                    for (int l = 0; l < 2; l++) {
                        yp = YMIN + j * cellY + l * cellY*0.5;
                        glm::vec3 dir(xp + 0.25 * cellX, yp + 0.25 * cellY, -EDIST);	//direction of the primary ray

                        Ray ray = Ray(eye, dir);
                        glm::vec3 col = trace(ray, 1);
                        sumR += col.r;
                        sumG += col.g;
                        sumB += col.b;
                    }
                }
                disCols[i][j] = glm::vec3(sumR*0.2,sumG*0.2,sumB*0.2);

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
    spherer->setTransparency(true,0.8);
    sceneObjects.push_back(spherer);		 //Add sphere to scene objects

    Sphere *sphereg = new Sphere(glm::vec3(7.0, -9.0, -70.0), 6.5);
    sphereg->setColor(glm::vec3(0.2, 1, 0.2));   //Set colour to green
    sceneObjects.push_back(sphereg);		 //Add sphere to scene objects

    Sphere *sphereb = new Sphere(glm::vec3(13.0, 13.0, -70.0), 3.0);
    sphereb->setColor(glm::vec3(0.6, 0.95, 1));   //Set colour to red
    sphereb->setShininess(5);
    sceneObjects.push_back(sphereb);		 //Add sphere to scene objects

    Sphere *sphere1 = new Sphere(glm::vec3(-5.0, 0.0, -90.0), 15.0);
    sphere1->setColor(glm::vec3(0, 0, 1));   //Set colour to blue
    sphere1->setSpecularity(false);
    sphere1->setReflectivity(true,0.5);
    sceneObjects.push_back(sphere1);		 //Add sphere to scene objects

    Plane *floorPlane = new Plane(glm::vec3(-20,-15,-40),
                             glm::vec3(20,-15,-40),
                             glm::vec3(20,-15,-200),
                             glm::vec3(-20,-15,-200));
    floorPlane->setColor(glm::vec3(0.8,0.8,0));
    sceneObjects.push_back(floorPlane);
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
/*
 * sphere.h
 *
 *  Created on: Mar 24, 2009
 *      Author: zrui
 */

#ifndef SPHERE_H_
#define SPHERE_H_
#include <math.h>
#include "stdlib.h"

class Sphere{
public:
	Sphere(double* p){
		center[0] = p[0];	center[1] = p[1];	center[2] = p[2];
		radius = 0;
		for(int i=0; i<3; i++) radius += (p[i+3]-p[i])*(p[i+3]-p[i]);
		radius = sqrt(radius);
	}
	Sphere(double *p, double radius){
		center[0] = p[0];	center[1] = p[1];	center[2] = p[2];
		this->radius = radius;
	}
	virtual ~Sphere(){
		if(center != NULL)
			free(center);
	};
	double center[3];
	double radius;


private:


};

#endif /* SPHERE_H_ */

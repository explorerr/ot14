/*
 *  Arc.h
 *
 *  Created on: Oct 30, 2009
 *      Author: zrui
 */

#ifndef ARC_H_
#define ARC_H_
#include <math.h>

class Arc{
public:
	Arc(vector<double> info){
        if(info.size()<8)
            throw StoppingException("Error: Arc constructor, not enough information given.\n");
        center[0] = info[0]; center[1] = info[1]; center[2] = info[2];
        radius = info[3];
        end[0] = info[4]; end[1] = info[5]; end[2] = info[6];
        angle = info[7];
        n[0] = end[0]-center[0];
        n[1] = end[1]-center[1];
        n[2] = end[2]-center[2];
        double len = sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
        n[0] /= len;
        n[1] /= len;
        n[2] /= len;
    }

	virtual ~Arc(){
		if(center != NULL) free(center);
	};

	double center[3]; //coordinates of the center point
	double radius; // radius of the circle or sphere
    double n[3];
    double end[3];
    double angle;
    int    type; // ref:wikipedia circle
                 // code: 0 arc
                 //       1 sector
                 //       2 segment
                 //       3 3D arc
                 //       4 3D sector
                 //       5 3D segment


private:


};

#endif /* ARC_H_ */

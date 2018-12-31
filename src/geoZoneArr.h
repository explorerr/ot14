/*
 * geoZoneArr.h
 *
 *  Created on: Jul 2, 2010
 *      Author: zrui
 */

#ifndef GEOZONEARR_H_
#define GEOZONEARR_H_


class geoZoneArr {
public:

	__int64 length;

	geoZoneArr(){
		incrementSize = 10;
		length = 0;
		size = 0;
		arr = NULL;
	}

	geoZoneArr(__int64 initSize){
		incrementSize = initSize;
		length = 0;
		size = 0;
		arr = NULL;
	}

	virtual ~geoZoneArr(){
        if(arr != NULL){
        	for(__int64 i=0; i<length; i++)
        		if(arr[i])
        			free(arr[i]);
        	free(arr);
        }
    };

	void setIncreMentSize(__int64 size){
		incrementSize = size;
	}


	/**
	 * This is the assignment operator for this class
	 * This function will not support "out of range" assignment
	 * The index must be within the length of the array
	 */

	__int64 &operator()(__int64 i, __int64 j ){

		if(i < 0 || i >= length){
			printf("\nError in geoZoneArr assignment operator: first subscript %lld out of range %lld", i, length);
			throw StoppingException("\n");
		}
		if(j < 0 || j >= arr[i]->length ){
			printf("\nError in geoZoneArr assignment operator: second subscript %lld out of range %lld", j, arr[i]->length);
			throw StoppingException("\n");
		}

		return (*arr[i])[j];
	}

	__int64 operator()(__int64 i, __int64 j ) const {
		if(i < 0 || i >= length){
			printf("\nError in geoZoneArr get operator: first subscript %lld out of range %lld", i, length);
			throw StoppingException("\n");
		}
		if(j < 0 || j >= arr[i]->length ){
			printf("\nError in geoZoneArr get operator: second subscript %lld out of range %lld", j, arr[i]->length);
			throw StoppingException("\n");
		}
		return (*arr[i])[j];
	}
	const geoZoneArr &operator=(const geoZoneArr &right){
		if(&right != this){
			if(size != right.size){
		        if(arr != NULL){
		        	for(__int64 i=0; i<length; i++)
		        		if(arr[i])
		        			free(arr[i]);
		        	free(arr);
		        }
				length =right.size;
				tryResize();
			}
			for(__int64 i=0; i<size; i++)
				for(__int64 j=0; j<arr[i]->length; j++)
					arr[i][j] =right.arr[i][j];
		}
		return *this;
	}

	void addZone(){
		length++;
        tryResize();
	}
	void addFace2Zone(__int64 zone, __int64 face){
		if(zone < 0 || zone >= length){
			printf("\nError in geoZoneArr addFace2Zone: Subscript of zone %lld out of range %lld.", zone, length);
			throw StoppingException("\n");
		}
		arr[zone]->add(face);

	}

	void add(__int64 zone, __int64 face){
		if(zone < 0 || zone >= CAP){
			printf("\nError in geoZoneArr get operator: first subscript %lld out of range %lld", zone, length);
			throw StoppingException("\n");
		}

		if(zone>=length){
			length++;
			tryResize();
		}
		if(!arr[zone])
			arr[zone] = new intArr();

		arr[zone]->add(face);


	}




	int getZoneFace(__int64 zone, __int64 i){

		if(zone < 0 || zone >= length){
			printf("\nError in geoZoneArr: Subscript of zone %lld out of range %lld.", zone, length);
			throw StoppingException("\n");
		}
		if(i < 0 || i >= arr[zone]->length){
			printf("\nError in geoZoneArr: Subscript of face %lld out of range %lld.", i, arr[zone]->length);
			throw StoppingException("\n");
		}
        return (*arr[zone])[i];
    }

    intArr* getZoneFaceArr(__int64 zone){
		if(zone < 0 || zone >= length){
			printf("\nError in geoZoneArr: Subscript of zone %lld out of range %lld.", zone, length);
			throw StoppingException("\n");
		}

		return arr[zone];

    }


	void print(){

		arr[0]->print();
	}






private:
    __int64 incrementSize;
    __int64 size;
	intArr **arr; // maintain an array of pointers to the faceIds that define
	            // the zone
	            // for each zone, there will be an array of intArr
	static const __int64 CAP =ARRLONGCAP;


	/**
	 * This function will compare the current length of the array and its size
	 *
	 * The current length is supposed to be updated in the calling function
	 * The calling function need to change the length first, and call the tryResize()
	 * before the function tries to change the content of the array
	 *
	 * This function will check if the length is larger than the CAP, if larger will print error message
	 */
	void tryResize() {
		if(length>CAP){
			printf("Error in geoZoneArr: array length larger than allowed capacity: %lld > %lld", length, CAP);
			throw StoppingException("\n");
		}

		while(length >= size){
			size = size + incrementSize;
			try{
				arr = (intArr**) realloc(arr, sizeof(intArr*)*size);
			}
			catch (...){
				throw StoppingException("Error in geoZoneArr: Memory allocation failed in elemArr.\n");
			}

			for(__int64 i=size-incrementSize; i<size; i++)
				arr[i] = NULL;
		}
	}

};

#endif /* GEOZONEARR_H_ */

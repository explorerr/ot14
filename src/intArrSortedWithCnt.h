#ifndef INTARRSORTEDWITHCNT_H_
#define INTARRSORTEDWITHCNT_H_
#include <memory>
#include <iostream>
#include "Project.h"
#include "math.h"


typedef struct{

    __int64 d;
    int cnt;

}data;


class intArrSortedWithCnt
{
public:
	__int64 length;
	__int64 incrementSize;

	intArrSortedWithCnt(){
		arr = NULL;
		incrementSize = 10;
		length = 0;
		size = 0;
	}

	intArrSortedWithCnt( __int64 initSize){
		arr = NULL;
		incrementSize = initSize;
		length = 0;
		size = 0;
	};
	virtual ~intArrSortedWithCnt(){
		if(arr != NULL)
			free(arr);
	};

	void setIncreMentSize( __int64 size){
		incrementSize = size;
	};


	 __int64 operator[]( __int64 subscript)const{
		if(subscript <0 || subscript >= length){
			printf("\nError in intArrSorted get operator: subscript %lld out of range %lld", subscript, length);
			throw StoppingException("\n");
		}
		return arr[subscript].d;
	};

     
    int getCnt(__int64 id){
        if(id>length-1){
			printf("Error in pointerArr, shallowDelete, id larger than total number of data in array, [%d]\n", id);
			throw StoppingException("\n");
		}

        return arr[id].cnt;


    }


    int getCntOfData(__int64 data){
		
        float id = find(data);
		double pos;
		double frac = modf(id, &pos);
		if(frac==0){ // already in the array, will return the cnt
			return arr[ (__int64)pos ].cnt;
        }
		else // not in the array, will return -1
			return -1;
    
    }

	const intArrSortedWithCnt &operator=(const intArrSortedWithCnt &right){
		if(&right != this){
			if(size != right.size){
				free( arr );
				length =right.size;
				tryResize();
			}
			for( __int64 i=0; i<size; i++){
				arr[i].d =right.arr[i].d;
                arr[i].cnt = right.arr[i].cnt;
            }
		}
		return *this;
	};

	 __int64 insert( __int64 data){

		if(length==0){
			length ++;
			tryResize(); //make sure there will be enough space
			arr[0].d = data;
            arr[0].cnt ++;
			return 0;
		}
		float id = find(data);
		double pos;
		double frac = modf(id, &pos);
		if(frac==0){ // already in the array, will increment the cnt
            arr[(__int64)pos].cnt++;
			return (__int64)pos;
        }
		if(frac>0)
			pos++;
		else
			pos=0;

		length++;
		tryResize();
		for( __int64 i=length-1; i>pos; i--){
			arr[i].d = arr[i-1].d;
            arr[i].cnt = arr[i-1].cnt;
        }
		arr[(__int64)pos].d = data;
        arr[(__int64)pos].cnt=1;
		return (__int64)(pos);
		
	}

	float find( __int64 data){
	
		if(length==0)
			return -1;
		 __int64 i = length/2;
		float find;
		if( data > arr[length-1].d )
			return length-0.5;
		else if ( data < arr[0].d )
			return -0.5;
		if( data > arr[i].d ){
		    return find = findHelper(i, length, data);
		}
		else if( data < arr[i].d ) {
			return find = findHelper(0, i, data);
		}
		else {
			return i;
		}
	}

    bool has( __int64 data){

        float f = find(data);
        if(f==-1)
        	return false;

		double intPart, fraPart;

		fraPart = modf (f , &intPart);

        if ( fabs(fraPart) < 0.001 ) // in the array
            return true;
        else 
            return false;

    }

    void clear(){
    	length = 0;
    	size = 0;
    	free(arr);
    	arr= NULL;

    }


	void shallowDelete(int id){
		if(id>length-1){
			printf("Error in pointerArr, shallowDelete, id larger than total number of data in array, [%d]\n", id);
			throw StoppingException("\n");
		}
		length --;
		for(int i=id; i<length; i++){
			arr[i].d = arr[i+1].d;
            arr[i].cnt = arr[i+1].cnt;
		}
	}

	void print(){
		for(int i=0; i<length; i++){
			cout<<"["<<arr[i].d<<", "<<arr[i].cnt<<"]"<<endl;
		}

	}

	data* arr;


private:
	__int64 size;
	static const  __int64 CAP =ARRLONGCAP;


	/**
	 * This function will compare the current length of the array and its size
	 *
	 * The current length is supposed to be updated in the calling function
	 * The calling function need to change the length first, and call the tryResize()
	 * before the function tries to change the content of the array
	 *
	 * This function will check if the length is larger than the CAP, if larger will pr __int64 error message
	 */
	void tryResize() {
//		if(length>CAP){
//			printf("Error in intArr: array length larger than allowed capacity: %lld > %lld", length, CAP);
//			throw StoppingException("\n");
//		}
		while(length >= size){
			size = size + incrementSize;
			arr = (data*) realloc(arr, sizeof(data)*size);
//			cout<<"allocating: ["<<arr<<"]"<<endl;
			if(!arr)
				throw StoppingException("Error in intArr: memory allocation failed in intArr.\n");
			for( __int64 i=size-incrementSize; i<size; i++){
				arr[i].d = -1;
                arr[i].cnt=0;
			}
		}
	};


		float findHelper( __int64 low,  __int64 high,  __int64 data){

		if ( low >= high )
			return (low+0.5);
		if( high-low == 1){
			if( data == arr[low].d )
				return low;
			if( data == arr[high].d )
				return high;
			return low+0.5;
		}

		 __int64 mid = ( low + high ) / 2;
		
		if( data == arr[mid].d )
			return mid;

		if( data > arr[mid].d )
			low = mid;
		else if ( data < arr[mid].d )
			high = mid;
		
		return findHelper(low, high, data);


	};


};

#endif /*INTARRSORTEDWITHCNT_H_*/

#ifndef EXCEPTIONS_
#define EXCEPTIONS_
#include <memory>
#include <exception>
class MyApplicationException{

public:
	  virtual const char* what() const throw()
	  {
	    return myMSG;
	  }

    MyApplicationException( char const *msg){myMSG = msg;};
	virtual ~MyApplicationException(){};
	const char *message()const{ return myMSG;};

protected:
	 char const *myMSG;
};


class ContinuingException : public MyApplicationException{
public:
	ContinuingException( char const *msg):MyApplicationException(msg){};
};

class StoppingException : public MyApplicationException{
public:
	StoppingException( char const *msg):MyApplicationException(msg){};
};

class GridOnModelException : public MyApplicationException{
public:
	GridOnModelException( char const *msg):MyApplicationException(msg){};
};
















/*

     class MathematicalException : public MyApplicationException {

     };
          class DivisionByZeroException : public MathematicalException {};
     class InvalidArgumentException : public MyApplicationException {};

     class MallocException : public MyApplicationException  {};


     class FileException : public MyApplicationException  { } ;
		 class FileOpenException : public FileException { } ;
		 class FileCloseException : public FileException { } ;
		 class FileWriteException : public FileException { } ;

	class InputDataException : public MyApplicationException { } ;
		class InputDataDimensionNotMatchException : public InputDataException { } ;
		class InputDataBodyNotFoundException : public InputDataException { } ;
		class InputDataTypeConversionException : public InputDataException { } ;
		class InputDataInfoSectionException : public InputDataException { } ;
		class InputDataConvertionException : public InputDataException { } ;


	class GeoModelException : public MyApplicationException { } ;
		class GeoModelLinearInfoNotEnoughException : public GeoModelException { } ;
		class GeoModelTriangleInfoNotEnoughException : public GeoModelException { } ;
		class GeoModelQuadInfoNotEnoughException : public GeoModelException { } ;
		class GeoModelPolygoneInfoNotEnoughException : public GeoModelException { } ;
		class GeoModelCircleInfoNotEnoughException : public GeoModelException { } ;
		class GeoModelCircleDiameterNotPositiveException : public GeoModelException { } ;
		class GeoModelSphereInfoNotEnoughException : public GeoModelException { } ;
		class GeoModelSphereDiameterNonPositiveException : public GeoModelException { } ;
		class GeoModelPolygoneVerticesLessThanThreeInPolyEquation : public GeoModelException { } ;
		class GeoModelPolygoneVerticesLessThanTwoInConstruction : public GeoModelException { } ;


*/

#endif /*EXCEPTIONS_*/

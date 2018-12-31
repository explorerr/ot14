#pragma once
#include <vector>
struct Triangle {

	/// <summary>Initializes a new triangle</summary>
	/// <param name="firstVertexIndex">Index of the triangle's first vertex</param>
	/// <param name="secondVertexIndex">Index of the triangle's second vertex</param>
	/// <param name="thirdVertexIndex">Index of the triangle's third vertex</param>
	public: Triangle(int firstVertexIndex, int secondVertexIndex, int thirdVertexIndex) :
		FirstVertexIndex(firstVertexIndex),
			SecondVertexIndex(secondVertexIndex),
			ThirdVertexIndex(thirdVertexIndex) {}

	/// <summary>Index of the triangle's first vertex</summary>
	public: int FirstVertexIndex;
	/// <summary>Index of the triangle's second vertex</summary>
	public: int SecondVertexIndex;
	/// <summary>Index of the triangle's third vertex</summary>
	public: int ThirdVertexIndex;

};

class Tessellator
{
public:
	Tessellator(void);
	~Tessellator(void);
	void Tessellate(std::vector<double> polyOut, std::vector<double> polyIn,  
		            std::vector<Triangle> &Triangles,
					std::vector<double> &vertices);
};


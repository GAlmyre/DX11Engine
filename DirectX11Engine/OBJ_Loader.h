// OBJ_Loader.h - A Single Header OBJ Model Loader

#pragma once
#include "Core/pch.h"
#include "Material.h"
// Iostream - STD I/O Library
#include <iostream>

// Vector - STD Vector/Array Library
#include <vector>

// String - STD String Library
#include <string>

// fStream - STD File I/O Library
#include <fstream>

// Math.h - STD math Library
#include <math.h>

// Print progress to console while loading (large models)
#define OBJL_CONSOLE_OUTPUT

using namespace DirectX;

// Namespace: OBJL
//
// Description: The namespace that holds eveyrthing that
//	is needed and used for the OBJ Model Loader
namespace objl
{
	// Structure: Vertex
	//
	// Description: Model Vertex object that holds
	//	a position, Normal, and Texture Coordinate

	bool operator==(const XMFLOAT3& a, const XMFLOAT3& b) {
		return (a.x == b.x && a.y == b.y && a.z == b.z);
	}

	bool operator!=(const XMFLOAT3& a, const XMFLOAT3& b) {
		return (a.x != b.x || a.y != b.y || a.z != b.z);
	}

	// Structure: Mesh
	//
	// Description: A Simple Mesh Object that holds
	//	a name, a vertex list, and an index list
	struct Mesh
	{
		// Default Constructor
		Mesh()
		{

		}
		// Variable Set Constructor
		Mesh(std::vector<VertexPositionNormalTexture>& _Vertices, std::vector<DWORD>& _Indices)
		{
			Vertices = _Vertices;
			Indices = _Indices;
		}
		// Mesh Name
		std::string MeshName;
		// Vertex List
		std::vector<VertexPositionNormalTexture> Vertices;
		// Index List
		std::vector<DWORD> Indices;

		// Material
		MaterialData MeshMaterial;
	};

	// Namespace: Math
	//
	// Description: The namespace that holds all of the math
	//	functions need for OBJL
	namespace math
	{
		// Vector3 Cross Product
		XMFLOAT3 CrossV3(const XMFLOAT3 a, const XMFLOAT3 b)
		{
			return XMFLOAT3(a.y * b.z - a.z * b.y,
				a.z * b.x - a.x * b.z,
				a.x * b.y - a.y * b.x);
		}

		// Vector3 Magnitude Calculation
		float MagnitudeV3(const XMFLOAT3 in)
		{
			return (sqrtf(powf(in.x, 2) + powf(in.y, 2) + powf(in.z, 2)));
		}

		// Vector3 DotProduct
		float DotV3(const XMFLOAT3 a, const XMFLOAT3 b)
		{
			return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
		}

		// Angle between 2 Vector3 Objects
		float AngleBetweenV3(const XMFLOAT3 a, const XMFLOAT3 b)
		{
			float angle = DotV3(a, b);
			angle /= (MagnitudeV3(a) * MagnitudeV3(b));
			return angle = acosf(angle);
		}

		// Projection Calculation of a onto b
		XMFLOAT3 ProjV3(const XMFLOAT3 a, const XMFLOAT3 b)
		{
			XMFLOAT3 bn = b / MagnitudeV3(b);
			return bn * DotV3(a, bn);
		}
	}

	// Namespace: Algorithm
	//
	// Description: The namespace that holds all of the
	// Algorithms needed for OBJL
	namespace algorithm
	{
		// Vector3 Multiplication Opertor Overload
		XMFLOAT3 operator*(const float& left, const XMFLOAT3& right)
		{
			return XMFLOAT3(right.x * left, right.y * left, right.z * left);
		}

		// A test to see if P1 is on the same side as P2 of a line segment ab
		bool SameSide(XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 a, XMFLOAT3 b)
		{
			XMFLOAT3 cp1 = math::CrossV3(b - a, p1 - a);
			XMFLOAT3 cp2 = math::CrossV3(b - a, p2 - a);

			if (math::DotV3(cp1, cp2) >= 0)
				return true;
			else
				return false;
		}

		// Generate a cross produect normal for a triangle
		XMFLOAT3 GenTriNormal(XMFLOAT3 t1, XMFLOAT3 t2, XMFLOAT3 t3)
		{
			XMFLOAT3 u = t2 - t1;
			XMFLOAT3 v = t3 - t1;

			XMFLOAT3 normal = math::CrossV3(u,v);

			return normal;
		}

		// Check to see if a Vector3 Point is within a 3 Vector3 Triangle
		bool inTriangle(XMFLOAT3 point, XMFLOAT3 tri1, XMFLOAT3 tri2, XMFLOAT3 tri3)
		{
			// Test to see if it is within an infinite prism that the triangle outlines.
			bool within_tri_prisim = SameSide(point, tri1, tri2, tri3) && SameSide(point, tri2, tri1, tri3)
				&& SameSide(point, tri3, tri1, tri2);

			// If it isn't it will never be on the triangle
			if (!within_tri_prisim)
				return false;

			// Calulate Triangle's Normal
			XMFLOAT3 n = GenTriNormal(tri1, tri2, tri3);

			// Project the point onto this normal
			XMFLOAT3 proj = math::ProjV3(point, n);

			// If the distance from the triangle to the point is 0
			//	it lies on the triangle
			if (math::MagnitudeV3(proj) == 0)
				return true;
			else
				return false;
		}

		// Split a String into a string array at a given token
		inline void split(const std::string &in,
			std::vector<std::string> &out,
			std::string token)
		{
			out.clear();

			std::string temp;

			for (int i = 0; i < int(in.size()); i++)
			{
				std::string test = in.substr(i, token.size());

				if (test == token)
				{
					if (!temp.empty())
					{
						out.push_back(temp);
						temp.clear();
						i += (int)token.size() - 1;
					}
					else
					{
						out.push_back("");
					}
				}
				else if (i + token.size() >= in.size())
				{
					temp += in.substr(i, token.size());
					out.push_back(temp);
					break;
				}
				else
				{
					temp += in[i];
				}
			}
		}

		// Get tail of string after first token and possibly following spaces
		inline std::string tail(const std::string &in)
		{
			size_t token_start = in.find_first_not_of(" \t");
			size_t space_start = in.find_first_of(" \t", token_start);
			size_t tail_start = in.find_first_not_of(" \t", space_start);
			size_t tail_end = in.find_last_not_of(" \t");
			if (tail_start != std::string::npos && tail_end != std::string::npos)
			{
				return in.substr(tail_start, tail_end - tail_start + 1);
			}
			else if (tail_start != std::string::npos)
			{
				return in.substr(tail_start);
			}
			return "";
		}

		// Get first token of string
		inline std::string firstToken(const std::string &in)
		{
			if (!in.empty())
			{
				size_t token_start = in.find_first_not_of(" \t");
				size_t token_end = in.find_first_of(" \t", token_start);
				if (token_start != std::string::npos && token_end != std::string::npos)
				{
					return in.substr(token_start, token_end - token_start);
				}
				else if (token_start != std::string::npos)
				{
					return in.substr(token_start);
				}
			}
			return "";
		}

		// Get element at given index position
		template <class T>
		inline const T & getElement(const std::vector<T> &elements, std::string &index)
		{
			int idx = std::stoi(index);
			if (idx < 0)
				idx = int(elements.size()) + idx;
			else
				idx--;
			return elements[idx];
		}
	}

	// Class: Loader
	//
	// Description: The OBJ Model Loader
	class Loader
	{
	public:
		// Default Constructor
		Loader()
		{

		}
		~Loader()
		{
			LoadedMeshes.clear();
		}

		// Load a file into the loader
		//
		// If file is loaded return true
		//
		// If the file is unable to be found
		// or unable to be loaded return false
		bool LoadFile(std::string Path)
		{
			// If the file is not an .obj file return false
			if (Path.substr(Path.size() - 4, 4) != ".obj")
				return false;


			std::ifstream file(Path);

			if (!file.is_open())
				return false;

			LoadedMeshes.clear();
			LoadedVertices.clear();
			LoadedIndices.clear();

			std::vector<XMFLOAT3> Positions;
			std::vector<XMFLOAT2> TCoords;
			std::vector<XMFLOAT3> Normals;

			std::vector<VertexPositionNormalTexture> Vertices;
			std::vector<DWORD> Indices;

			std::vector<std::string> MeshMatNames;

			bool listening = false;
			std::string meshname;

			Mesh tempMesh;

			#ifdef OBJL_CONSOLE_OUTPUT
			const unsigned int outputEveryNth = 1000;
			unsigned int outputIndicator = outputEveryNth;
			#endif

			std::string curline;
			while (std::getline(file, curline))
			{
				#ifdef OBJL_CONSOLE_OUTPUT
				if ((outputIndicator = ((outputIndicator + 1) % outputEveryNth)) == 1)
				{
					if (!meshname.empty())
					{
						std::cout
							<< "\r- " << meshname
							<< "\t| vertices > " << Positions.size()
							<< "\t| texcoords > " << TCoords.size()
							<< "\t| normals > " << Normals.size()
							<< "\t| triangles > " << (Vertices.size() / 3)
							<< (!MeshMatNames.empty() ? "\t| material: " + MeshMatNames.back() : "");
					}
				}
				#endif

				// Generate a Mesh Object or Prepare for an object to be created
				if (algorithm::firstToken(curline) == "o" || algorithm::firstToken(curline) == "g" || curline[0] == 'g')
				{
					if (!listening)
					{
						listening = true;

						if (algorithm::firstToken(curline) == "o" || algorithm::firstToken(curline) == "g")
						{
							meshname = algorithm::tail(curline);
						}
						else
						{
							meshname = "unnamed";
						}
					}
					else
					{
						// Generate the mesh to put into the array

						if (!Indices.empty() && !Vertices.empty())
						{
							// Create Mesh
							tempMesh = Mesh(Vertices, Indices);
							tempMesh.MeshName = meshname;

							// Insert Mesh
							LoadedMeshes.push_back(tempMesh);

							// Cleanup
							Vertices.clear();
							Indices.clear();
							meshname.clear();

							meshname = algorithm::tail(curline);
						}
						else
						{
							if (algorithm::firstToken(curline) == "o" || algorithm::firstToken(curline) == "g")
							{
								meshname = algorithm::tail(curline);
							}
							else
							{
								meshname = "unnamed";
							}
						}
					}
					#ifdef OBJL_CONSOLE_OUTPUT
					std::cout << std::endl;
					outputIndicator = 0;
					#endif
				}
				// Generate a Vertex position
				if (algorithm::firstToken(curline) == "v")
				{
					std::vector<std::string> spos;
					XMFLOAT3 vpos;
					algorithm::split(algorithm::tail(curline), spos, " ");

					vpos.x = std::stof(spos[0]);
					vpos.y = std::stof(spos[1]);
					vpos.z = std::stof(spos[2]);

					Positions.push_back(vpos);
				}
				// Generate a Vertex Texture Coordinate
				if (algorithm::firstToken(curline) == "vt")
				{
					std::vector<std::string> stex;
					XMFLOAT2 vtex;
					algorithm::split(algorithm::tail(curline), stex, " ");

					vtex.x = std::stof(stex[0]);
					vtex.y = std::stof(stex[1]);

					TCoords.push_back(vtex);
				}
				// Generate a Vertex Normal;
				if (algorithm::firstToken(curline) == "vn")
				{
					std::vector<std::string> snor;
					XMFLOAT3 vnor;
					algorithm::split(algorithm::tail(curline), snor, " ");

					vnor.x = std::stof(snor[0]);
					vnor.y = std::stof(snor[1]);
					vnor.z = std::stof(snor[2]);

					Normals.push_back(vnor);
				}
				// Generate a Face (vertices & indices)
				if (algorithm::firstToken(curline) == "f")
				{
					// Generate the vertices
					std::vector<VertexPositionNormalTexture> vVerts;
					GenVerticesFromRawOBJ(vVerts, Positions, TCoords, Normals, curline);

					// Add Vertices
					for (int i = 0; i < int(vVerts.size()); i++)
					{
						Vertices.push_back(vVerts[i]);

						LoadedVertices.push_back(vVerts[i]);
					}

					std::vector<unsigned int> iIndices;

					VertexTriangluation(iIndices, vVerts);

					// Add Indices
					for (int i = 0; i < int(iIndices.size()); i++)
					{
						unsigned int indnum = (unsigned int)((Vertices.size()) - vVerts.size()) + iIndices[i];
						Indices.push_back(indnum);

						indnum = (unsigned int)((LoadedVertices.size()) - vVerts.size()) + iIndices[i];
						LoadedIndices.push_back(indnum);

					}
				}
				// Get Mesh Material Name
				if (algorithm::firstToken(curline) == "usemtl")
				{
					MeshMatNames.push_back(algorithm::tail(curline));

					// Create new Mesh, if Material changes within a group
					if (!Indices.empty() && !Vertices.empty())
					{
						// Create Mesh
						tempMesh = Mesh(Vertices, Indices);
						tempMesh.MeshName = meshname;
						int i = 2;
						while(1) {
							tempMesh.MeshName = meshname + "_" + std::to_string(i);

							for (auto &m : LoadedMeshes)
								if (m.MeshName == tempMesh.MeshName)
									continue;
							break;
						}

						// Insert Mesh
						LoadedMeshes.push_back(tempMesh);

						// Cleanup
						Vertices.clear();
						Indices.clear();
					}

					#ifdef OBJL_CONSOLE_OUTPUT
					outputIndicator = 0;
					#endif
				}
				// Load Materials
				if (algorithm::firstToken(curline) == "mtllib")
				{
					// Generate LoadedMaterial

					// Generate a path to the material file
					std::vector<std::string> temp;
					algorithm::split(Path, temp, "/");

					std::string pathtomat = "";

					if (temp.size() != 1)
					{
						for (int i = 0; i < temp.size() - 1; i++)
						{
							pathtomat += temp[i] + "/";
						}
					}


					pathtomat += algorithm::tail(curline);

					#ifdef OBJL_CONSOLE_OUTPUT
					std::cout << std::endl << "- find materials in: " << pathtomat << std::endl;
					#endif

					// Load Materials
					LoadMaterials(pathtomat);
				}
			}

			#ifdef OBJL_CONSOLE_OUTPUT
			std::cout << std::endl;
			#endif

			// Deal with last mesh

			if (!Indices.empty() && !Vertices.empty())
			{
				// Create Mesh
				tempMesh = Mesh(Vertices, Indices);
				tempMesh.MeshName = meshname;

				// Insert Mesh
				LoadedMeshes.push_back(tempMesh);
			}

			file.close();

			// Set Materials for each Mesh
			for (int i = 0; i < MeshMatNames.size(); i++)
			{
				std::string matname = MeshMatNames[i];

				// Find corresponding material name in loaded materials
				// when found copy material variables into mesh material
				for (int j = 0; j < LoadedMaterials.size(); j++)
				{
					if (LoadedMaterials[j].Name == matname)
					{
						LoadedMeshes[i].MeshMaterial = LoadedMaterials[j];
						break;
					}
				}
			}

			if (LoadedMeshes.empty() && LoadedVertices.empty() && LoadedIndices.empty())
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		// Loaded Mesh Objects
		std::vector<Mesh> LoadedMeshes;
		// Loaded Vertex Objects
		std::vector<VertexPositionNormalTexture> LoadedVertices;
		// Loaded Index Positions
		std::vector<unsigned int> LoadedIndices;
		// Loaded Material Objects
		std::vector<MaterialData> LoadedMaterials;

	private:
		// Generate vertices from a list of positions, 
		//	tcoords, normals and a face line
		void GenVerticesFromRawOBJ(std::vector<VertexPositionNormalTexture>& oVerts,
			const std::vector<XMFLOAT3>& iPositions,
			const std::vector<XMFLOAT2>& iTCoords,
			const std::vector<XMFLOAT3>& iNormals,
			std::string icurline)
		{
			std::vector<std::string> sface, svert;
			VertexPositionNormalTexture vVert;
			algorithm::split(algorithm::tail(icurline), sface, " ");

			bool noNormal = false;

			// For every given vertex do this
			for (int i = 0; i < int(sface.size()); i++)
			{
				// See What type the vertex is.
				int vtype;

				algorithm::split(sface[i], svert, "/");

				// Check for just position - v1
				if (svert.size() == 1)
				{
					// Only position
					vtype = 1;
				}

				// Check for position & texture - v1/vt1
				if (svert.size() == 2)
				{
					// position & Texture
					vtype = 2;
				}

				// Check for position, Texture and Normal - v1/vt1/vn1
				// or if position and Normal - v1//vn1
				if (svert.size() == 3)
				{
					if (svert[1] != "")
					{
						// position, Texture, and Normal
						vtype = 4;
					}
					else
					{
						// position & Normal
						vtype = 3;
					}
				}

				// Calculate and store the vertex
				switch (vtype)
				{
				case 1: // P
				{
					vVert.position = algorithm::getElement(iPositions, svert[0]);
					vVert.textureCoordinate = XMFLOAT2(0, 0);
					noNormal = true;
					oVerts.push_back(vVert);
					break;
				}
				case 2: // P/T
				{
					vVert.position = algorithm::getElement(iPositions, svert[0]);
					vVert.textureCoordinate = algorithm::getElement(iTCoords, svert[1]);
					noNormal = true;
					oVerts.push_back(vVert);
					break;
				}
				case 3: // P//N
				{
					vVert.position = algorithm::getElement(iPositions, svert[0]);
					vVert.textureCoordinate = XMFLOAT2(0, 0);
					vVert.normal = algorithm::getElement(iNormals, svert[2]);
					oVerts.push_back(vVert);
					break;
				}
				case 4: // P/T/N
				{
					vVert.position = algorithm::getElement(iPositions, svert[0]);
					vVert.textureCoordinate = algorithm::getElement(iTCoords, svert[1]);
					vVert.normal = algorithm::getElement(iNormals, svert[2]);
					oVerts.push_back(vVert);
					break;
				}
				default:
				{
					break;
				}
				}
			}

			// take care of missing normals
			// these may not be truly acurate but it is the 
			// best they get for not compiling a mesh with normals	
			if (noNormal)
			{
				XMFLOAT3 A = oVerts[0].position - oVerts[1].position;
				XMFLOAT3 B = oVerts[2].position - oVerts[1].position;

				XMFLOAT3 normal = math::CrossV3(A, B);

				for (int i = 0; i < int(oVerts.size()); i++)
				{
					oVerts[i].normal = normal;
				}
			}
		}

		// Triangulate a list of vertices into a face by printing
		//	inducies corresponding with triangles within it
		void VertexTriangluation(std::vector<unsigned int>& oIndices,
			const std::vector<VertexPositionNormalTexture>& iVerts)
		{
			// If there are 2 or less verts,
			// no triangle can be created,
			// so exit
			if (iVerts.size() < 3)
			{
				return;
			}
			// If it is a triangle no need to calculate it
			if (iVerts.size() == 3)
			{
				oIndices.push_back(0);
				oIndices.push_back(1);
				oIndices.push_back(2);
				return;
			}

			// Create a list of vertices
			std::vector<VertexPositionNormalTexture> tVerts = iVerts;

			while (true)
			{
				// For every vertex
				for (int i = 0; i < int(tVerts.size()); i++)
				{
					// pPrev = the previous vertex in the list
					VertexPositionNormalTexture pPrev;
					if (i == 0)
					{
						pPrev = tVerts[tVerts.size() - 1];
					}
					else
					{
						pPrev = tVerts[i - 1];
					}

					// pCur = the current vertex;
					VertexPositionNormalTexture pCur = tVerts[i];

					// pNext = the next vertex in the list
					VertexPositionNormalTexture pNext;
					if (i == tVerts.size() - 1)
					{
						pNext = tVerts[0];
					}
					else
					{
						pNext = tVerts[i + 1];
					}

					// Check to see if there are only 3 verts left
					// if so this is the last triangle
					if (tVerts.size() == 3)
					{
						// Create a triangle from pCur, pPrev, pNext
						for (int j = 0; j < int(tVerts.size()); j++)
						{
							if (iVerts[j].position == pCur.position)
								oIndices.push_back(j);
							if (iVerts[j].position == pPrev.position)
								oIndices.push_back(j);
							if (iVerts[j].position == pNext.position)
								oIndices.push_back(j);
						}

						tVerts.clear();
						break;
					}
					if (tVerts.size() == 4)
					{
						// Create a triangle from pCur, pPrev, pNext
						for (int j = 0; j < int(iVerts.size()); j++)
						{
							if (iVerts[j].position == pCur.position)
								oIndices.push_back(j);
							if (iVerts[j].position == pPrev.position)
								oIndices.push_back(j);
							if (iVerts[j].position == pNext.position)
								oIndices.push_back(j);
						}

						XMFLOAT3 tempVec;
						for (int j = 0; j < int(tVerts.size()); j++)
						{
							if (tVerts[j].position != pCur.position
								&& tVerts[j].position != pPrev.position
								&& tVerts[j].position != pNext.position)
							{
								tempVec = tVerts[j].position;
								break;
							}
						}

						// Create a triangle from pCur, pPrev, pNext
						for (int j = 0; j < int(iVerts.size()); j++)
						{
							if (iVerts[j].position == pPrev.position)
								oIndices.push_back(j);
							if (iVerts[j].position == pNext.position)
								oIndices.push_back(j);
							if (iVerts[j].position == tempVec)
								oIndices.push_back(j);
						}

						tVerts.clear();
						break;
					}

					// If Vertex is not an interior vertex
					float angle = math::AngleBetweenV3(pPrev.position - pCur.position, pNext.position - pCur.position) * (180 / 3.14159265359);
					if (angle <= 0 && angle >= 180)
						continue;

					// If any vertices are within this triangle
					bool inTri = false;
					for (int j = 0; j < int(iVerts.size()); j++)
					{
						if (algorithm::inTriangle(iVerts[j].position, pPrev.position, pCur.position, pNext.position)
							&& iVerts[j].position != pPrev.position
							&& iVerts[j].position != pCur.position
							&& iVerts[j].position != pNext.position)
						{
							inTri = true;
							break;
						}
					}
					if (inTri)
						continue;

					// Create a triangle from pCur, pPrev, pNext
					for (int j = 0; j < int(iVerts.size()); j++)
					{
						if (iVerts[j].position == pCur.position)
							oIndices.push_back(j);
						if (iVerts[j].position == pPrev.position)
							oIndices.push_back(j);
						if (iVerts[j].position == pNext.position)
							oIndices.push_back(j);
					}

					// Delete pCur from the list
					for (int j = 0; j < int(tVerts.size()); j++)
					{
						if (tVerts[j].position == pCur.position)
						{
							tVerts.erase(tVerts.begin() + j);
							break;
						}
					}

					// reset i to the start
					// -1 since loop will add 1 to it
					i = -1;
				}

				// if no triangles were created
				if (oIndices.size() == 0)
					break;

				// if no more vertices
				if (tVerts.size() == 0)
					break;
			}
		}

		// Load Materials from .mtl file
		bool LoadMaterials(std::string path)
		{
			// If the file is not a material file return false
			if (path.substr(path.size() - 4, path.size()) != ".mtl")
				return false;

			std::ifstream file(path);

			// If the file is not found return false
			if (!file.is_open())
				return false;

			MaterialData tempMaterial;

			bool listening = false;

			// Go through each line looking for material variables
			std::string curline;
			while (std::getline(file, curline))
			{
				// new material and material name
				if (algorithm::firstToken(curline) == "newmtl")
				{
					if (!listening)
					{
						listening = true;

						if (curline.size() > 7)
						{
							tempMaterial.Name = algorithm::tail(curline);
						}
						else
						{
							tempMaterial.Name = "none";
						}
					}
					else
					{
						// Generate the material

						// Push Back loaded Material
						LoadedMaterials.push_back(tempMaterial);

						// Clear Loaded Material
						tempMaterial = MaterialData();

						if (curline.size() > 7)
						{
							tempMaterial.Name = algorithm::tail(curline);
						}
						else
						{
							tempMaterial.Name = "none";
						}
					}
				}
				// Ambient Color
				if (algorithm::firstToken(curline) == "Ka")
				{
					std::vector<std::string> temp;
					algorithm::split(algorithm::tail(curline), temp, " ");

					if (temp.size() != 3)
						continue;

					tempMaterial.Ka.x = std::stof(temp[0]);
					tempMaterial.Ka.y = std::stof(temp[1]);
					tempMaterial.Ka.z = std::stof(temp[2]);
				}
				// Diffuse Color
				if (algorithm::firstToken(curline) == "Kd")
				{
					std::vector<std::string> temp;
					algorithm::split(algorithm::tail(curline), temp, " ");

					if (temp.size() != 3)
						continue;

					tempMaterial.Kd.x = std::stof(temp[0]);
					tempMaterial.Kd.y = std::stof(temp[1]);
					tempMaterial.Kd.z = std::stof(temp[2]);
				}
				// Specular Color
				if (algorithm::firstToken(curline) == "Ks")
				{
					std::vector<std::string> temp;
					algorithm::split(algorithm::tail(curline), temp, " ");

					if (temp.size() != 3)
						continue;

					tempMaterial.Ks.x = std::stof(temp[0]);
					tempMaterial.Ks.y = std::stof(temp[1]);
					tempMaterial.Ks.z = std::stof(temp[2]);
				}
				// Specular Exponent
				if (algorithm::firstToken(curline) == "Ns")
				{
					tempMaterial.Ns = std::stof(algorithm::tail(curline));
				}
				// Optical Density
				if (algorithm::firstToken(curline) == "Ni")
				{
					tempMaterial.Ni = std::stof(algorithm::tail(curline));
				}
				// Dissolve
				if (algorithm::firstToken(curline) == "d")
				{
					tempMaterial.D = std::stof(algorithm::tail(curline));
				}
				// Illumination
				if (algorithm::firstToken(curline) == "illum")
				{
					tempMaterial.Illum = std::stoi(algorithm::tail(curline));
				}
				// Ambient Texture Map
				if (algorithm::firstToken(curline) == "map_Ka")
				{
					tempMaterial.Map_Ka = algorithm::tail(curline);
				}
				// Diffuse Texture Map
				if (algorithm::firstToken(curline) == "map_Kd")
				{
					tempMaterial.Map_Kd = algorithm::tail(curline);
				}
				// Specular Texture Map
				if (algorithm::firstToken(curline) == "map_Ks")
				{
					tempMaterial.Map_Ks = algorithm::tail(curline);
				}
				// Specular Hightlight Map
				if (algorithm::firstToken(curline) == "map_Ns")
				{
					tempMaterial.Map_Ns = algorithm::tail(curline);
				}
				// Alpha Texture Map
				if (algorithm::firstToken(curline) == "map_d")
				{
					tempMaterial.Map_d = algorithm::tail(curline);
				}
				// Bump Map
				if (algorithm::firstToken(curline) == "map_Bump" || algorithm::firstToken(curline) == "map_bump" || algorithm::firstToken(curline) == "bump")
				{
					tempMaterial.Map_bump = algorithm::tail(curline);
				}
			}

			// Deal with last material

			// Push Back loaded Material
			LoadedMaterials.push_back(tempMaterial);

			// Test to see if anything was loaded
			// If not return false
			if (LoadedMaterials.empty())
				return false;
			// If so return true
			else
				return true;
		}
	};
}

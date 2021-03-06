
//------------------------------------------------------------------------------
// Copyright 2017 Corey Toler-Franklin, University of Florida
// MySphere.cpp
// Sphere Object
//------------------------------------------------------------------------------



#include "MySphere.h"
//-----------------------------------------------
// ConStructor
//-----------------------------------------------
MySphere::MySphere()
    : m_globalCount (0),
      m_levels      (3)
{
    // set the output fileneame
    m_pFileName = "../../data/meshes/mysphere.obj";
}



//-----------------------------------------------
// Destructor
//-----------------------------------------------
MySphere::~MySphere()
{
    ClearMesh();
}


char * MySphere::FileName(void)
{
    return(m_pFileName);
}


//-------------------------------------------------
// Initializes the TriangleIndices with vertices a, b and c
//-------------------------------------------------
TriangleIndices MySphere::MakeTIndices(int a, int b, int c)
{
    TriangleIndices T;
    T.i1 = a;
    T.i2 = b;
    T.i3 = c;
    return(T);
}



//-----------------------------------------------
// TO DO: Complete this function.
// Offset each point to the sphere surface
// input - vertices
// output - updated  vertices
//-----------------------------------------------
int MySphere::Offset(STVector3 p, std::vector<STVector3>* vertices)
{

    //-----------------------------------------------
    // TO DO: Compute and offset along the normal to the point
    //-----------------------------------------------
    // 
    //------------------------------------------------
	float pos = (1.0 + sqrtf(5.0)) / 2.0;
	float radius = sqrtf(1 + pos * pos);
	p.Normalize();
    vertices->push_back(STVector3(p.x * radius , p.y * radius , p.z  * radius));
    return m_globalCount++;
}



//-----------------------------------------------------------------------
// TO DO:Complete this function. Computes the midpoint along a face edge.
//                               It returns indices of the smallest point.
//                               The vertex list must also be updated with this midpoint
//                               Duplicates should be removed.
//
// input - p1 and p2 are indices into the vertex list for the current face edge
// output -   midPointIndices, an std::map that stores a key to the new index       
//            the vertices are also updated by a call to the Offset function
//-----------------------------------------------------------------------
int MySphere::MidPoint(int p1, int p2, std::multimap<long, int> *midPointIndices, std::vector<STVector3> *vertices)
{

        int index = 0;

        //-----------------------------------------------------------------------
        // Find the middle point at each edge defined by p1 and p2. Remove duplicates.
        // Call the offset() function to offset the point from the icocahedron to the
        // sphere surface        
        //---------------------------------------------------------------------------
        //
        //
        //--------------------------------------------------------------------------

        //check duplicates
        long smallerIndice = p1 < p2 ? p1 : p2;
        long largerIndice = p1 > p2 ? p1 : p2;
        long key = (smallerIndice << 16) + largerIndice;

        std::multimap<long, int>::iterator search = midPointIndices->find(key);

        if(search == midPointIndices->end()){ //no duplicate
            
            STVector3 point1 = (*vertices)[p1];
            STVector3 point2 = (*vertices)[p2];
            STVector3 midPoint = STVector3((point1.x + point2.x) / 2.0, (point1.y + point2.y) / 2.0, (point1.z + point2.z) / 2.0);

            index = Offset(midPoint, vertices);
            midPointIndices->insert(std::pair<long, int>(key, index));
        }else{
            index = search->second;
        }


        return(index);
}


//----------------------------------------------------------
// TO DO: Store your mesh data in the triangleMesh
//-----------------------------------------------------------
void MySphere::GenerateMesh(STTriangleMesh  *tmesh, std::vector<TriangleIndices> face, std::vector<STVector3> vertices, int nvert)
{
    //----------------------------------------------------------
    // TO DO: Add vertices and faces to a mesh.
    //-----------------------------------------------------------
    //
    //
    //-----------------------------------------------------------
    for(unsigned int i=0; i < vertices.size(); i++){
        tmesh->AddVertex(vertices[i].x, vertices[i].y, vertices[i].z);
    }
    for(unsigned int i=0; i < face.size();i++){
        tmesh->AddFace(face[i].i1, face[i].i2, face[i].i3);
    }

    m_TriangleMeshes.push_back(tmesh);

}



//---------------------------------------------------------
// TO DO:Split triangles into 4 smaller triangles
//----------------------------------------------------------
void MySphere::SubDivideTriangles(int level, std::vector<TriangleIndices> *facesIn,  std::vector<TriangleIndices> *facesOut, std::vector<STVector3> *vertices)
{
    std::multimap<long, int> midPointIndices;

    for (int i = 0; i < level; i++) {

        int nFaces = facesIn->size();
        for(int j = 0; j < nFaces; ++j) {

            //---------------------------------------------------------
            // TO DO:Split triangles into 4 smaller triangles
            // For each edge of each face, compute the midpoints.
            // Use the MidPoint function to update the vertices.
            // Update the faces in each iteration.
            //---------------------------------------------------------
            TriangleIndices triangleIndices = (*facesIn)[j];
            int verticeA = triangleIndices.i1;
            int verticeB = triangleIndices.i2;
            int verticeC = triangleIndices.i3;

            int a = MidPoint(verticeA, verticeB, &midPointIndices, vertices);
            int b = MidPoint(verticeB, verticeC, &midPointIndices, vertices);
            int c = MidPoint(verticeC, verticeA, &midPointIndices, vertices);

            facesOut->push_back(MakeTIndices(verticeA, a, c));
            facesOut->push_back(MakeTIndices(verticeB, b, a));
            facesOut->push_back(MakeTIndices(verticeC, c, b));
            facesOut->push_back(MakeTIndices(a, b, c));
        }
        (*facesIn) = (*facesOut);
    }
}


//-------------------------------------------------------
// TO DO: Create faces for the Iscohedron
// Each TriangleIndices stores the counter clocksise vertex indices
// See definition of TriangleIndices and use this structure to set face indices
// Store your faces in m_faces
//-------------------------------------------------------
void MySphere::InitFaces(void)
{

    //m_faces.clear();


    m_faces.push_back(MakeTIndices(0, 11, 5));
    m_faces.push_back(MakeTIndices(0, 5, 1));
    m_faces.push_back(MakeTIndices(0, 1, 7));
    m_faces.push_back(MakeTIndices(0, 7, 10));
    m_faces.push_back(MakeTIndices(0, 10, 11));

    m_faces.push_back(MakeTIndices(1, 5, 9));
    m_faces.push_back(MakeTIndices(5, 11, 4));
    m_faces.push_back(MakeTIndices(11, 10, 2));
    m_faces.push_back(MakeTIndices(10, 7, 6));
    m_faces.push_back(MakeTIndices(7, 1, 8));

    m_faces.push_back(MakeTIndices(3, 9,4));
    m_faces.push_back(MakeTIndices(3, 4, 2));
    m_faces.push_back(MakeTIndices(3, 2, 6));
    m_faces.push_back(MakeTIndices(3, 6, 8));
    m_faces.push_back(MakeTIndices(3, 8, 9));

    m_faces.push_back(MakeTIndices(4, 9 ,5));
    m_faces.push_back(MakeTIndices(2, 4, 11));
    m_faces.push_back(MakeTIndices(6, 2, 10));
    m_faces.push_back(MakeTIndices(6, 7 , 8));
    m_faces.push_back(MakeTIndices(8, 1,9));
}


//--------------------------------------------------------------------
// Initialize 12 verticies for an iscohedron
// centered at zero. See icosphere_visual.pdf in the docs/folder
//---------------------------------------------------------------------
void MySphere::InitVertices(void)
{
    m_vertices.clear();

    float pos = (1.0 + sqrtf(5.0))/2.0;

    m_vertices.push_back(STVector3(-1,  pos,  0));
    m_vertices.push_back(STVector3(1,   pos,  0));
    m_vertices.push_back(STVector3(-1, -pos,  0));
    m_vertices.push_back(STVector3(1,  -pos,  0));

    m_vertices.push_back(STVector3(0, -1,  pos));
    m_vertices.push_back(STVector3(0,  1,  pos));
    m_vertices.push_back(STVector3(0, -1, -pos));
    m_vertices.push_back(STVector3(0,  1, -pos));

    m_vertices.push_back(STVector3(pos,   0, -1));
    m_vertices.push_back(STVector3(pos,   0,  1));
    m_vertices.push_back(STVector3(-pos,  0, -1));
    m_vertices.push_back(STVector3(-pos,  0,  1));
}



//----------------------------------------------------------------
// TO DO: Complete this function to create your sphere
// The sphere is a unit iscosphere centered at the origin (0,0,0).
//----------------------------------------------------------------
void MySphere::Create(int levels)
{
    // reset the global count
    // increased every time an offset is added
    m_globalCount = 0;



    // Creates the initial verticies
    InitVertices();

    //-----------------------------------------------------
    // TO DO: create the faces of the intIcosahedron
    // Implement this function MySphere::InitFaces
    //-----------------------------------------------------
    InitFaces();
    //-----------------------------------------------------


    //---------------------------------------------------------------
    // TO DO: Recursively split each triangle into four triangles
    // See images in docs/icosahedron/
    // 1. Make space to store your new faces
    // 2. Determine the levels for the recursion by changing the value in m_levels
    // 3. Call and implement MySphere::SubDivideTriangles to subdivide the trangles
    //----------------------------------------------------------------
    //
    //-----------------------------------------------------------------
    std::vector<TriangleIndices> facesOut;
	m_globalCount = m_vertices.size();
    m_levels = levels;
    SubDivideTriangles(m_levels, &m_faces, &facesOut, &m_vertices);

    //-----------------------------------------------------------------
    // TO DO: Once faces are generated:
    //      1. allocate space for a new triangle mesh in m_TriangleMeshes
    //      2. call GenerateMesh to create the traingle mesh for your sphere
    //-----------------------------------------------------------------
    //
    GenerateMesh(new STTriangleMesh(), m_faces, m_vertices, m_vertices.size());
    //-----------------------------------------------------------------

    // save the file
    Save(m_pFileName);

}



//----------------------------------------------------------------
// Saves the mesh stored in m_pTriangleMeshes in the file filename
//----------------------------------------------------------------
void MySphere::Save(char *filename)
 {
   // save the result sphere
    for(int id=0; id < (int)m_TriangleMeshes.size(); id++)
        m_TriangleMeshes[id]->Write(filename);
 }



//----------------------------------------------------------------
// Return the triangle mesh list
//----------------------------------------------------------------
std::vector<STTriangleMesh *> MySphere::GetTriangleMesh(void)
{
    return(m_TriangleMeshes);
}



//----------------------------------------------------------------
// Return the triangle mesh at the index id
//----------------------------------------------------------------
STTriangleMesh * MySphere::GetTriangleMesh(int id)
{
    return(m_TriangleMeshes[id]);
}



//----------------------------------------------------------------
// TO DO: Clear the mesh - Cleanup
//----------------------------------------------------------------
void MySphere::ClearMesh(void)
{
    
    for(unsigned int i = 0; i < m_TriangleMeshes.size(); i++){
        delete m_TriangleMeshes[i];
    }

}
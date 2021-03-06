//---------------------------------------------------------------
// main.cpp
// Main program for OpenGL Projects
// Corey Toler-Franklin University of Florida
// Addapted from Tiny Object Loader
//---------------------------------------------------------------

//-------------------------------------------------------
// For this project, we use OpenGL, FreeGLUT
// and GLEW (to load OpenGL extensions)
//-------------------------------------------------------
#include "stglew.h"
#include <stdio.h>
#include <string.h>
#include <map>
#include <queue>
#include "MySphere.h"

//--------------------------------------------------
// Globals used by this application.
// As a rule, globals are Evil, but this is a small application
// and the design of FreeGLUT makes it hard to avoid them.
//------------------------------------------------


// Window size, kept for screenshots
static int gWindowSizeX = 0;
static int gWindowSizeY = 0;

// File locations
std::string vertexShader;
std::string fragmentShader;
std::string normalMap;
std::string displacementMap;
std::string meshOBJ;

// Light source attributes
static float ambientLight[]  = {0.9, 0.9, 0.9, 1.0};
static float diffuseLight[]  = {1.00, 1.00, 1.00, 1.0};
static float specularLight[] = {0.2, 0.2, 0.2, 1.0};
float lightPosition[] = {-10.0f, -15.0f, 0.0f, 1.0f};


// textures
STImage   *surfaceNormImg;
STTexture *surfaceNormTex;
STImage   *surfaceDisplaceImg;
STTexture *surfaceDisplaceTex;

// shaders
STShaderProgram *shader;


// camera params
STVector3 mPosition;
STVector3 mLookAt;
STVector3 mRight;
STVector3 mUp;

// Stored mouse position for camera rotation, panning, and zoom.
int gPreviousMouseX = -1;
int gPreviousMouseY = -1;
int gMouseButton = -1;



// texture modes
typedef enum {
    Color               = 0,
    NormalMapping       = 1,
    DisplacementMapping = 2
}TextureType;



// mesh types
typedef enum {
    Mesh                = 0,
    Axis                = 1
}MeshType;

// meshes and mesh states
bool smooth = true; // smooth/flat shading for mesh
TextureType textureType = TextureType::Color; // default is color texture
std::queue<TextureType> textureQueue;
bool proxyType=true; // use sphere mapping
int globallevels = 3;
std::vector<STTriangleMesh*> gTriangleMeshes;
STPoint3 gMassCenter;
std::pair<STPoint3,STPoint3> gBoundingBox;
STTriangleMesh* gCoordAxisTriangleMesh = NULL;
int TesselationDepth = 100;
MeshType meshType = MeshType::Mesh; // mesh type
std::queue<MeshType> meshQueue;



//-----------------------------------------------
// Clean up
//-----------------------------------------------
void ClearGlobalMesh()
{
    // remove the mesh
    for(int id=0; id < (int)gTriangleMeshes.size();id++)
        delete gTriangleMeshes[id];
    if(gCoordAxisTriangleMesh != NULL)
        delete gCoordAxisTriangleMesh;
}



void SetUpAndRight()
{
    mRight = STVector3::Cross(mLookAt - mPosition, mUp);
    mRight.Normalize();
    mUp = STVector3::Cross(mRight, mLookAt - mPosition);
    mUp.Normalize();
}

void resetCamera()
{
    mLookAt=STVector3(0.f,0.f,0.f);
    mPosition=STVector3(0.f,5.f,15.f);
    mUp=STVector3(0.f,1.f,0.f);

    SetUpAndRight();
}

void resetUp()
{
    mUp = STVector3(0.f,1.f,0.f);
    mRight = STVector3::Cross(mLookAt - mPosition, mUp);
    mRight.Normalize();
    mUp = STVector3::Cross(mRight, mLookAt - mPosition);
    mUp.Normalize();
}


//-----------------------------------------------
// example - creates a plane and axis in 3D
// mesh creation
//-----------------------------------------------
void CreateYourOwnMesh()
{
    float leftX   = -2.0f;
    float rightX  = -leftX;
    float nearZ   = -2.0f;
    float farZ    = -nearZ;

    gCoordAxisTriangleMesh= new STTriangleMesh();
    for (int i = 0; i < TesselationDepth+1; i++){
        for (int j = 0; j < TesselationDepth+1; j++) {
            float s0 = (float) i / (float) TesselationDepth;
            float x0 =  s0 * (rightX - leftX) + leftX;
            float t0 = (float) j / (float) TesselationDepth;
            float z0 = t0 * (farZ - nearZ) + nearZ;

            gCoordAxisTriangleMesh->AddVertex(x0,(x0*x0+z0*z0)*0.0f,z0,s0,t0);
        }
    }
    for (int i = 0; i < TesselationDepth; i++){
        for (int j = 0; j < TesselationDepth; j++) {
            unsigned int id0=i*(TesselationDepth+1)+j;
            unsigned int id1=(i+1)*(TesselationDepth+1)+j;
            unsigned int id2=(i+1)*(TesselationDepth+1)+j+1;
            unsigned int id3=i*(TesselationDepth+1)+j+1;
            gCoordAxisTriangleMesh->AddFace(id0,id2,id1);
            gCoordAxisTriangleMesh->AddFace(id0,id3,id2);
        }
    }
    gCoordAxisTriangleMesh->Build();
    gCoordAxisTriangleMesh->mMaterialAmbient[0]=0.2f;
    gCoordAxisTriangleMesh->mMaterialAmbient[1]=0.2f;
    gCoordAxisTriangleMesh->mMaterialAmbient[2]=0.6f;
    gCoordAxisTriangleMesh->mMaterialDiffuse[0]=0.2f;
    gCoordAxisTriangleMesh->mMaterialDiffuse[1]=0.2f;
    gCoordAxisTriangleMesh->mMaterialDiffuse[2]=0.6f;
    gCoordAxisTriangleMesh->mMaterialSpecular[0]=0.6f;
    gCoordAxisTriangleMesh->mMaterialSpecular[1]=0.6f;
    gCoordAxisTriangleMesh->mMaterialSpecular[2]=0.6f;
    gCoordAxisTriangleMesh->mShininess=8.0f;
}



//
// Initialize the application, loading all of the settings that
// we will be accessing later in our fragment shaders.
//
void Setup()
{
    // Set up lighting variables in OpenGL
    // Once we do this, we will be able to access them as built-in
    // attributes in the shader (see examples of this in normalmap.frag)
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_SPECULAR,  specularLight);
    glLightfv(GL_LIGHT0, GL_AMBIENT,   ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,   diffuseLight);

    surfaceNormImg = new STImage(normalMap);
    surfaceNormTex = new STTexture(surfaceNormImg);

    surfaceDisplaceImg = new STImage(displacementMap);
    surfaceDisplaceTex = new STTexture(surfaceDisplaceImg);

    shader = new STShaderProgram();
    shader->LoadVertexShader(vertexShader);
    shader->LoadFragmentShader(fragmentShader);

    resetCamera();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    // textures
    textureQueue.push(TextureType::NormalMapping); 
    textureQueue.push(TextureType::DisplacementMapping);
    textureQueue.push(TextureType::Color);


    // load the mesh
    STTriangleMesh::LoadObj(gTriangleMeshes,meshOBJ);

    // set bounding box
    if(gTriangleMeshes.size()) {
        meshType = MeshType::Mesh;
        meshQueue.push(MeshType::Axis);
        meshQueue.push(MeshType::Mesh);
        gMassCenter=STTriangleMesh::GetMassCenter(gTriangleMeshes);
        std::cout<<"Mass Center: "<<gMassCenter<<std::endl;
        gBoundingBox=STTriangleMesh::GetBoundingBox(gTriangleMeshes);
        std::cout<<"Bounding Box: "<<gBoundingBox.first<<" - "<<gBoundingBox.second<<std::endl;
    }
    else {
        meshType = MeshType::Axis; // no mesh to draw in this case
        meshQueue.push(MeshType::Mesh);
        meshQueue.push(MeshType::Axis);
    }
    CreateYourOwnMesh();
}





//-------------------------------------
// Camera adjustment methods
//-------------------------------------
void RotateCamera(float delta_x, float delta_y)
{
    float yaw_rate=1.f;
    float pitch_rate=1.f;

    mPosition -= mLookAt;
    STMatrix4 m;
    m.EncodeR(-1*delta_x*yaw_rate, mUp);
    mPosition = m * mPosition;
    m.EncodeR(-1*delta_y*pitch_rate, mRight);
    mPosition = m * mPosition;

    mPosition += mLookAt;
}

void ZoomCamera(float delta_y)
{
    STVector3 direction = mLookAt - mPosition;
    float magnitude = direction.Length();
    direction.Normalize();
    float zoom_rate = 0.1f*magnitude < 0.5f ? .1f*magnitude : .5f;
    if(delta_y * zoom_rate + magnitude > 0)
    {
        mPosition += (delta_y * zoom_rate) * direction;
    }
}

void StrafeCamera(float delta_x, float delta_y)
{
    float strafe_rate = 0.05f;

    mPosition -= strafe_rate * delta_x * mRight;
    mLookAt   -= strafe_rate * delta_x * mRight;
    mPosition += strafe_rate * delta_y * mUp;
    mLookAt   += strafe_rate * delta_y * mUp;
}



//----------------------------------------------------------------
// Display the output image from our vertex and fragment shaders
//-----------------------------------------------------------------
void DisplayCallback()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    SetUpAndRight();

    gluLookAt(mPosition.x,mPosition.y,mPosition.z,
              mLookAt.x,mLookAt.y,mLookAt.z,
              mUp.x,mUp.y,mUp.z);

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    // Texture 0: surface normal map
    glActiveTexture(GL_TEXTURE0);
    surfaceNormTex->Bind();

    // Texture 1: surface normal map
    glActiveTexture(GL_TEXTURE1);
    surfaceDisplaceTex->Bind();

    // Bind the textures we've loaded into openGl to
    // the variable names we specify in the fragment
    // shader.
    shader->SetTexture("normalTex", 0);
    shader->SetTexture("displacementTex", 1);
      shader->SetTexture("colorTex", 2);

    // Invoke the shader.  Now OpenGL will call our
    // shader programs on anything we draw.
    shader->Bind();

    if(meshType == MeshType::Mesh)
    {
        shader->SetUniform("normalMapping", -1.0);
        shader->SetUniform("displacementMapping", -1.0);
        shader->SetUniform("colorMapping", 1.0);

        glPushMatrix();
        // Pay attention to scale
        STVector3 size_vector=gBoundingBox.second-gBoundingBox.first;
        float maxSize=(std::max)((std::max)(size_vector.x,size_vector.y),size_vector.z);
        glScalef(3.0f/maxSize,3.0f/maxSize,3.0f/maxSize);
        glTranslatef(-gMassCenter.x,-gMassCenter.y,-gMassCenter.z);
        for(int id=0; id < (int)gTriangleMeshes.size(); id++) {
                   gTriangleMeshes[id]->Draw(smooth);
        }
        glPopMatrix();
    }
    else if(meshType == MeshType::Axis)
    {
        // set up shader textures
        if(textureType == TextureType::Color) {
            shader->SetUniform("displacementMapping", -1.0);
            shader->SetUniform("normalMapping", -1.0);
            shader->SetUniform("colorMapping", 1.0);
        }
        else if(textureType == TextureType::NormalMapping){
            shader->SetUniform("displacementMapping", -1.0);
            shader->SetUniform("normalMapping", 1.0);
            shader->SetUniform("colorMapping", -1.0);
        }
        else if(textureType == TextureType::DisplacementMapping){
            shader->SetUniform("displacementMapping", 1.0);
            shader->SetUniform("normalMapping", -1.0);
            shader->SetUniform("colorMapping", -1.0);
            shader->SetUniform("TesselationDepth", TesselationDepth);
        }

        // draw the geometry here
        if(gCoordAxisTriangleMesh)
            gCoordAxisTriangleMesh->Draw(smooth);
        if(gTriangleMeshes.size()) {
            for(unsigned int id=0;id<gTriangleMeshes.size();id++) {
                gTriangleMeshes[id]->Draw(smooth);
            }
        }
    }



    // must bind the shader
    shader->UnBind();

    // set textures
    glActiveTexture(GL_TEXTURE0);
    surfaceNormTex->UnBind();

    // set textures
    glActiveTexture(GL_TEXTURE1);
    surfaceDisplaceTex->UnBind();

    // swap buffers
    glutSwapBuffers();
}



//--------------------------------------------------------------
// Reshape the window and record the size so
// that we can use it for screenshots.
//-------------------------------------------------------------
void ReshapeCallback(int w, int h)
{
    gWindowSizeX = w;
    gWindowSizeY = h;

    glViewport(0, 0, gWindowSizeX, gWindowSizeY);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    // Set up a perspective projection
    float aspectRatio = (float) gWindowSizeX / (float) gWindowSizeY;
    gluPerspective(30.0f, aspectRatio, .1f, 10000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



//-------------------------------------------------
// keyboard callback - special keys
//-------------------------------------------------
void SpecialKeyCallback(int key, int x, int y)
{
    switch(key) {
        case GLUT_KEY_LEFT:
            StrafeCamera(10,0);
            break;
        case GLUT_KEY_RIGHT:
            StrafeCamera(-10,0);
            break;
        case GLUT_KEY_DOWN:
            StrafeCamera(0,-10);
            break;
        case GLUT_KEY_UP:
            StrafeCamera(0,10);
            break;
        default:
            break;
    }
    glutPostRedisplay();
}



//-------------------------------------------------
//  Mouse event handler
//-------------------------------------------------
void MouseCallback(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON
        || button == GLUT_RIGHT_BUTTON
        || button == GLUT_MIDDLE_BUTTON)
    {
        gMouseButton = button;
    } else
    {
        gMouseButton = -1;
    }

    if (state == GLUT_UP)
    {
        gPreviousMouseX = -1;
        gPreviousMouseY = -1;
    }
}



//-------------------------------------------------
// Mouse active motion callback (when button is pressed)
//-------------------------------------------------
void MouseMotionCallback(int x, int y)
{
    if (gPreviousMouseX >= 0 && gPreviousMouseY >= 0)
    {
        //compute delta
        float deltaX = x-gPreviousMouseX;
        float deltaY = y-gPreviousMouseY;
        gPreviousMouseX = x;
        gPreviousMouseY = y;

        //orbit, strafe, or zoom
        if (gMouseButton == GLUT_LEFT_BUTTON)
        {
            RotateCamera(deltaX, deltaY);
        }
        else if (gMouseButton == GLUT_MIDDLE_BUTTON)
        {
            StrafeCamera(deltaX, deltaY);
        }
        else if (gMouseButton == GLUT_RIGHT_BUTTON)
        {
            ZoomCamera(deltaY);
        }

    } else
    {
        gPreviousMouseX = x;
        gPreviousMouseY = y;
    }

}




//-------------------------------------------------
// keyboard callback
//-------------------------------------------------
void KeyCallback(unsigned char key, int x, int y)
{

    switch (key) {

        // create sphere
        case 'c':  {
            MySphere  sphereObject;
            std::cout << "Processing..." << std::endl;
            sphereObject.Create(globallevels); 
            std::cout << "Sphere created!" << std::endl;
            break;
        }


        // toggle the mesh from the createYourOwnMesh on and off
        case 'd':{
            meshType = meshQueue.front();
            meshQueue.push(meshQueue.front());
            meshQueue.pop();
            break;
        }


        // save a screen shot as data/images/screenshot.jpg
        case 's': {
            STImage* screenshot = new STImage(gWindowSizeX, gWindowSizeY);
            screenshot->Read(0, 0);
            screenshot->Save("../../data/images/screenshot.jpg");
            delete screenshot;
            break;
        }

        // reset the camera
        case 'r':
            resetCamera();
            break;

        // reset camera up vector
        case 'u':
            resetUp();
            break;

        // replace the current mesh with the latest sphere object
        case 'm':{
            MySphere sphereObject;
            std::vector<STTriangleMesh*> tempMesh;
            STTriangleMesh::LoadObj(tempMesh,sphereObject.FileName());
            if(tempMesh.size()) {
                gTriangleMeshes = tempMesh;
                gMassCenter=STTriangleMesh::GetMassCenter(gTriangleMeshes);
                gBoundingBox=STTriangleMesh::GetBoundingBox(gTriangleMeshes);
           }
			meshType = MeshType::Mesh;
            break;
        }

        // switch between color, normalMapping and displacementMapping
        case 'n': {
            textureType = textureQueue.front();
            textureQueue.push(textureQueue.front());
            textureQueue.pop();
            break;
        }
        
        // loop subdivide algorithm
        case 'l':
            if(meshType == MeshType::Mesh) {
                gTriangleMeshes[0]->LoopSubdivide();
            }
            break;

        // texturemapping using a spherical proxy
         case 't':
            gTriangleMeshes[0]->CalculateTextureCoordinatesViaSphericalProxy();
            break;

        // switch between smooth shading and flat shading
        case 'f': 
            smooth = !smooth;
            break;

        // save the triangle mesh
        case 'w':
            for (unsigned int id = 0; id<gTriangleMeshes.size(); id++)
                gTriangleMeshes[id]->Write("output.obj");
            break;

        // set levels
        case 'v':  {
            std::cout << "Enter the number of levels:";
            std::cin >> globallevels;
            break;
        }

        // draw corrdinate axis
        case 'a':
            for (int id = 0; id < (int) gTriangleMeshes.size(); id++)
                gTriangleMeshes[id]->mDrawAxis = !gTriangleMeshes[id]->mDrawAxis;
            if (gCoordAxisTriangleMesh != 0)
                gCoordAxisTriangleMesh->mDrawAxis = !gCoordAxisTriangleMesh->mDrawAxis;
            break;

        // quit
        case 'q':
            exit(0);

        default:
            break;
    }

    // redraw scene
    glutPostRedisplay();
}




//-------------------------------------------------
// main program loop
//-------------------------------------------------
int main(int argc, char** argv)
{

    // initialize the mesh file to load
    std::string meshpath = std::string("../../data/meshes/");
    std::string meshfilename;
    std::cout << "Enter OBJ File Name(otherwise press enter none):";
    std::cin >> meshfilename;
    meshOBJ = meshpath + meshfilename;


    // intialize vertex and fragment shaders
    vertexShader   = std::string("kernels/default.vert");
    fragmentShader = std::string("kernels/phong.frag");
    normalMap      = std::string("../../data/images/world_map.jpeg");
    displacementMap= std::string("../../data/images/world_map.jpeg");


    // initialize free glut
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(20, 20);
    glutInitWindowSize(640, 480);
    glutCreateWindow("proj1_mesh");


    // initialize GLEW.
#ifndef __APPLE__
    glewInit();
    if(!GLEW_VERSION_2_0) {
        printf("Your graphics card or graphics driver does\n"
               "\tnot support OpenGL 2.0, trying ARB extensions\n");

        if(!GLEW_ARB_vertex_shader || !GLEW_ARB_fragment_shader) {
            printf("ARB extensions don't work either.\n");
            printf("\tYou can try updating your graphics drivers.\n"
                   "\tIf that does not work, you will have to find\n");
            printf("\ta machine with a newer graphics card.\n");
            exit(1);
        }
    }
#endif

    // set-up the scene
    Setup();

    // add callback functions
    glutDisplayFunc(DisplayCallback);
    glutReshapeFunc(ReshapeCallback);
    glutSpecialFunc(SpecialKeyCallback);
    glutKeyboardFunc(KeyCallback);
    glutMouseFunc(MouseCallback);
    glutMotionFunc(MouseMotionCallback);
    glutIdleFunc(DisplayCallback);

    // run OpenGL mainloop
    glutMainLoop();

    // clean up
    ClearGlobalMesh();

    return 0;
}

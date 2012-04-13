// Triangle.cpp

#include <GLTools.h> // OpenGL toolkit
#include <GLShaderManager.h> // Shader Manager Class
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>  // Geometry Transform Pipeline
#include <GLFrustum.h>
#include <math3d.h>
#include <GLFrame.h>

//#ifdef __APPLE__
//#include <glut/glut.h>          // OS X version of GLUT
//#else
#define FREEGLUT_STATIC
#include <GL/glut.h>            // Windows FreeGlut equivalent
//#endif

GLBatch	triangleBatch;
GLShaderManager	shaderManager;

GLMatrixStack modelViewMatrix_Stack;
GLMatrixStack projectionMatrix_Stack;
GLGeometryTransform transformPipeline; 
GLFrustum viewFrustum;

GLFrame cameraFrame;

GLBatch floorBatch;
GLTriangleBatch torusBatch;


///////////////////////////////////////////////////
// Screen changes size or is initialized
void ChangeSize(int width, int height)
{
	glViewport(0, 0, width, height);
	//naredi projekcijsko matriko in jo nalo�i na njen stack
	viewFrustum.SetPerspective(25.0f, float(width)/float(height), 1.0f, 100.0f);
	projectionMatrix_Stack.LoadMatrix(viewFrustum.GetProjectionMatrix());
	//nastavi transformation pipeline da uporablja sklada matrik: modelview in projection
	transformPipeline.SetMatrixStacks(modelViewMatrix_Stack, projectionMatrix_Stack);
}

void SpecialKeys(int key, int x, int y)
{
	float linear=0.1f;
	float angular=float(m3dDegToRad(5.0f));

	if(key==GLUT_KEY_UP)
	{
		cameraFrame.MoveForward(linear);
		
	}
	if(key==GLUT_KEY_DOWN)
		cameraFrame.MoveForward(-linear);
	if(key==GLUT_KEY_LEFT)
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);
	if(key==GLUT_KEY_RIGHT)
		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);
}

//void keyPressed (unsigned char key, int x, int y)
//{
//	float linear=0.1f;
//	float angular=float(m3dDegToRad(5.0f));
//
//	if(key=='w')
//		cameraFrame.MoveForward(linear);
//	if(key=='s')
//		cameraFrame.MoveForward(-linear);
//	if(key=='a')
//		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);
//	if(key=='d')
//		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);
//}

///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
{
	shaderManager.InitializeStockShaders();

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//// Load up a triangle
	//GLfloat vVerts[] = { -0.5f, 0.0f, 0.0f, 
	//	                  0.5f, 0.0f, 0.0f,
	//					  0.0f, 0.5f, 0.0f,
	//					 -0.5f, 0.0f, 0.0f,
	//					  0.5f, 0.0f, 0.0f,
	//					  0.0f,-0.5f, 0.0f
	//					};

	//triangleBatch.Begin(GL_TRIANGLES, 6);
	//triangleBatch.CopyVertexData3f(vVerts);
	//triangleBatch.End();

	gltMakeTorus(torusBatch, 0.4f, 0.15f, 30, 30);

	floorBatch.Begin(GL_LINES, 324);
    for(GLfloat x = -20.0; x <= 20.0f; x+= 0.5) {
        floorBatch.Vertex3f(x, -0.55f, 20.0f);
        floorBatch.Vertex3f(x, -0.55f, -20.0f);
        
        floorBatch.Vertex3f(20.0f, -0.55f, x);
        floorBatch.Vertex3f(-20.0f, -0.55f, x);
        }
    floorBatch.End();    
}


// Called to draw scene
void RenderScene(void)
{

	static GLfloat vFloorBarva[]={0.0f, 0.0f, 1.0f, 1.0f};
	static GLfloat vKrofBarva[]={1.0f, 0.0f, 0.0f, 1.0f};
	// pocisti barvo in depth bufferje
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


	/*M3DMatrix44f mRotationMatrix;
	m3dRotationMatrix44(mRotationMatrix, m3dDegToRad(45), 0.0f, 0.0f, 1.0f);

	GLfloat vRed[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	shaderManager.UseStockShader(GLT_SHADER_FLAT,mRotationMatrix, vRed);
	triangleBatch.Draw();*/

	modelViewMatrix_Stack.PushMatrix(); //matrika identiteta se shrani na sklad
	
	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelViewMatrix_Stack.PushMatrix(mCamera);

	M3DVector4f vLightPos={5.0f, 10.0f, 5.0f, 1.0f};
	M3DVector4f vLightEyePos;
	//transformiranje koordinate lu�i v glede na kamero
	m3dTransformVector4(vLightEyePos, vLightPos, mCamera);

	//izri�i tla
	shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vFloorBarva);
	floorBatch.Draw();

	//nari�i krof
	modelViewMatrix_Stack.Translate(0.0f, 0.0f, -4.5f);

	//uporaba ze narejnega point light diffuse shaderja
	shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(),vLightEyePos,vKrofBarva);
	torusBatch.Draw();
	
	modelViewMatrix_Stack.PopMatrix();
	modelViewMatrix_Stack.PopMatrix();

	// Perform the buffer swap to display back buffer
	glutSwapBuffers();

	glutPostRedisplay();
}



///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[])
{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Triangle");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return 1;
		}
	
	glutSpecialFunc(SpecialKeys);
	//glutKeyboardFunc(keyPressed);
	SetupRC();

	glutMainLoop();
	return 0;
}
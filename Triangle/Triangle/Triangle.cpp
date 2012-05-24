// Triangle.cpp

#include <GLTools.h> // OpenGL toolkit
#include <GLShaderManager.h> // Shader Manager Class
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>  // Geometry Transform Pipeline
#include <GLFrustum.h>
#include <math3d.h>
#include <GLFrame.h>
#include <StopWatch.h>

//#ifdef __APPLE__
//#include <glut/glut.h>          // OS X version of GLUT
//#else
#define FREEGLUT_STATIC
#include <GL/glut.h>            // Windows FreeGlut equivalent
//#endif

static GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static GLfloat vBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat vGrey[] =  { 0.5f, 0.5f, 0.5f, 1.0f };

M3DVector4f vLightPos={5.0f, 10.0f, 5.0f, 1.0f};   //pozicija luci

static const GLenum windowBuff[]={GL_BACK_LEFT};
static const GLenum fboBuffs[]={GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};

static GLint mirrorTexWidth=800;
static GLint mirrorTexHeight=800;

GLsizei	 screenWidth;			// Desired window or desktop width
GLsizei  screenHeight;			// Desired window or desktop height

GLboolean bFullScreen;			// Request to run full screen
GLboolean bAnimated;			// Request for continual updates


//GLBatch	triangleBatch;
GLShaderManager	shaderManager;

GLMatrixStack modelViewMatrix_Stack;
GLMatrixStack projectionMatrix_Stack;
GLGeometryTransform transformPipeline; 
GLFrustum viewFrustum;

GLFrame cameraFrame;  //frame za kamero
GLFrame mirrorFrame; //frame za ogledalo

GLBatch floorBatch;
GLTriangleBatch torusBatch;
GLTriangleBatch sphereBatch;
GLBatch mirrorBatch;
GLBatch mirrorBorderBatch;

GLuint	uiTextures[3];
GLuint fboName;
GLuint mirrorTexture;
GLuint depthBufferName;



///////////////////////////////////////////////////
// Screen changes size or is initialized
void ChangeSize(int width, int height)
{
	glViewport(0, 0, width, height);
	//naredi projekcijsko matriko in jo naloži na njen stack
	viewFrustum.SetPerspective(25.0f, float(width)/float(height), 1.0f, 100.0f);
	projectionMatrix_Stack.LoadMatrix(viewFrustum.GetProjectionMatrix());
	//nastavi transformation pipeline da uporablja sklada matrik: modelview in projection
	transformPipeline.SetMatrixStacks(modelViewMatrix_Stack, projectionMatrix_Stack);
	modelViewMatrix_Stack.LoadIdentity();

	screenWidth=width;
	screenHeight=height;
}

void SpecialKeys(int key, int x, int y)
{
	float linear=0.40f;
	float angular=float(m3dDegToRad(2.5f));

	if(key==GLUT_KEY_UP)
		cameraFrame.MoveForward(linear);	

	if(key==GLUT_KEY_DOWN)
		cameraFrame.MoveForward(-linear);

	if(key==GLUT_KEY_LEFT)
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);

	if(key==GLUT_KEY_RIGHT)
		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);

	 
}

bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
	GLbyte *pBits;
	int nWidth, nHeight, nComponents;
	GLenum eFormat;
	
	// Read the texture bits
	pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
	if(pBits == NULL) 
		return false;
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, nWidth, nHeight, 0,
				 eFormat, GL_UNSIGNED_BYTE, pBits);
    free(pBits);

    if(minFilter == GL_LINEAR_MIPMAP_LINEAR || 
       minFilter == GL_LINEAR_MIPMAP_NEAREST ||
       minFilter == GL_NEAREST_MIPMAP_LINEAR ||
       minFilter == GL_NEAREST_MIPMAP_NEAREST)
        glGenerateMipmap(GL_TEXTURE_2D);
            
	return true;
}

void makeMirrorAndBorder()
{
	mirrorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
		mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		mirrorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
		mirrorBatch.Normal3f( 0.0f, 1.0f, 0.0f);
		mirrorBatch.Vertex3f(-1.0f, 0.0f, 0.0f);

		mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		mirrorBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
		mirrorBatch.Normal3f(0.0f, 1.0f, 0.0f);
		mirrorBatch.Vertex3f(1.0f, 0.0f, 0.0f);

		mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		mirrorBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
		mirrorBatch.Normal3f(0.0f, 1.0f, 0.0f);
		mirrorBatch.Vertex3f(1.0f, 2.0f, 0.0f);

		mirrorBatch.Color4f(1.0f, 0.0f, 0.0f, 1.0f);
		mirrorBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
		mirrorBatch.Normal3f( 0.0f, 1.0f, 0.0f);
		mirrorBatch.Vertex3f(-1.0f, 2.0f, 0.0f);
	mirrorBatch.End();

	mirrorBorderBatch.Begin(GL_TRIANGLE_STRIP, 13);
		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 0.1f, 0.01f);

		mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 0.0f, 0.01f);

		mirrorBorderBatch.Normal3f(0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(1.0f, 0.1f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(1.0f, 0.0f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(0.9f, 0.0f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(1.0f, 2.0f, 0.01f);
			
		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(0.9f, 2.0f, 0.01f);
			
		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(1.0f, 1.9f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 2.f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 1.9f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-0.9f, 2.f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-1.0f, 0.0f, 0.01f);

		mirrorBorderBatch.Normal3f( 0.0f, 0.0f, 1.0f);
		mirrorBorderBatch.Vertex3f(-0.9f, 0.0f, 0.01f);
	mirrorBorderBatch.End();
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
	//glEnable(GL_CULL_FACE);  //samo ce so trikotniki pravilno orientirani, je pa performance-dobro


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

	gltMakeTorus(torusBatch, 0.4f, 0.15f, 30, 30); // napolni batch s krofom

	gltMakeSphere(sphereBatch,0.1f,40,20);   //napolni (triangle)batch s kroglo

	// Make the solid ground
	GLfloat texSize = 15.0f;
	floorBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
	floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	floorBatch.Vertex3f(-20.0f, -0.41f, 20.0f);
	
	floorBatch.MultiTexCoord2f(0, texSize, 0.0f);
    floorBatch.Vertex3f(20.0f, -0.41f, 20.0f);
	
	floorBatch.MultiTexCoord2f(0, texSize, texSize);
	floorBatch.Vertex3f(20.0f, -0.41f, -20.0f);
	
	floorBatch.MultiTexCoord2f(0, 0.0f, texSize);
	floorBatch.Vertex3f(-20.0f, -0.41f, -20.0f);
	floorBatch.End();

	makeMirrorAndBorder();


	glGenTextures(3, uiTextures);   //generiranje Texture Objects za uèinkovito manjavanje med teksturami 
	
	// bindanje marble.tga za tla
	glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
	LoadTGATexture("marble.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	//bindanje moonlike.tga za kroglo
	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	LoadTGATexture("moonlike.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	LoadTGATexture("LavaCracks.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	//ustvari in bindaj FBO
	glGenFramebuffers(1,&fboName);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);

	//ustvari depth renderbuffer
	glGenRenderbuffers(1, &depthBufferName);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferName);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32,mirrorTexWidth,mirrorTexHeight);

	//ustvari texksturo kamor se bo shranila reflekcija
	glGenTextures(1, &mirrorTexture);
	glBindTexture(GL_TEXTURE_2D, mirrorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mirrorTexWidth, mirrorTexHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//nastavi teksturo na  GL_COLOR_ATTACHMENT0 in depth FBO
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTexture, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferName);

	// Make sure all went well
	gltCheckErrors();
	
	// Reset framebuffer binding
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

}


void ShutdownRC(void)
{
    glDeleteTextures(3, uiTextures);
	glDeleteTextures(1, &mirrorTexture);

	// Make sure default FBO is bound
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	// Cleanup textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Cleanup RBOs
	glDeleteRenderbuffers(1, &depthBufferName);

	// Cleanup FBOs
	glDeleteFramebuffers(1, &fboName);
}

void DrawWorld(GLfloat yRot)
{
	static GLfloat vFloorBarva[]={0.0f, 0.0f, 1.0f, 1.0f};
	static GLfloat vKrofBarva[]={1.0f, 0.0f, 0.0f, 1.0f};
	M3DMatrix44f mCamera;
	//cameraFrame.GetCameraMatrix(mCamera);
	//modelViewMatrix_Stack.PushMatrix(mCamera);
	modelViewMatrix_Stack.GetMatrix(mCamera);
	
	M3DVector4f vLightEyePos;
	//transformiranje koordinate luèi v glede na kamero
	m3dTransformVector4(vLightEyePos, vLightPos, mCamera);

	////izriši tla
	//shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vFloorBarva);
	//floorBatch.Draw();
	modelViewMatrix_Stack.PushMatrix();

	glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
	static GLfloat vWhite[]={1.0f, 1.0f, 1.0f};
	static GLfloat vFloorColor[] = { 1.0f, 1.0f, 1.0f, 0.75f};
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeline.GetModelViewProjectionMatrix(), vFloorColor, 0);

	floorBatch.Draw();

	modelViewMatrix_Stack.PopMatrix();

	modelViewMatrix_Stack.PushMatrix();
	//nariši krof

	modelViewMatrix_Stack.Translate(0.0f, 0.2f, -2.5f);
	modelViewMatrix_Stack.Rotate(yRot,1.0f, 1.0f, 1.0f);
	

	//uporaba ze narejnega point light diffuse shaderja
	shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(),vLightEyePos,vKrofBarva);
	torusBatch.Draw();
	modelViewMatrix_Stack.PopMatrix();
	
	//modelViewMatrix_Stack.Translate(1.0f, 0.0f, -2.0f);
	//modelViewMatrix_Stack.Scale(3.0f,3.0f,3.0f);

	modelViewMatrix_Stack.PushMatrix();
	modelViewMatrix_Stack.Translate(0.0f, 0.2f, -2.5f);
	modelViewMatrix_Stack.Rotate(-yRot,1.0f, 0.0f, 0.0f);

	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
								 modelViewMatrix_Stack.GetMatrix(),
								 transformPipeline.GetProjectionMatrix(),
								 vLightEyePos, 
								 vWhite,
								 0);
	sphereBatch.Draw();


	modelViewMatrix_Stack.PopMatrix();
	//modelViewMatrix_Stack.PopMatrix();

}
// Called to draw scene
void RenderScene(void)
{
	static CStopWatch	rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;

	
	// pocisti barvo in depth bufferje
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	M3DVector3f vCameraPos;
	M3DVector3f vCameraForward;
	M3DVector3f vMirrorPos;
	M3DVector3f vMirrorForward;

	cameraFrame.GetOrigin(vCameraPos);
	cameraFrame.GetForwardVector(vCameraForward);

	// nastavi pozicijo za mirror frame
	vMirrorPos[0] = 0.0;
	vMirrorPos[1] = 0.1f;
	vMirrorPos[2] = -6.0f; // view pos is actually behind mirror
	mirrorFrame.SetOrigin(vMirrorPos);

	// Calculate direction of mirror frame (camera)
	// Because the position of the mirror is known relative to the origin
	// find the direction vector by adding the mirror offset to the vector
	// of the viewer-origin
	vMirrorForward[0] = vCameraPos[0];
	vMirrorForward[1] = vCameraPos[1];
	vMirrorForward[2] = (vCameraPos[2] + 5);
	m3dNormalizeVector3(vMirrorForward);
	mirrorFrame.SetForwardVector(vMirrorForward);
	
	// first render from the mirrors perspective
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboName);
	glDrawBuffers(1, fboBuffs);
	glViewport(0, 0, mirrorTexWidth, mirrorTexHeight);

	// Draw scene from the perspective of the mirror camera
	modelViewMatrix_Stack.PushMatrix();	
		M3DMatrix44f mMirrorView;
		mirrorFrame.GetCameraMatrix(mMirrorView);
		modelViewMatrix_Stack.MultMatrix(mMirrorView);

		// Flip the mirror camera horizontally for the reflection
		modelViewMatrix_Stack.Scale(-1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DrawWorld(yRot);

		modelViewMatrix_Stack.PopMatrix();

		// Reset FBO. Draw world again from the real cameras perspective
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffers(1, windowBuff);
	glViewport(0, 0, screenWidth, screenHeight);
	modelViewMatrix_Stack.PushMatrix();	
		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMatrix_Stack.MultMatrix(mCamera);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DrawWorld(yRot);

		// Now draw the mirror surfaces
		modelViewMatrix_Stack.PushMatrix();
			modelViewMatrix_Stack.Translate(0.0f, -0.4f, -5.0f);
			if(vCameraPos[2] > -5.0)
			{
				glBindTexture(GL_TEXTURE_2D, mirrorTexture); // Reflection
				shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(), 0);
			}
			else
			{
				// If the camera is behind the mirror, just draw black
				shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
			}
			mirrorBatch.Draw();
	
			/*shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGrey);
			mirrorBorderBatch.Draw();*/
		modelViewMatrix_Stack.PopMatrix();
	modelViewMatrix_Stack.PopMatrix();


	/*
	M3DMatrix44f mRotationMatrix;
	m3dRotationMatrix44(mRotationMatrix, m3dDegToRad(45), 0.0f, 0.0f, 1.0f);
	
	GLfloat vRed[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	shaderManager.UseStockShader(GLT_SHADER_FLAT,mRotationMatrix, vRed);
	triangleBatch.Draw();*/

	//modelViewMatrix_Stack.PushMatrix(); //matrika identiteta se shrani na sklad
	
	//DrawWorld(yRot);

	//modelViewMatrix_Stack.PopMatrix();

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
	ShutdownRC();
	return 0;
}
// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Other Libs (to load images)
#include <SOIL/SOIL.h>

// Include CV
#include <cv.h>
#include <highgui.h>

#include "../common/shader.hpp"
#include "../functions/function.hpp"
using namespace std;
using namespace cv;

int main( void )
{
	GLFWwindow* window;
	CvCapture* capture;
	IplImage* image;

	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Image segmentation", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

  GLuint VertexArrayID;
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

	// The textures we're going to render to
	GLuint UpdatedTexture;
	GLuint CamTexture;
	GLuint BackgroundTexture;
	glGenTextures(1, &UpdatedTexture);
	glGenTextures(1, &CamTexture);
	glGenTextures(1, &BackgroundTexture);

	// Give an empty image to OpenGL ( the last "0" means "empty" )
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, 1024, 768, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

  capture = cvCaptureFromCAM(0);
  assert(capture);

	// The fullscreen quad's FBO
	static const GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};

	static const GLfloat g_quad_uv_buffer_data[] = {
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
	};

	GLuint quad_vertexbuffer;
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint quad_uvbuffer;
	glGenBuffers(1, &quad_uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_uv_buffer_data), g_quad_uv_buffer_data, GL_STATIC_DRAW);

	// Create and compile our GLSL program from the shaders
	GLuint quad_programID = LoadShaders( "shaders/Passthrough.vertexshader", "shaders/Passthrough.fragmentshader" );
	GLuint texID = glGetUniformLocation(quad_programID, "UpdatedTexture");
	GLuint camID = glGetUniformLocation(quad_programID, "CamTexture");
	GLuint backID = glGetUniformLocation(quad_programID, "BackgroundTexture");

	Mat frameBGR;
	Mat frameGray;
	Mat frameGrayFormer;
	Mat frameGrayBackground;
	Mat frameUpdated;
	Mat frameHSV;
	Mat frameHueBackground;
	Mat frameSatBackground;
	Mat frameBlue;
	Mat frameGreen;
	Mat frameRed;
	Mat frameHue;
	Mat frameSaturation;
	Mat frameValue;
	uchar* dataFrameGray;
	uchar* dataFrameGrayFormer;
	uchar* dataFrameGrayBackground;
	uchar* dataFrameUpdated;
	uchar* dataFrameHSV;
	uchar* dataFrameBlue;
	uchar* dataFrameGreen;
	uchar* dataFrameRed;
	uchar* dataFrameHue;
	uchar* dataFrameSat;
	uchar* dataFrameSatBackground;
	uchar* dataFrameVal;
	std::vector<Mat> channelsHSV, channelsBGR;
	uint frameRows; // image->height
	uint frameColumns; // image->width
	bool firstCapture = true;

	do {

    image = cvQueryFrame(capture);
  	if(!cvGrabFrame(capture)) { // capture a frame
      printf("Could not grab a frame\n\7");
      exit(0);
    }

		// BGR Frame
		frameBGR = cvarrToMat(image);
		frameRows   = frameBGR.rows; // image->height
		frameColumns = frameBGR.cols; // image->width
		dataFrameUpdated = (uchar*)calloc(frameRows * frameColumns, sizeof(uchar));

		// Red, Green, Blue Frames
		split(frameBGR, channelsBGR);
		frameBlue = channelsBGR[0];
		frameGreen = channelsBGR[1];
		frameRed = channelsBGR[2];
		dataFrameBlue = frameBlue.data;
		dataFrameGreen = frameGreen.data;
		dataFrameRed = frameRed.data;

		// 1st Gray Frame (used as current frame)
		cvtColor(frameBGR, frameGray, CV_BGR2GRAY);
		dataFrameGray = frameGray.data;

		// Hue, Saturation, Value Frames
		cvtColor(frameBGR, frameHSV, CV_BGR2HSV);
		dataFrameHSV = frameHSV.data;
		split(frameHSV, channelsHSV);
		frameHue = channelsHSV[0];
		frameSaturation = channelsHSV[1];
		frameValue = channelsHSV[2];
		dataFrameHue = frameHue.data;
		dataFrameSat = frameSaturation.data;
		dataFrameVal = frameValue.data;

		// -------------------------------------------------------------------------------------------------------------------------------------
		// -------------------------------------------------------------------------------------------------------------------------------------

		if (!firstCapture) {
			SimpleBackgroundDetectionHSV(dataFrameSatBackground, dataFrameSat, dataFrameUpdated, frameRows, frameColumns); // background detection in HSV
			// SimpleBackgroundDetectionRGB(dataFrameGrayBackground, dataFrameGray, dataFrameUpdated, frameRows, frameColumns); // background detection in RGB
			// MvtDetectionRGB(dataFrameGrayFormer, dataFrameGray, dataFrameUpdated, frameRows, frameColumns); // movement detection
		} else {
			// Background Gray frame (taken at the beginnning)
			frameGrayBackground =  frameGray.clone();
			dataFrameGrayBackground = frameGrayBackground.data;
			// Background HSV frame (taken at the beginnning)
			frameSatBackground =  frameSaturation.clone();
			dataFrameSatBackground = frameSatBackground.data;
			SimpleBackgroundDetectionHSV(dataFrameSat, dataFrameSat, dataFrameUpdated, frameRows, frameColumns); // background detection in HSV
			//SimpleBackgroundDetectionRGB(dataFrameGray, dataFrameGray, dataFrameUpdated, frameRows, frameColumns); // background detection in RGB
			//MvtDetectionRGB(dataFrameGray, dataFrameGray, dataFrameUpdated, frameRows, frameColumns); // movement detection
			firstCapture = false;
		}
		// Sobel(dataFrameGray, dataFrameUpdated, frameRows, frameColumns); // sobel :  edge detection
		// Threshold(dataFrameGray, dataFrameUpdated, frameRows, frameColumns); // threshold : otsu's algorithm
		// SkinDetectionRGB(dataFrameRed, dataFrameGreen, dataFrameBlue, dataFrameUpdated, frameRows, frameColumns); // skin detection in RGB

		// 2nd Gray frame (used as former frame)
		frameGrayFormer =  frameGray.clone();
		dataFrameGrayFormer = frameGrayFormer.data;

		// Updated Frame
		frameUpdated = Mat(frameRows, frameColumns, CV_8UC1, dataFrameUpdated);

    // -------------------------------------------------------------------------------------------------------------------------------------
		// -------------------------------------------------------------------------------------------------------------------------------------

		// Clear the screen
		glClear( GL_COLOR_BUFFER_BIT );

		// Use our shader
		glUseProgram(quad_programID);

		// -------------------------------------------------------------------------------------------------------------------------------------
		// ----------------------------------------------------------- UPDATED TEXTURE ---------------------------------------------------------
		// -------------------------------------------------------------------------------------------------------------------------------------
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, UpdatedTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameColumns, frameRows, 0, GL_RED, GL_UNSIGNED_BYTE, dataFrameUpdated);
		// Set our "renderedTexture" sampler to user Texture Unit 0
		glUniform1i(texID, 0);

		// -------------------------------------------------------------------------------------------------------------------------------------
		// ----------------------------------------------- CAMERA TEXTURE (FOREGROUND) ---------------------------------------------------------
		// -------------------------------------------------------------------------------------------------------------------------------------
		// Bind our texture in Texture Unit 1
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, CamTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_BGR, GL_UNSIGNED_BYTE, image->imageData);
		// Set our "renderedTexture" sampler to user Texture Unit 1
		glUniform1i(camID, 1);

		//-------------------------------------------------------------------------------------------------------------------------------------
	  //----------------------------------------------------------- BACKGROUND TEXTURE ------------------------------------------------------
		//-------------------------------------------------------------------------------------------------------------------------------------
		// Bind our texture in Texture Unit 2
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, BackgroundTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		int width, height;
		uchar* backgroundImage = SOIL_load_image("images/plage.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, backgroundImage);
		//Set our "renderedTexture" sampler to user Texture Unit 2
		glUniform1i(backID, 2);

		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, quad_uvbuffer);
		glVertexAttribPointer(
			1,                  // attribute 1. No particular reason for 1, but must match the layout in the shader.
			3,                  // size
		GL_FLOAT,          	  // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the triangles
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		free(dataFrameUpdated);
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	glDeleteBuffers(1, &quad_vertexbuffer);
	glDeleteBuffers(1, &quad_uvbuffer);
	glDeleteTextures(1, &UpdatedTexture);
	glDeleteTextures(1, &CamTexture);
	glDeleteTextures(1, &BackgroundTexture);
  glDeleteVertexArrays (1, &VertexArrayID);
  glDeleteProgram (quad_programID);
  // Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

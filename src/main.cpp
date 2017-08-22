
#ifdef WIN32
#include <windows.h>
#endif 
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <set>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define FREEIMAGE_LIB
#include <FreeImage.h>

#include <mesh.h>
#include <object.h>
#include <ocTree.h>
#include <ocTree.h>
#include <unistd.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <mutex>
#include <renderNode.h>
#include <synchObject.h>

// Forward declarations of the needed functions
void setup(void);
void cleanup(void);
void draw(void);
void reshape(int w, int h);
bool createShaders();
void keyboardFunction(unsigned char k, int x, int y);
void motionFunction (int x, int y);
void mouseFunction (int Button, int state, int x, int y);

FIBITMAP* loadImage(const std::string filename);

// Global variables

// @FIXME: make less absolute
const std::string SelctionLogPath("/home/ri43hiq/selection_log.txt");

std::mutex listMutex;
glm::mat4 model_matrix;
glm::mat4 wand_model_matrix;
std::vector <std::pair <size_t, glm::vec3> >  verticesList;

ocTree* myOcTree;

std::set<size_t> vertexSelection;
bool addedVertices;
bool removedVertices;

float screen_width = 30.0; //real size in cm
float screen_height = 30.0; //real size in cm
float near_clipping = 50.; //how far is the screen away from the user (in cm)
float far_clipping = 4000.; // up to where do we want to render (in cm)

float colors_object[4];

int max_window_width;
int max_window_height;

int old_window_x;
int old_window_y;
int old_mouse_x = 0;
int old_mouse_y = 0;

int left_button_state = 0;
int middle_button_state = 0;

glm::vec3 translation = glm::vec3(0.f,0.f,-100.f);

FIBITMAP* img_texture = 0;
GLuint shaderprogram;

GLuint vao; // Vertex array object
GLuint vertexBuffer; // Declare ID holder vertex buffer
GLuint colourBuffer; // Declare colour buffer
GLuint indexBuffer; // Declare index buffer


GLint pvm_matrix_location;
GLint normal_matrix;
GLint vm_matrix;

Object object1;
Object wand_pos;

std::vector<Mesh*> meshes;
std::vector<Mesh*> meshes_wand;

std::vector<glm::vec3> vertices;

using namespace synchlib;

std::shared_ptr<renderNode> rNode;
std::shared_ptr<SynchObject<size_t> > selectionSyncher;
std::shared_ptr<SynchObject<size_t> > deselectionSyncher;

/* #####################################################
/	Main program
/  ##################################################### */

/**
 * selectionSyncherReceiveFunction()
 * is called whenever new data is sent
 *
 * - build list of size_ts;
 * - change color of vertices in that list
 */
void selectionSyncherReceiveFunction (std::shared_ptr<SynchObject<size_t> > selecSynch) {
	size_t data = selecSynch->getData();
//	std::cout << "Adding vertex #" << int(data) << std::endl;
	listMutex.lock();

	vertexSelection.insert(data);

	listMutex.unlock();
	addedVertices = true;
}

void deselectionSyncherReceiveFunction (std::shared_ptr<SynchObject<size_t> > selecSynch) {
	size_t data = selecSynch->getData();
//	std::cout << "Deleting vertex #" << int(data) << std::endl;

	listMutex.lock();

	if (vertexSelection.find(data) != vertexSelection.end()) {
		vertexSelection.erase(data);
	}

	listMutex.unlock();
	removedVertices = true;
}

int main(int argc, char **argv) {
	std::string file = argv[2];

	colors_object[0] = 1.0;
	colors_object[1] = 0.0;
	colors_object[2] = 0.0;
	colors_object[3] = 0.99;	// DO NOT CHANGE THIS LINE

	rNode = std::make_shared<renderNode>(file, argc, argv);
	std::function<void(std::shared_ptr<SynchObject<size_t> >)> receiveFuncAdd(&selectionSyncherReceiveFunction);
	std::function<void(std::shared_ptr<SynchObject<size_t> >)> receiveFuncRem(&deselectionSyncherReceiveFunction);

	selectionSyncher = SynchObject<size_t>::create();
	selectionSyncher->addReceiveFunction(receiveFuncAdd);

	deselectionSyncher = SynchObject<size_t>::create();
	deselectionSyncher->addReceiveFunction(receiveFuncRem);

	rNode->addSynchObject(selectionSyncher, renderNode::RECEIVER);
	rNode->addSynchObject(deselectionSyncher, renderNode::RECEIVER);

	rNode->init();
	rNode->startSynching();


	// Get maximum window size (screen resolution of primary monitor)
#ifdef WIN32
	max_window_width = GetSystemMetrics(SM_CXSCREEN);
	max_window_height = GetSystemMetrics(SM_CYSCREEN);


#else

#endif
	// testfiles

//	std::string obj_path = "/home/ri43hiq/selection_app/obj_files/cow_FINAL.obj";
//	std::string obj_path = "/home/ri43hiq/selection_app/obj_files/cow_FINAL.obj";
	std::string obj_path = "/home/ri43hiq/selection_app/obj_files/P51_FINAL.obj";
	std::string wand_path = "/home/ri43hiq/selection_app/obj_files/wand_pos.obj";


	glutInit(&argc, argv);		// First initialise GLUT
	/* ################# Call our setup function ################# */
	setup();

	max_window_height = glutGet((GLenum)GLUT_SCREEN_HEIGHT);
	max_window_width = glutGet((GLenum)GLUT_SCREEN_WIDTH);

	// initialize & load Object object1
	if(!object1.loadObject(obj_path,
		aiProcess_CalcTangentSpace       |
		aiProcess_Triangulate
//		aiProcess_JoinIdenticalVertices  |
//		aiProcess_SortByPType
		)) {
			std::cout<<"display object found"<<std::endl;
	} else {
		// if the file could be opened via ASSIMP, call its uploadAllMeshes() function
		object1.uploadAllMeshes();
	}

	if(!wand_pos.loadObject(wand_path,
		aiProcess_CalcTangentSpace       |
		aiProcess_Triangulate            |
		aiProcess_JoinIdenticalVertices  |
		aiProcess_SortByPType)) {
			std::cout<<"MAIN > wand_pos.obj not found"<<std::endl;
	} else {
		// if the file could be opened via ASSIMP, call its uploadAllMeshes() function
		wand_pos.uploadAllMeshes();
	}

	// retrieve mesh objects from loaded object
	meshes = object1.getMeshes();
	meshes_wand = wand_pos.getMeshes();

	// Call our function for creating shaders (compiling and linking)
	if(!createShaders()){
		std::cout<<"Error creating shaders"<<std::endl;
		exit(0);
	}

	usleep(10000);

	pvm_matrix_location = glGetUniformLocation(shaderprogram, "pvm");
	normal_matrix = glGetUniformLocation(shaderprogram, "normalMatrix");
	vm_matrix = glGetUniformLocation(shaderprogram, "vmMatrix");
    glutMainLoop();    // Go into the glut main loop
    rNode->stopSynching();
	cleanup();

    return 0;
}

/* #####################################################
/	create Window, setup openGL states
/  ##################################################### */
void setup(void) {
	addedVertices = false;
	removedVertices = false;
	/* ######################
	/ GLUT Setup
	/  ###################### */
	glutInitContextVersion (3, 3);

	// setting up display
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_STEREO);
    // Configure Window Postion
    glutInitWindowPosition(old_window_x, old_window_y);
    // Configure Window Size
    glutInitWindowSize(1200,800);

    // Create Window (creates also a context)
    glutCreateWindow("VR LAB 02: OpenGL Recab + OpenGL Core");

	// Initialise glew
	glewExperimental = GL_TRUE; //needed as it is old!
	GLenum err = glewInit();
	if (GLEW_OK != err)	{
	   std::cerr<<"Error: "<<glewGetErrorString(err)<<std::endl;
	} else {
	   if (GLEW_VERSION_3_3)
	   {
		  std::cout<<"Driver supports OpenGL 3.3:"<<std::endl;
	   }
	}

	{ // Ignore first error as it is within the GLEW/OpenGL relationship ;)
		GLenum glErr;
		glErr = glGetError();
	}

    glutDisplayFunc(draw);
	glutIdleFunc(draw);

	// Set closing behaviour: If we leave the mainloop (e.g. through user input or closing the window) we continue after the function "glutMainLoop()"
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutFullScreen();
	/* ######################
	/ OpenGL Setup
	/  ###################### */

	// insert gluErrorString beneath every line here to debug error
	glClearColor(.00f,0.0f,0.0f,1.0f);
	// Set depth which is used when clearing the drawing buffer (1.0 is maximum depth)
	glClearDepth(1.0);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);

	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Disable blending
	glDisable(GL_BLEND);

}

//	Clean up function
void cleanup(void){
	// Free memory for texture and deinitialise image loading library
	if(img_texture)
		FreeImage_Unload(img_texture);
	FreeImage_DeInitialise();

	//Free up graphics memory
	glDeleteBuffers(1,&vertexBuffer);
	glDeleteProgram(shaderprogram);
	glDeleteVertexArrays(1, &vao);
}

//	Drawing function (is called for every frame)
void draw(void) {

	float colors_wand[4] = {0.0, 0.80, 0.0, 1.0};
	meshes_wand[0]->changeAllColors(colors_wand);
	wand_pos.updateAllMeshes();

	if (addedVertices) {
		meshes[0]->changeColorsAdd(vertexSelection, colors_object);
		object1.updateAllMeshes();
		addedVertices = false;
		removedVertices = false;
	}

	if (removedVertices) {
		meshes[0]->changeColorsRem(vertexSelection, colors_object);
		object1.updateAllMeshes();
		addedVertices = false;
		removedVertices = false;
	}

//	std::cout << "vertexSelection.size: " << vertexSelection.size() << std::endl;

	GLenum glErr;
	glm::mat4 pMatL, pMatR;
	glm::mat4 wand_pMatL, wand_pMatR;
	rNode->getProjectionMatrices(pMatL,pMatR);

	glm::mat4 vMat;
	rNode->getSceneTrafo(vMat);
	wand_model_matrix =  rNode->getWandTrafo();
	float wand_offset = 0.;

	/**
	 * not up to date
	 * WAND_POS
	 */
	{
		// LEFT
		glDrawBuffer(GL_BACK_LEFT);
		// Clear drawing buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderprogram);

		// WAND
		glm::mat3 vmMat = glm::transpose(glm::inverse(glm::mat3(wand_model_matrix)));
		glm::mat4 pvmMatrix = pMatL *  wand_model_matrix* glm::translate(glm::vec3(0.,0.,wand_offset));
		glProgramUniformMatrix4fv(shaderprogram, pvm_matrix_location, 1, GL_FALSE,glm::value_ptr(pvmMatrix));
		glProgramUniformMatrix3fv(shaderprogram, normal_matrix, 1, GL_FALSE,glm::value_ptr(vmMat));
		glProgramUniformMatrix4fv(shaderprogram, vm_matrix, 1, GL_FALSE,glm::value_ptr(wand_model_matrix));
		wand_pos.drawWithoutParams();
		glUseProgram(0);
	}
	{
		glUseProgram(shaderprogram);

		// LOADED OBJECT
		glm::mat3 vmMat = glm::transpose(glm::inverse(glm::mat3(vMat)));

		glm::mat4 pvmMatrix = pMatL * vMat;
		glProgramUniformMatrix4fv(shaderprogram, pvm_matrix_location, 1, GL_FALSE,glm::value_ptr(pvmMatrix));
		glProgramUniformMatrix3fv(shaderprogram, normal_matrix, 1, GL_FALSE,glm::value_ptr(vmMat));
		glProgramUniformMatrix4fv(shaderprogram, vm_matrix, 1, GL_FALSE,glm::value_ptr(vMat));

		// proj_matrix, view_matrix, model_matrix
		object1.drawWithoutParams();
		glUseProgram(0);

	}

	{
		// RIGHT
		glDrawBuffer(GL_BACK_RIGHT);
		// Clear drawing buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderprogram);

		// WAND
		glm::mat3 vmMat = glm::transpose(glm::inverse(glm::mat3( wand_model_matrix)));
		glm::mat4 pvmMatrix = pMatR *  wand_model_matrix* glm::translate(glm::vec3(0.,0.,wand_offset));
		glProgramUniformMatrix4fv(shaderprogram, pvm_matrix_location, 1, GL_FALSE,glm::value_ptr(pvmMatrix));
		glProgramUniformMatrix3fv(shaderprogram, normal_matrix, 1, GL_FALSE,glm::value_ptr(vmMat));
		glProgramUniformMatrix4fv(shaderprogram, vm_matrix, 1, GL_FALSE,glm::value_ptr(wand_model_matrix));
		wand_pos.drawWithoutParams();
		glUseProgram(0);
	}
	{
		glUseProgram(shaderprogram);

		// LOADED OBJECT
		glm::mat3 vmMat = glm::transpose(glm::inverse(glm::mat3(vMat)));

		glm::mat4 pvmMatrix = pMatR * vMat;
		glProgramUniformMatrix4fv(shaderprogram, pvm_matrix_location, 1, GL_FALSE,glm::value_ptr(pvmMatrix));
		glProgramUniformMatrix3fv(shaderprogram, normal_matrix, 1, GL_FALSE,glm::value_ptr( vmMat ));
		glProgramUniformMatrix4fv(shaderprogram, vm_matrix, 1, GL_FALSE,glm::value_ptr(vMat));
		object1.drawWithoutParams();
		glUseProgram(0);
	}

	if(!rNode->synchFrame()){
		glutLeaveMainLoop();
	}
	glutSwapBuffers();
}

/* #####################################################
/	Reshape function (is called if the window size is changed and on window creation)
/	@DELETED
/  ##################################################### */

// WASD navigation
// @DELETED

char* filetobuf(char *file)
{
    FILE *fptr;
    long length;
    char *buf;

    fptr = fopen(file, "rb"); /* Open file for reading */
    if (!fptr) /* Return NULL on failure */{
		std::cout<<"could not open "<<file<<std::endl;
        return NULL;
	}
    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    buf = (char*)malloc(length+1);
    fseek(fptr, 0, SEEK_SET);
    fread(buf, length, 1, fptr);
    fclose(fptr);
    buf[length] = 0;

    return buf; /* Return the buffer */
}

	// Simple function to create, compile and link shaders (from "https://www.opengl.org/wiki/Tutorial2:_VAOs,_VBOs,_Vertex_and_Fragment_Shaders_(C_/_SDL)" )
bool createShaders(){
	int IsCompiled_VS, IsCompiled_FS;
    int IsLinked;
    int maxLength;
    char *vertexInfoLog;
    char *fragmentInfoLog;
    char *shaderProgramInfoLog;

	/* These pointers will receive the contents of our shader source code files */
    char *vertexsource, *fragmentsource;

    /* These are handles used to reference the shaders */
    GLuint vertexshader, fragmentshader;

	 /* Read our shaders into the appropriate buffers */

    vertexsource = filetobuf("/home/ri43hiq/workspace/selection_app/shader/vertex.vert");
    fragmentsource = filetobuf("/home/ri43hiq/workspace/selection_app/shader/fragment.frag");
    vertexshader = glCreateShader(GL_VERTEX_SHADER); // Create an empty vertex shader handle/
    glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);

    /* Compile the vertex shader */
    glCompileShader(vertexshader);

    glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &IsCompiled_VS);
    if(IsCompiled_VS == FALSE)
    {
       glGetShaderiv(vertexshader, GL_INFO_LOG_LENGTH, &maxLength);

       /* The maxLength includes the NULL character */
       vertexInfoLog = (char *)malloc(maxLength);

       glGetShaderInfoLog(vertexshader, maxLength, &maxLength, vertexInfoLog);
	   	   	 std::cout<<"could not compile vertex shader: \n"<<vertexInfoLog<<std::endl;
       /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
       /* In this simple program, we'll just leave */
       free(vertexInfoLog);
	   free(vertexsource);
	   free(fragmentsource);
       return false;
    }

    /* Create an empty fragment shader handle */
    fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);

    /* Compile the fragment shader */
    glCompileShader(fragmentshader);

    glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &IsCompiled_FS);
    if(IsCompiled_FS == FALSE)
    {
       glGetShaderiv(fragmentshader, GL_INFO_LOG_LENGTH, &maxLength);

       /* The maxLength includes the NULL character */
       fragmentInfoLog = (char *)malloc(maxLength);

       glGetShaderInfoLog(fragmentshader, maxLength, &maxLength, fragmentInfoLog);
	   	 std::cout<<"could not compile fragment shader: \n"<<fragmentInfoLog<<std::endl;
       /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
       /* In this simple program, we'll just leave */
       free(fragmentInfoLog);
	   free(vertexsource);
	   free(fragmentsource);
       return false;
    }

    shaderprogram = glCreateProgram();

    /* Attach our shaders to our program */
    glAttachShader(shaderprogram, vertexshader);
    glAttachShader(shaderprogram, fragmentshader);
    glLinkProgram(shaderprogram);


    glGetProgramiv(shaderprogram, GL_LINK_STATUS, (int *)&IsLinked);
    if(IsLinked == FALSE)
    {
       /* Noticed that glGetProgramiv is used to get the length for a shader program, not glGetShaderiv. */
       glGetProgramiv(shaderprogram, GL_INFO_LOG_LENGTH, &maxLength);

       /* The maxLength includes the NULL character */
       shaderProgramInfoLog = (char *)malloc(maxLength);

       /* Notice that glGetProgramInfoLog, not glGetShaderInfoLog. */
       glGetProgramInfoLog(shaderprogram, maxLength, &maxLength, shaderProgramInfoLog);
	   std::cout<<"could not link program: \n"<<shaderProgramInfoLog<<std::endl;

       /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
       /* In this simple program, we'll just leave */
       free(shaderProgramInfoLog);
	   free(vertexsource);
	   free(fragmentsource);
       return false;
    }

	return true;
}

FIBITMAP* loadImage(const std::string filename){
    FreeImage_Initialise();
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename.c_str(), 0);

    if (format == -1) {
        std::cerr << "Could not find image: " << filename << " - Aborting." << std::endl;
        return false;
    }
    if (format == FIF_UNKNOWN) {
        std::cerr << "Couldn't determine file format - attempting to get from file extension..." << std::endl;
        format = FreeImage_GetFIFFromFilename(filename.c_str());
        if (!FreeImage_FIFSupportsReading(format)) {
            std::cerr << "Detected image format cannot be read!" << std::endl;
            return false;
        }
    }
    FIBITMAP* bitmap = FreeImage_Load(format, filename.c_str());
    int bitsPerPixel = FreeImage_GetBPP(bitmap);
    FIBITMAP* bitmap32;
    if (bitsPerPixel == 32) {
            bitmap32 = bitmap;
    } else {

        bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
        FreeImage_Unload(bitmap);
    }


    return bitmap32;
}

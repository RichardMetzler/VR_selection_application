
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
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/ext.hpp"

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


#include <synchObject.h>
#include <renderServer.h>

// Forward declarations of the needed functions
void setup(void);
void cleanup(void);
void draw(void);
void keyboardFunction(unsigned char k, int x, int y);
void checkButton();
void addSendSizeTs();
void removeSendSizeTs();

FIBITMAP* loadImage(const std::string filename);

// Global variables

const std::string SelctionLogPath("/home/ri43hiq/workspace/textfiles/selection_logs/selection_log.txt");

bool addButtonPressed = false;
bool remButtonPressed = false;

bool firstAction;

std::vector <std::pair <size_t, glm::vec3> >  verticesList;

std::chrono::high_resolution_clock::time_point t1;
std::chrono::high_resolution_clock::time_point t2;

ocTree* myOcTree;

std::map<size_t, size_t> vertexSelection;
std::set<size_t> newlySelected;
std::set<size_t> newlyDeleted;

float screen_width = 30.0; //real size in cm
float screen_height = 30.0; //real size in cm
float near_clipping = 50.; //how far is the screen away from the user (in cm)
float far_clipping = 4000.; // up to where do we want to render (in cm)

int max_window_width;
int max_window_height;

int old_window_x;
int old_window_y;
int old_mouse_x = 0;
int old_mouse_y = 0;

float maxSearchRadius;

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

glm::mat4 model_matrix;

Object object1;
Object wand_pos;

std::vector<Mesh*> meshes;
std::vector<Mesh*> meshes_wand;
std::vector<glm::vec3> vertices;

using namespace synchlib;

std::shared_ptr<renderServer> rServer;
std::shared_ptr<SynchObject<size_t> > selectionSyncher;
std::shared_ptr<SynchObject<size_t> > deselectionSyncher;

/* #####################################################
/	Main program
/  ##################################################### */

//auto t1 = std::chrono::high_resolution_clock::now();
//std::this_thread::sleep_for(std::chrono::seconds(2));
//auto t2 = std::chrono::high_resolution_clock::now();
//std::cout<<"time: "<<std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()<<std::endl;


int main(int argc, char **argv) {

	if(argc < 2){
		//kein Argument: Beenden!
	}
	std::string file = argv[1];

	rServer = std::make_shared<renderServer>(file, argc, argv);

	selectionSyncher = SynchObject<size_t>::create();
	rServer->addSynchObject(selectionSyncher,renderServer::SENDER,0);

	deselectionSyncher = SynchObject<size_t>::create();
	rServer->addSynchObject(deselectionSyncher,renderServer::SENDER,0);

	rServer->init(true);
	rServer->startSynching();

	// Get maximum window size (screen resolution of primary monitor)
	#ifdef WIN32
		max_window_width = GetSystemMetrics(SM_CXSCREEN);
		max_window_height = GetSystemMetrics(SM_CYSCREEN);


	#else

	#endif
	// testfiles

//	 std::string obj_path = "obj_files/testthing_thick.ply";
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
//			aiProcess_JoinIdenticalVertices  |
//			aiProcess_SortByPType
			)) {
			std::cout<<"object not found"<<std::endl;
	} else {
		// if the file could be opened via ASSIMP, call its uploadAllMeshes() function
		std::cout << "object found" << std::endl;
		object1.uploadAllMeshes();
	}

	if(!wand_pos.loadObject(wand_path,
			aiProcess_CalcTangentSpace		|
			aiProcess_Triangulate )) {
			std::cout<<"SERVER > wand_pos not found"<<std::endl;
	} else {
		std::cout<<"wand_pos.obj found"<<std::endl;
		wand_pos.uploadAllMeshes();
	}

	// retrieve mesh objects from loaded object
	meshes = object1.getMeshes();
	meshes_wand = wand_pos.getMeshes();


//	myOcTree = new ocTree(meshes, 100, 3, true);		// additional information will be printed during construction
//	myOcTree = new ocTree(meshes, 100, 3, false);		// no additional information will be printed during construction
	myOcTree = new ocTree(meshes, 50, 4, false);			// cow
	std::vector<glm::vec3> vertices = myOcTree->getVertices();

	// converting the <vec3> vector to a vector of <size_t, vec3> pairs
	for(size_t t = 0; t < vertices.size();++t){
		std::pair<size_t, glm::vec3> myPair;
		myPair = std::make_pair(t,vertices[t]);
		verticesList.push_back(myPair);
	}

	myOcTree->buildTreeRecursively(verticesList);

	myOcTree->debugFirstVertex();
	myOcTree->debugTreeInfo();

	maxSearchRadius = myOcTree->getMaximumSearchRadius();

	// Call our function for creating shaders (compiling and linking)
//	if(!createShaders()){
//		std::cout<<"Error creating shaders"<<std::endl;
//		exit(0);
//	}

//	usleep(10000);

	pvm_matrix_location = glGetUniformLocation(shaderprogram, "pvm");
	normal_matrix = glGetUniformLocation(shaderprogram, "normalMatrix");
//	vm_matrix = glGetUniformLocation(shaderprogram, "vmMatrix");

    glutMainLoop();    // Go into the glut main loop
    rServer->stopSynching();
	cleanup();

    return 0;
}

/* #####################################################
/	create Window, setup openGL states
/  ##################################################### */
void setup(void) {
	// setting bool vars for timelogging
	t1 = std::chrono::high_resolution_clock::now();

//	if (!firstAction) {
//		firstAction = false;
//	} else {
//		firstAction = true;
//	}

	/* ######################
	/ GLUT Setup
	/  ###################### */
	glutInitContextVersion (3, 3);

	// setting up display
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
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

    glutDisplayFunc(draw); // Call to the drawing function (if glutPostDisplay is called or anything else has changed)
	glutIdleFunc(draw); // Call to drawing function (if nothing has happed, but we want animations to be drawn smoothly)
	glutKeyboardFunc(keyboardFunction);

	// Set closing behaviour: If we leave the mainloop (e.g. through user input or closing the window) we continue after the function "glutMainLoop()"
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	/* ######################
	/ OpenGL Setup
	/  ###################### */

	// insert gluErrorString beneath every line here to debug error
	glClearColor(.40f,0.4f,0.4f,1.0f);
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

// @DELETED WASD navigation etc
void keyboardFunction(unsigned char k, int x, int y) {
	switch (k) {
		case 'p':
		case 'P':
		{
			// save current selection to text file
			std::ofstream file(SelctionLogPath);
//			std::string data("vertId\tx\t\ty\t\tz\n");
			std::string data("vertId\n");
			file << data;

			auto it = vertexSelection.begin();
			for (int n=0; n != vertexSelection.size(); ++n) {
				std::stringstream line;
				line << it->first << "\t" << it->second << std::endl; // only id; use line below for id + coordinates
//				line << *it << "\t" << verticesList[*it].second.x << "\t" << verticesList[*it].second.y << "\t" << verticesList[*it].second.z << "\n";
				file << line.str();
				std::advance(it, 1);
			}
			std::cout << "vertex selection printed to selection_log.txt" << std::endl;
			break;
		}

//		case 'c':
//		case 'C':
//		{
//			// reset current selection to be empty
//			float colors[4] = {0.89, 0.89, 0.89, 1.0};
//			meshes[0]->changeColorsRem(vertexSelection, colors);
//			vertexSelection.clear();
//			object1.updateAllMeshes();
//			std::cout << "vertex selection cleared" << std::endl;
//			break;
//		}

		case 's':
		case 'S':
		{
			std::cout << "S pressed, searching for leaf 001100110000" << std::endl;
			std::vector<bool> check = {false,false,true,true,false,false,true,true,false,false,false,false};		// 001100110000
			ocTree* testNodeId = myOcTree->getNodeByIdentifierArray(check, false);
			testNodeId->debugIdentifierasString();
			auto it = testNodeId->getVerticesInBounds().begin();
			for (int n=0; n != testNodeId->getVerticesInBounds().size(); ++n) {
				std::cout << "x,y,z: " << (*it).second.x << ", "<< (*it).second.y << ", "<< (*it).second.z << std::endl;
				++it;
			}
			break;
		}

//		case 'h':
//		case 'H':
//		{
//			std::cout << "H pressed" << std::endl;
//			glm::vec3 target {13.1326, 100.045, 4.747};
//			myOcTree->addVerticesToSelectionByCoordinates (target, maxSearchRadius, vertexSelection, true);
//
//			addSendSizeTs();
//			break;
//		}

		default:
			break;
	}
}

//	Drawing function (is called for every frame)
void draw(void) {
	GLenum glErr;
	checkButton();

	// get inverse scene transformation matrix
	glm::mat4 view_matrix;
	rServer->getSceneTransform(view_matrix);
	glm::mat4 myMat = glm::inverse(glm::mat4(view_matrix));

	if (addButtonPressed == true) {
		// get position of wand, brutally cast it
		glm::mat4 wandTransform = rServer->getWandTransform();
		glm::vec3 wandPos = wandTransform[3].xyz();
		glm::vec4 wandPosv4(wandPos.x, wandPos.y, wandPos.z, 1.0f);
		glm::vec4 queryPointv4 = myMat * wandPosv4;

		// get list of vertices
		glm::vec3 queryPoint(queryPointv4.x, queryPointv4.y, queryPointv4.z);
		myOcTree->addVerticesToSelectionByCoordinates (queryPoint, maxSearchRadius, newlySelected, false);
		// @FIXME: Zeit einbauen
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1);
		size_t time_span_ms = time_span.count();

		addSendSizeTs();
		for (auto it = newlySelected.begin(); it != newlySelected.end(); ++it) {
			vertexSelection.insert(std::pair<size_t,size_t>(*it, time_span_ms));
		}
		newlySelected.clear();
	}

	if (remButtonPressed == true) {
		glm::mat4 wandTransform = rServer->getWandTransform();
		glm::vec3 wandPos = wandTransform[3].xyz();
		glm::vec4 wandPosv4(wandPos.x, wandPos.y, wandPos.z, 1.0f);
		glm::vec4 queryPointv4 = myMat * wandPosv4;

		// get list of vertices
		glm::vec3 queryPoint(queryPointv4.x, queryPointv4.y, queryPointv4.z);
		myOcTree->addVerticesToSelectionByCoordinates (queryPoint, maxSearchRadius, newlySelected, false);
		removeSendSizeTs();
//		vertexSelection.insert(newlySelected.begin(), newlySelected.end());
		for (auto it = newlySelected.begin(); it != newlySelected.end(); ++it) {
			if (vertexSelection.find(*it) != vertexSelection.end()) {
				vertexSelection.erase(*it);
			}
		}
		newlySelected.clear();
	}

	glutSwapBuffers();
}

void addSendSizeTs()
{
	/**
	 * VERTICES ADDED
	 */
	// iterate through list of vertices & send positions
	std::set<size_t>::iterator it = newlySelected.begin();
	for (int n=0; n != newlySelected.size(); ++n) {
//		std::cout << n << " - sending vertex #" << (*it) << std::endl;
		selectionSyncher->setData(*it);
		selectionSyncher->send();
		std::advance(it, 1);
	}
	std::cout << "vertexSelection size: " << vertexSelection.size() << std::endl;
}

void removeSendSizeTs()
{
	std::set<size_t>::iterator it = newlySelected.begin();
	for (int n =0; n != newlySelected.size(); ++n) {
		deselectionSyncher->setData(*it);
		deselectionSyncher->send();
		std::advance(it, 1);
	}
	std::cout << "vertexSelection size: " << vertexSelection.size() << std::endl;
}

void checkButton(){
	std::list<glm::ivec2> buttonQueue;
	rServer->getButtonQueue(buttonQueue);
	for(auto it = buttonQueue.begin(); it != buttonQueue.end(); ++it){
		// BUTTON 3 - ADD (left button of the wand)
		if((*it)[0] == 3){
			if((*it)[1] == 1) {
				addButtonPressed = true;
				t2 = std::chrono::high_resolution_clock::now();
			}
			if ((*it)[1] == 0) {
				addButtonPressed = false;
			}
		}
		// BUTTON 1 - REMOVE (right button of the wand)
		if((*it)[0] == 1){
			// button 1 is pressed (1 == pressed)
			if((*it)[1] == 1) {
				remButtonPressed = true;
				t2 = std::chrono::high_resolution_clock::now();
			}
			// button 1 is released
			if ((*it)[1] == 0) {
				remButtonPressed = false;
			}
		}
//		std::cout << "vertexSelection.size: " << vertexSelection.size() << std::endl;


		/**
		 * non-relevant buttons for this application
		 */

		// BUTTON 2
		if((*it)[0] == 2){
			// button 2 is pressed (1 == pressed)
			if((*it)[1] == 1) {
				std::cout << "Button 2 PRESSED" << std::endl;
			}
			// button 2 is pressed (01 == pressed)
			if ((*it)[1] == 0) {
				std::cout << "Button 2 RELEASED" << std::endl;
			}
		}

		// BUTTON 4
		if((*it)[0] == 4){
			if((*it)[1] == 1) {
				std::cout << "Button 4 PRESSED" << std::endl;
			}
			if ((*it)[1] == 0) {
				std::cout << "Button 4 RELEASED" << std::endl;
			}
		}
	}
}

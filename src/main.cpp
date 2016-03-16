#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#define _GLIBCXX_USE_NANOSLEEP
#include <thread>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

#include "GLSL.h"
#include "Program.h"
#include "Camera.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Scene.h"

#include "SoundPlayer.h"
#include <Importer.hpp>
#include "AnimatedCharacter.hpp"

#include "ogldev_skinned_mesh.h"
#include "ogldev_util.h"
#include "skinning_technique.h"
#include "texture.h"

#include "Cheb.hpp"

using namespace std;
//using namespace Eigen;

bool keyToggles[256] = {false}; // only for English keyboards!

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from
string OBJ_FILE = "";
string ATTACHMENT_FILE = "";
string SKELETON_FILE = "";

shared_ptr<Camera> camera;
shared_ptr<Program> prog;
shared_ptr<Program> progSimple;
shared_ptr<Program> progPhong;
shared_ptr<Program> progSkinPhong;
shared_ptr<Program> boneProg;
shared_ptr<Scene> scene;

SoundPlayer soundPlayer;

//float h = 0.0;
//shared_ptr<AnimatedCharacter> chicken;

//SkinnedMesh chickenMesh;
//shared_ptr<SkinningTechnique> m_pEffect;
//DirectionalLight m_directionalLight;

//===== Cheb =====//
shared_ptr<Cheb> cheb;

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
	keyToggles[key] = !keyToggles[key];
	switch(key) {
		case 'h':
			scene->step();
			break;
		case 'r':
			scene->reset();
			break;
	}
}

static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if(state == GLFW_PRESS) {
		camera->mouseMoved(xmouse, ymouse);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if(action == GLFW_PRESS) {
		bool shift = mods & GLFW_MOD_SHIFT;
		bool ctrl  = mods & GLFW_MOD_CONTROL;
		bool alt   = mods & GLFW_MOD_ALT;
		camera->mouseClicked(xmouse, ymouse, shift, ctrl, alt);
	}
}

static void init()
{
	GLSL::checkVersion();
	
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);
	// Enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	progSimple = make_shared<Program>();
	progSimple->setShaderNames(RESOURCE_DIR + "simple_vert.glsl", RESOURCE_DIR + "simple_frag.glsl");
	progSimple->setVerbose(false); // Set this to true when debugging.
	progSimple->init();
	progSimple->addUniform("P");
	progSimple->addUniform("MV");
	
	prog = make_shared<Program>();
	prog->setVerbose(true); // Set this to true when debugging.
	prog->setShaderNames(RESOURCE_DIR + "phong_vert.glsl", RESOURCE_DIR + "phong_frag.glsl");
	prog->init();
	prog->addUniform("P");
	prog->addUniform("MV");
	prog->addUniform("kdFront");
	prog->addUniform("kdBack");
	prog->addAttribute("vertPos");
	prog->addAttribute("vertNor");
	prog->addAttribute("vertTex");
    
    boneProg = make_shared<Program>();
    boneProg->setVerbose(true);
    boneProg->setShaderNames(RESOURCE_DIR + "bone_vert.glsl", RESOURCE_DIR + "bone_frag.glsl");
    boneProg->init();
    boneProg->addUniform("P");
    boneProg->addUniform("MV");
    boneProg->addUniform("bones");
    boneProg->addAttribute("vertPos");
    boneProg->addAttribute("vertNor");
    boneProg->addAttribute("vertTex");
    boneProg->addAttribute("boneIds");
    boneProg->addAttribute("boneIds2");
    boneProg->addAttribute("boneWeights");
    boneProg->addAttribute("boneWeights2");
    
//    boneProg->addUniform("uTexUnit");
    
    
    progPhong = make_shared<Program>();
    progPhong->setVerbose(true); // Set this to true when debugging.
    progPhong->setShaderNames(RESOURCE_DIR + "phong_vert1.glsl", RESOURCE_DIR + "phong_frag1.glsl");
    progPhong->init();
    progPhong->addUniform("P");
    progPhong->addUniform("MV");
    progPhong->addAttribute("vertPos");
    progPhong->addAttribute("vertNor");
    progPhong->addAttribute("vertTex");
    
    progSkinPhong = make_shared<Program>();
    progSkinPhong->setVerbose(true); // Set this to true when debugging.
    progSkinPhong->setShaderNames(RESOURCE_DIR + "phong_skinning_vert.glsl", RESOURCE_DIR + "phong_skinning_frag.glsl");
    progSkinPhong->init();
    progSkinPhong->addUniform("P");
    progSkinPhong->addUniform("MV");
    progSkinPhong->addUniform("bindPose");
    progSkinPhong->addUniform("animTrans");
    progSkinPhong->addAttribute("vertPos");
    progSkinPhong->addAttribute("vertNor");
    progSkinPhong->addAttribute("vertTex");
    
    progSkinPhong->addAttribute("weights0");
    progSkinPhong->addAttribute("weights1");
    progSkinPhong->addAttribute("weights2");
    progSkinPhong->addAttribute("weights3");
    progSkinPhong->addAttribute("weights4");
    progSkinPhong->addAttribute("bones0");
    progSkinPhong->addAttribute("bones1");
    progSkinPhong->addAttribute("bones2");
    progSkinPhong->addAttribute("bones3");
    progSkinPhong->addAttribute("numBoneInfluences");
    
    GLSL::checkError(GET_FILE_LINE);
    
	camera = make_shared<Camera>();
    camera->setTranslation(Eigen::Vector3f(0.0, 0.0, -2.0));
    camera->setRotation(Eigen::Vector2f(-10.0, 10.0));

	scene = make_shared<Scene>();
	scene->load(RESOURCE_DIR);
	scene->tare();
	scene->init();
    
    // Initialize the audio player
    soundPlayer.playBackgroundMusic();

    //===== Init Cheb ======//
    cheb = make_shared<Cheb>(OBJ_FILE, ATTACHMENT_FILE, SKELETON_FILE, Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    
//    m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
//    m_directionalLight.AmbientIntensity = 0.55f;
//    m_directionalLight.DiffuseIntensity = 0.9f;
//    m_directionalLight.Direction = Vector3f(1.0f, 0.0, 0.0);
//    
//    m_pEffect = make_shared<SkinningTechnique>();
//    
//    if (!m_pEffect->Init())
//    {
//        printf("Error initializing the lighting technique\n");
//    }
//    
//    m_pEffect->Enable();
//    
//    m_pEffect->SetColorTextureUnit(0);
//    m_pEffect->SetDirectionalLight(m_directionalLight);
//    m_pEffect->SetMatSpecularIntensity(0.0f);
//    m_pEffect->SetMatSpecularPower(0);
    
    // Initialize the chicken
//    chicken = make_shared<AnimatedCharacter>((RESOURCE_DIR + "Chicken/bartender.dae"), 1, 1.0f, 0);
    
//    if (!chickenMesh.LoadMesh((RESOURCE_DIR + "Chicken/bartender.dae")))
//    {
//        printf("Mesh load failed\n");
//    }
//    else
//    {
//        cout << "Successfully loaded the chicken mesh" << endl;
//    }
    
    // Initialize time.
    glfwSetTime(0.0);
	
	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);
}

void render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	
	// Use the window size for camera.
	glfwGetWindowSize(window, &width, &height);
	camera->setAspect((float)width/(float)height);
	
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles[(unsigned)'l']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	
	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
	camera->applyViewMatrix(MV);

	// Draw grid
	progSimple->bind();
	glUniformMatrix4fv(progSimple->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
	glUniformMatrix4fv(progSimple->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
	glLineWidth(2.0f);
	float x0 = -0.5f;
	float x1 = 0.5f;
	float z0 = -0.5f;
	float z1 = 0.5f;
	int gridSize = 10;
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	for(int i = 1; i < gridSize; ++i) {
		if(i == gridSize/2) {
			glColor3f(0.1f, 0.1f, 0.1f);
		} else {
			glColor3f(0.8f, 0.8f, 0.8f);
		}
		float x = x0 + i / (float)gridSize * (x1 - x0);
		glVertex3f(x, 0.0f, z0);
		glVertex3f(x, 0.0f, z1);
	}
	for(int i = 1; i < gridSize; ++i) {
		if(i == gridSize/2) {
			glColor3f(0.1f, 0.1f, 0.1f);
		} else {
			glColor3f(0.8f, 0.8f, 0.8f);
		}
		float z = z0 + i / (float)gridSize * (z1 - z0);
		glVertex3f(x0, 0.0f, z);
		glVertex3f(x1, 0.0f, z);
	}
	glEnd();
	glColor3f(0.4f, 0.4f, 0.4f);
	glBegin(GL_LINE_LOOP);
	glVertex3f(x0, 0.0f, z0);
	glVertex3f(x1, 0.0f, z0);
	glVertex3f(x1, 0.0f, z1);
	glVertex3f(x0, 0.0f, z1);
	glEnd();
	progSimple->unbind();

	// Draw scene
	prog->bind();
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
	MV->pushMatrix();
	scene->draw(MV, prog);
	MV->popMatrix();
	prog->unbind();
    
    // Draw Cheb
    if (keyToggles[(unsigned)'g'])
    {
        progSkinPhong->bind();
        cheb->init();
        glUniformMatrix4fv(progSkinPhong->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
        
        cheb->drawGPU(MV, progSkinPhong);
        
        progSkinPhong->unbind();
    }
    else
    {
        progPhong->bind();
        glUniformMatrix4fv(progPhong->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
        
        MV->scale(Eigen::Vector3f(0.4, 0.4, 0.4));
        
        cheb->draw(MV, progPhong);
        
        progPhong->unbind();
    }
    
//    boneProg->bind();
//    glUniformMatrix4fv(boneProg->getUniform("P"), 1, GL_FALSE, P->topMatrix().data());
//    MV->pushMatrix();
    // Draw something here
//    if(!chicken->isAnimating()) {
//        chicken->startAnimation("walk");
//    }
    
//    chicken->position = Eigen::Vector3f(0.0, 0.0, 0.0);
//    chicken->scale = Eigen:: Vector3f(1.0, 1.0, 1.0);
//    chicken->rotate = 0;
    
//    float newTime = glfwGetTime();
//    chicken->drawChar(MV, boneProg, 1.0/60.0);
    
//    vector<Matrix4f> Transforms;
//    
//    chickenMesh.BoneTransform((float)glfwGetTime(), Transforms);
//    
//    chickenMesh.Render();
    
//    MV->popMatrix();
//    boneProg->unbind();
    
//    h += newTime;
	
	//////////////////////////////////////////////////////
	// Cleanup
	//////////////////////////////////////////////////////
	
	// Pop stacks
	MV->popMatrix();
	P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
}

void stepperFunc()
{
	while(true) {
		if(keyToggles[(unsigned)' ']) {
            scene->addSpheres(cheb->getVertPos());
			scene->step();
		}
		this_thread::sleep_for(chrono::microseconds(1));
	}
}

int main(int argc, char **argv)
{
    if(argc < 5) {
        cout << "Please specify the resource directory, .obj file, attachment file, and skeleton file respectively." << endl;
        return 0;
    }
    RESOURCE_DIR = argv[1] + string("/");
    OBJ_FILE = argv[2];
    ATTACHMENT_FILE = argv[3];
    SKELETON_FILE = argv[4];
	
	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(700, 700, "Brandon Clark", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Initialize scene.
	init();
	// Start simulation thread.
	thread stepperThread(stepperFunc);
    
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
    
	// Quit program.
	stepperThread.detach();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

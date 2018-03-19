#include "Scene.h"
#include "Shaders.h"

using namespace std;

Scene::Scene(Camera* c) {
    // Create the skybox
    _skybox = new SkyBox(fetchShader("cubemap.vtx", "cubemap.frag"), c);

    // Create the light source
    glm::vec3 lightPos(0.5f, 0.8f, 1.5f);
    glm::vec3 lightCol(1.0f, 1.0f, 1.0f);
    _lightSrc = new LightSource(fetchShader("phongShader.vtx", "phongShader.frag"), c, lightPos, lightCol);

    // Load objects in the scene
    _objects.push_back(new Shape(fetchShader("phongShader.vtx", "phongShader.frag"), c, Shape::Type::SQUARE_3D, lightPos, lightCol));
    _objects.push_back(new Shape(fetchShader("phongShader.vtx", "phongShader.frag"), c, Shape::Type::PYRAMID, lightPos, lightCol));
    _objects.push_back(new Shape(fetchShader("phongShader.vtx", "phongShader.frag"), c, Shape::Type::STONE_PYRAMID, lightPos, lightCol));
    _objects.push_back(new Ground(fetchShader("phongShader.vtx", "phongShader.frag"), c));

    Cube* testCube = new Cube(fetchShader("phongShader.vtx", "phongShader.frag"), c, _lightSrc);
    testCube->set2DTexture("assets/grass.jpg");
    testCube->setPosition(glm::vec3(-0.8, 0.2, 0.0));
    testCube->setSize(0.1);
    _objects.push_back(testCube);

    Cube* testCube2 = new Cube(fetchShader("phongShader.vtx", "phongShader.frag"), c, _lightSrc);
    testCube2->setColor(glm::vec3(0.3, 0.5, 0.8));
    testCube2->setPosition(glm::vec3(-0.1, -0.35, 0.5));
    testCube2->setSize(0.3);
    testCube2->isLit(true);
    testCube2->setRotation(glm::vec3(0.0, 0.0, 1.0), 0.1);
    _objects.push_back(testCube2);

    Cube* testCube3 = new Cube(fetchShader("phongShader.vtx", "phongShader.frag"), c, _lightSrc);
    testCube3->set2DTexture("assets/stones.jpg");
    testCube3->setPosition(glm::vec3(0.3, 0.2, 0.0));
    testCube3->setSize(0.2);
    testCube3->isLit(true);
    _objects.push_back(testCube3);
}

void Scene::draw() {
    // Enable blending to create transparency effect
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);       // accept fragment if closer to camera

    // Clear the screen
    glClearColor(0.0, 0.0, 0.0, 1.0);   // black
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render our objects
    _skybox->render();  // always 1st
    _lightSrc->render();
    for (auto it : _objects)
        it->render();

    // Check for problems
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) printErr(err);

    // Flush the buffers
    glFlush();
}

void Scene::printErr(GLenum err) {
    switch (err) {
        // Error descriptions can be found at https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetError.xhtml
        case GL_INVALID_ENUM:
            std::cerr << "Error: invalid enum" << std::endl;
            break;
        case GL_INVALID_VALUE:
            std::cerr << "Error: invalid value"  << std::endl;
            break;
        case GL_INVALID_OPERATION:
            std::cerr << "Error: invalid op"  << std::endl;
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            std::cerr << "Error: invalid frame buffer op"  << std::endl;
            break;
        case GL_OUT_OF_MEMORY:
            std::cerr << "Error: out of memory" << std::endl;
            break;
        case GL_STACK_UNDERFLOW:
            std::cerr << "Error: stack underflow"  << std::endl;
            break;
        case GL_STACK_OVERFLOW:
            std::cerr << "Error: stack overflow" << std::endl;
            break;
        default:
            std::cerr << "Unknown error: " << err << std::endl;
            break;
    }
}

void Scene::cleanUp() {
    _skybox->cleanUp();
    delete _skybox;

    for (auto it : _objects) {
        it->cleanUp();
        delete it;
    }
}
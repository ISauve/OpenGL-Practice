#ifndef OPENGL_OBJECT_H
#define OPENGL_OBJECT_H

#include <SFML/Graphics.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "../Glad.h"

static bool DEBUG = false;

/*************************************************************
                   Abstract Base Classes
 *************************************************************/

class Scene;
class Object {
protected:
    GLuint _shaderProgram;
    GLuint _vao;

    // Pointer to the scene in order to access the camera, light source, terrain, etc
    Scene* _scene;

    // Buffer ID storage for clean-up purposes: individually references may also be kept
    std::vector <GLuint> _bufferIDs;
    std::vector <GLuint> _textureIDs;

    // State information
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;
    bool _lit;
    glm::vec3 _position;
    float _size;
    glm::vec3 _rotationAxis;
    float _rotationSpeed;

    // Helpers
    GLuint initializeVAO();
    GLuint storeToVBO(GLfloat*, int);
    GLuint storeToVBO(GLfloat*, int, GLfloat*, int);
    GLuint storeToEBO(GLuint*, int);
    GLuint storeTex(std::string, GLenum = GL_REPEAT);
    GLuint storeCubeMap(std::vector<std::string>&);

public:
    Object(GLuint, Scene*);
    virtual ~Object();

    virtual void render() {};   // Can throw a std::runtime_error

    /**** Modifiers ****/
    virtual void isLit(bool b) { _lit = b; };

    // Sets position relative to the terrain
    virtual void setPosition(glm::vec3);    // Can throw a std::runtime_error exception if terrain isn't set

    virtual void setSize(float s) { _size = s; };
    virtual void setRotation(glm::vec3 axis) { _rotationAxis = axis; _rotationSpeed = 0; };
    virtual void setRotation(glm::vec3 axis, float speed) { _rotationAxis = axis; _rotationSpeed = speed; };
    // Note: Have to define 2 versions of setRotation bc can't set default arguments on virtual functions
};

/*************************************************************
                       Special Objects
 *************************************************************/

class SkyBox : public Object {
public:
    SkyBox(GLuint, Scene*);
    ~SkyBox() final { glDeleteProgram(_shaderProgram); };

    void render() override;

    // Set helpful error messages for base class modifiers that don't make sense
    void isLit(bool) override                   { std::cerr << "Error: can't change skybox lighting\n"; };
    void setPosition(glm::vec3) override        { std::cerr << "Error: can't change skybox position\n"; };
    void setSize(float) override                { std::cerr << "Error: can't change skybox size\n"; };
    void setRotation(glm::vec3) override        { std::cerr << "Error: can't change skybox rotation\n"; };
    void setRotation(glm::vec3, float) override { std::cerr << "Error: can't change skybox rotation\n"; };
};


class LightSource : public Object {
    glm::vec3 _onColor;
    glm::vec3 _color;
    bool _changed;

public:
    LightSource(GLuint, Scene*, glm::vec3, glm::vec3);
    ~LightSource() final { glDeleteProgram(_shaderProgram); };

    void render() override;

    // Accessors
    glm::vec3 Position() { return _position; };
    glm::vec3 Color() { return _color; };

    // Modifiers
    void setColor(glm::vec3);
    void isLit(bool) override;
    void setSize(float) override;

    // Base class modifiers that don't make sense
    void setRotation(glm::vec3) override        { std::cerr << "Error: rotation not implemented for light sources\n"; };
    void setRotation(glm::vec3, float) override { std::cerr << "Error: rotation not implemented for light sources\n"; };
};


class Terrain : public Object {
    const float SIZE = 100.0f;  // size of each square terrain object
    const float MAX_HEIGHT = 5.0f;
    int _texture;
    int _numIndices;

    int _vertexCount;       // number of vertices along each side
    sf::Image _heightMap;   // height map for this

    // Calculate these at initialization
    std::vector< std::vector<float> > _heights;
    std::vector< std::vector<glm::vec3> > _normals;

    float barryCentric(glm::vec3, glm::vec3, glm::vec3, glm::vec2);
    void unbind();

public:
    Terrain(GLuint, Scene*, std::string);
    ~Terrain() final { glDeleteProgram(_shaderProgram); };

    void render() override;

    // Accessor
    float getSize() { return SIZE; };
    float getHeightAt(float, float);
    glm::vec3 getNormalAt(int, int);

    void set2DTexture(std::string);

    // Sets the position of the terrain in absolute terms
    void setPosition(glm::vec3 p) override { _position = p; };

    // Base class modifiers that don't make sense
    void setSize(float) override                { std::cerr << "Error: terrain size is a compile-time constant\n"; };
    void setRotation(glm::vec3) override        { std::cerr << "Error: can't change terrain rotation\n"; };
    void setRotation(glm::vec3, float) override { std::cerr << "Error: can't change terrain rotation\n"; };
};


/*************************************************************
                          Shapes
 *************************************************************/

class Shape : public Object {
protected:
    int _texture;
    int _numElements;
    bool _usesIndices;

    void unbind();

public:
    Shape(GLuint, Scene*);
    virtual ~Shape() { glDeleteProgram(_shaderProgram); };

    void render() override;
};


class Cube : public Shape {
public:
    Cube(GLuint, Scene*);

    void setColor(glm::vec3);           // Applies a uniform color
    void setColors(GLfloat*, int);      // Applies a custom color data for each vertex
    void set2DTexture(std::string);     // Applies texture to each face
};


class Square : public Shape {
public:
    Square(GLuint, Scene*);

    void set2DTexture(std::string);
};

/*************************************************************
                          Models
 *************************************************************/

// Represents a small portion of a model
// Should never be instantiated outside of Model class
class Mesh : public Object {
public:     // Forward declarations
    struct Vertex;
    struct Texture;

private:
    std::vector<Vertex> _vertices;
    std::vector<unsigned int> _indices;
    std::vector<Texture> _textures;

    bool _blend;

    // Overridden version for this class only (defn in Object.cpp)
    GLuint storeToVBO(Mesh::Vertex*, long);
    void unbind();

public:
    Mesh(GLuint, Scene*);

    void render() override;

    // Modifiers
    void addData(std::vector<Vertex>, std::vector<unsigned int>, std::vector<Texture>);
    void setBlend(bool b) { _blend = b; };

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };
    struct Texture {
        std::string name;
        std::string path;
        GLuint id;
    };
};


// Container that loads data using assimp & turns it into a collection of mesh objects
class Model : public Object {
    std::vector<Mesh*> _meshes;
    std::string _pathRoot;

    // Processing helpers
    void processNode(aiNode*, const aiScene*, GLuint, Scene*);
    Mesh* processMesh(aiMesh*, const aiScene*, GLuint, Scene*);
    std::vector<Mesh::Texture> getTextures(aiMaterial*, aiTextureType, std::string, std::string);

public:
    Model(std::string, GLuint, Scene*);
    ~Model() final;

    void render() override;

    // Modifiers
    void setBlend(bool);
    void isLit(bool) override;
    void setPosition(glm::vec3) override;
    void setSize(float) override;
    void setRotation(glm::vec3) override;
    void setRotation(glm::vec3, float) override;
};

#endif

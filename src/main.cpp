#include <iostream>
#include "shaders.h"

using namespace std;

// ------------------------------------------------
//  Forward declarations
// ------------------------------------------------

GLFWwindow* initWindow();
void renderFlashingTriangle(float time);
void renderRectangle();

// ------------------------------------------------
//  Main
// ------------------------------------------------

int main(int argc, char** argv) {
    // Initialize GLFW & the window context
    glfwInit();
    GLFWwindow* window = initWindow();
    if (!window) {
        cerr << "Failed to create OpenGL context... exiting" << endl;
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        cerr << "Failed to initialize GLAD... exiting" << endl;
        return -1;
    }
    cout << "Created OpenGL " << GLVersion.major  << "." <<  GLVersion.minor << " context" <<  endl;

    // Enter the rendering loop
    auto t_start = chrono::high_resolution_clock::now();
    while( !glfwWindowShouldClose(window) ) {
        // Enable blending to create transparency effect
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Draw a black background
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        auto t_now = chrono::high_resolution_clock::now();
        float time = chrono::duration_cast<chrono::duration<float>>(t_now - t_start).count();
        renderFlashingTriangle(time);
        glDrawArrays(GL_TRIANGLES, 0, 3);   // Draws the currently bound VAO

        renderRectangle();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);  // Draws the currently bound VAO using indices

        // Flush the buffers
        glFlush();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return  0;
}

// ------------------------------------------------
//  Helpers
// ------------------------------------------------

GLFWwindow* initWindow() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Practice", nullptr, nullptr);
    return window;
}

void renderFlashingTriangle(float time) {
    // Create & bind a vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Define the vertices of the triangle
    GLfloat vertices[][5] = {
            { -0.8f, 0.9f, 1.0f, 0.0f, 0.0f },  // Red
            { -0.9f, 0.7f, 0.0f, 1.0f, 0.0f },  // Green
            { -0.7f, 0.7f, 0.0f, 0.0f, 1.0f }   // Blue
    };

    // Send the data to the OpenGL server by storing it in a buffer object
    GLuint vbo;
    glGenVertexArrays(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Initialize our shaders & generate a shader program
    std::vector<ShaderInfo> shaders = {
            { GL_VERTEX_SHADER,     "src/passThrough.vtx" },
            { GL_FRAGMENT_SHADER,   "src/triangle.frag" }
    };
    GLuint shaderProgram = LoadShaders(shaders);
    glUseProgram(shaderProgram);

    // Connect the shader ‘in’ variables to the data in the buffer
    GLint posAttrib = glGetAttribLocation(shaderProgram, "vPosition");
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE,
                          5*sizeof(float),   // Offset btwn consecutive vertices
                          0);                // Offset fr/ beginning of vertex
    glEnableVertexAttribArray(posAttrib);

    GLint colAttrib = glGetAttribLocation(shaderProgram, "vColor");
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE,
                          5*sizeof(float),
                          (void*)(2*sizeof(float)));  // Need to skip over 2 attributes to read color data
    glEnableVertexAttribArray(colAttrib);

    // Vary the transparency with time
    GLint uniColor = glGetUniformLocation(shaderProgram, "transparency");
    time = (sin(time * 2.0f) + 1.0f) / 2.0;
    glUniform1f(uniColor, time);
}

void renderRectangle() {
    // Create & bind a vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Initialize data for the vertices we're drawing
    GLfloat positions[] = {
             -0.6f, 0.9f,  // Top-left
             -0.4f, 0.9f,  // Top-right
             -0.4f, 0.7f,  // Bottom-right
             -0.6f, 0.7f,  // Bottom-left
    };
    GLfloat colors[] = {
            1.0f, 0.0f, 1.0f,  // Pink
            0.0f, 1.0f, 1.0f,  // Light blue
            1.0f, 1.0f, 0.0f,  // Yellow
            1.0f, 1.0f, 1.0f,  // White
    };

    // Initialize a vertex buffer object
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // We'll use glBufferSubData to load the data in 2 parts
    glBufferData(GL_ARRAY_BUFFER,                     // target
                 sizeof(positions) + sizeof(colors),  // total size
                 NULL,                                // no data (yet)
                 GL_STATIC_DRAW);                     // method

    glBufferSubData(GL_ARRAY_BUFFER,     // target
                    0,                   // no offset
                    sizeof(positions),   // size
                    positions);          // data

    glBufferSubData(GL_ARRAY_BUFFER,     // target
                    sizeof(positions),   // offset = sizeof previous data entered
                    sizeof(colors),      // size
                    colors);             // data

    // Specify indices to select which vertices to draw
    GLuint indices[] = {
            0, 1, 2,        // Triangle 1
            2, 3, 0,        // Triangle 2
    };

    // Initialize an element array buffer object to store these indices
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    // Store the data in this buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Initialize our shaders & generate a shader program
    std::vector<ShaderInfo> shaders = {
            { GL_VERTEX_SHADER,     "src/passThrough.vtx" },
            { GL_FRAGMENT_SHADER,   "src/rectangle.frag" }
    };
    GLuint shaderProgram = LoadShaders(shaders);
    glUseProgram(shaderProgram);

    // Connect the shader ‘in’ variables to the data in the GL_ARRAY_BUFFER
    GLint posAttrib = glGetAttribLocation(shaderProgram, "vPosition");
    glVertexAttribPointer(posAttrib,   // shader attribute location
                          2,           // size (number of components per vertex)
                          GL_FLOAT,    // data type
                          GL_FALSE,    // if data should be normalized
                          0,           // offset between consecutive vertices (0 = "tightly packed")
                          0);          // offset from the start of the buffer object
    glEnableVertexAttribArray(posAttrib);

    GLint colAttrib = glGetAttribLocation(shaderProgram, "vColor");
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(positions)));
    glEnableVertexAttribArray(colAttrib);
}
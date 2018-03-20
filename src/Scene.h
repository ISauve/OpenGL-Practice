#ifndef OPENGL_SCENE_H
#define OPENGL_SCENE_H

#include "Objects/Object.h"

#include <vector>

class Scene {
    std::vector<Shape*> _shapes;
    SkyBox* _skybox;
    LightSource* _lightSrc;

    bool _isLit;

    void printErr(GLenum);

public:
    Scene(Camera* c);
    ~Scene();

    void draw();
    void toggleLight();
};


#endif //OPENGL_SCENE_H

#ifndef TRANSPARENCY_SCENE_H
#define TRANSPARENCY_SCENE_H

#include "IScene.h"
#include "SimpleGLShader.h"
#include "Cube.h"
#include "Mesh.h"

#include "glm/glm.hpp"
#include <vector>

#define MAX 6

#define SCENE2

class TransparencyScene : public IScene {
private:
	SimpleGLShader prog;

	glm::vec3 bgColor;
	
	GLuint maxNodes;
	GLuint counterBuffer, llBuffer;
    GLuint quadVAO, headPtrTex;
    GLuint pass1Index, pass2Index;
    GLuint clearBuf;

	//Skybox* skybox;

	RenderObject *cube;
	RenderObject *plane;

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

    void setMatrices();
    void compileAndLinkShader();
    void drawScene();
    void initShaderStorage();
	void pass1(Camera* camera);
    void pass2();
    void clearBuffers();

	std::vector<glm::vec4> colors;

	float angle = 0.0f;
	GLuint quadPosition;

public:
	TransparencyScene(int w, int h);
	~TransparencyScene();

    void initScene();
    void update( float t );
	void draw(Camera* camera);
    void resize(int, int);

	GLuint nodes = 20;

	void addNode() {
		maxNodes = ++nodes * width * height;
		std::cout << "Nodes: " << nodes << std::endl;
		prog.send_uniform("maxNodes", maxNodes);
	}
	void removeNode() {
		maxNodes = --nodes * width * height;
		std::cout << "Nodes: " << nodes << std::endl;
		prog.send_uniform("maxNodes", maxNodes);
	}

};

#endif // TRANSPARENCY_SCENE_H

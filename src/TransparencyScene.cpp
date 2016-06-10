#include "TransparencyScene.h"
#include <iostream>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/constants.hpp"

TransparencyScene::TransparencyScene(int w, int h) : IScene(w, h) { }
TransparencyScene::~TransparencyScene() {
	// Delete all
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	// TODO
}

void TransparencyScene::initScene() {
	compileAndLinkShader();
	bgColor = glm::vec3(0.0f);
#ifdef SCENE1
	cube = new Mesh("../models/teapot.obj", "../textures/metal.png");
#endif // SCENE1
#ifdef SCENE2
	cube = new Mesh("../models/monkeyhead.obj", "../textures/container.jpg");
#endif // SCENE2
#ifdef SCENE3
	cube = new Mesh("../models/dead 123456.obj", "../textures/CHR_Deadpool_Body_TEXTSET_Color_NormX.jpg");
#endif // SCENE3
#ifdef SCENE4
	bgColor = glm::vec3(1.0f);
	cube = new Mesh("../models/DarthVader.obj", "../textures/metal.png");
#endif // SCENE4
#ifdef SCENE5
	bgColor = vec3(0.8);
	cube = new Mesh("../models/juanjo.obj", "../textures/CHR_Deadpool_Body_TEXTSET_Color_NormX.jpg");
#endif // SCENE5

	plane = new Mesh("../models/plane.obj", "../textures/metal.png");
	glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0f);

	prog.use();
	initShaderStorage();
	prog.unuse();

	pass1Index = glGetSubroutineIndex(prog.program(), GL_FRAGMENT_SHADER, "pass1");
	pass2Index = glGetSubroutineIndex(prog.program(), GL_FRAGMENT_SHADER, "pass2");

	GLfloat vertices[] = { 
		-1.0f, -1.0f, 0.0f, 
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 
		-1.0f, 1.0f, 0.0f 
	};

	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);

	glGenBuffers(1, &quadPosition);
	glBindBuffer(GL_ARRAY_BUFFER, quadPosition);
	glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, quadPosition);
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	for(int i = 0; i < 20; i++) {
		float r = ((float)( std::rand() % 1000)) * 0.001;
		float g = ((float)( std::rand() % 1000)) * 0.001;
		float b = ((float)( std::rand() % 1000)) * 0.001;
		float a = ((float)( std::rand() % 1000)) * 0.001;
		if(a >= 0.85) {
			a = 0.4;
		}
		colors.push_back(glm::vec4(r, g, b, a));
	}

	std::cout << "HP: " << glGetUniformLocation(prog.program(), "headPointers") << std::endl;
	std::cout << "NNC: " << glGetUniformLocation(prog.program(), "nextNodeCounter") << std::endl;
	std::cout << "L: " << glGetUniformLocation(prog.program(), "lilit") << std::endl;
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	std::cout << "Vertex max: " << pow(MAX, 3) * cube->_size << std::endl;

	//skybox = new Skybox("textures/canyon/");
}


void TransparencyScene::update( float t ) {
	angle += 0.1 * t;
}

void TransparencyScene::draw(Camera* camera) {
	//skybox->render(camera);
	prog.use();
	clearBuffers();
	pass1(camera);
	pass2();
	prog.unuse();
}

void TransparencyScene::pass1(Camera* camera) {
	glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &pass1Index);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	view = camera->GetViewMatrix();
	projection = camera->GetProjectionMatrix();

	glDepthMask(GL_FALSE);

	// draw scene
	drawScene();

	glFinish();
}

void TransparencyScene::pass2() {
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glDepthMask(GL_TRUE);
	glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &pass2Index);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	prog.send_uniform("bgColor", bgColor);

	view = glm::mat4(1.0f);
	projection = glm::mat4(1.0f);
	model = glm::mat4(1.0f);

	setMatrices();

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);
}

void TransparencyScene::clearBuffers() {
	static GLuint zero = 0;

	// Reset atomic counter.
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, counterBuffer);
	glClearBufferData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, GL_RED, GL_UNSIGNED_INT, &zero);
	//glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);

	// Reset head pointers copying clear buffer into texture.
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf);

	glBindTexture(GL_TEXTURE_2D, headPtrTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void TransparencyScene::setMatrices() {
	glm::mat4 modelView = view * model;

	prog.send_uniform("normal", glm::mat3(glm::vec3(modelView[0]), glm::vec3(modelView[1]), glm::vec3(modelView[2])));
	prog.send_uniform("modelView", modelView);
	prog.send_uniform("modelViewProj", projection * modelView);
}

void TransparencyScene::resize(int w, int h) {
	/*maxNodes = 20 * w * h;
	GLint nodeSize = 5 * sizeof(GLfloat)+1 * sizeof(GLuint); // The size of a linked list node

	// Our atomic counter
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, buffers[COUNTER_BUFFER]);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);

	// The buffer for the head pointers, as an image texture
	glBindTexture(GL_TEXTURE_2D, headPtrTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, w, h);
	glBindImageTexture(0, headPtrTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

	// The buffer of linked lists
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffers[LINKED_LIST_BUFFER]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxNodes * nodeSize, NULL, GL_DYNAMIC_DRAW);

	std::vector<GLuint> headPtrClearBuf(w*h, 0xffffffff);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, headPtrClearBuf.size() * sizeof(GLuint), &headPtrClearBuf[0], GL_STATIC_COPY);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);*/

	glViewport(0, 0, w, h);
	width = w;
	height = h;
	maxNodes = nodes * width * height;
	prog.send_uniform("maxNodes", maxNodes);

	/*std::vector<GLuint> headPtrClearBuf(width*height, 0xffffffff);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, headPtrClearBuf.size() * sizeof(GLuint), &headPtrClearBuf[0], GL_STATIC_COPY);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);*/
}

void TransparencyScene::compileAndLinkShader() {
	prog.load("../shader/shader.vert", GL_VERTEX_SHADER);
	prog.load("../shader/shader.frag", GL_FRAGMENT_SHADER);
	prog.compile_and_link();
	prog.add_uniform("maxNodes");
	prog.add_uniform("normal");
	prog.add_uniform("modelView");
	prog.add_uniform("modelViewProj");
	prog.add_uniform("Kd");
	prog.add_uniform("bgColor");
}

void TransparencyScene::drawScene() {
	int n = 0;
	float size = 1.0f;

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cube->texture);
	glUniform1i(glGetUniformLocation(prog.program(), "texImage"), 1);

#ifdef SCENE1
	size = 0.03f;
	for (int i = 0; i < MAX; i++) {
		for (int j = 0; j < MAX; j++) {
			for (int k = 0; k < MAX; k++) {
				vec4 kd = colors[n++ % colors.size()];
				prog.send_uniform("Kd", kd);
				model = glm::translate(mat4(1.0f), vec3(i * 2, j * 2, k * 2));
				//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
				model = glm::rotate(model, glm::radians(-angle), glm::vec3(0, 1, 0));
				model = glm::scale(model, glm::vec3(size));
				setMatrices();
				cube->render();
			}
		}
	}
#endif // SCENE1
#ifdef SCENE2
	size = 0.5f;
	for (int i = 0; i < MAX / 2; i++) {
		for (int j = 0; j < MAX; j++) {
			for (int k = 0; k < MAX; k++) {
				vec4 kd = colors[n++ % colors.size()];
				kd.w = 1.0f;
				prog.send_uniform("Kd", kd);
				//model = glm::translate(mat4(1.0f), vec3(i * 2, -200, k * 2));
				model = glm::translate(mat4(1.0f), vec3(i * 2, j * 2, k * 2));
				//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
				model = glm::rotate(model, glm::radians(-angle), glm::vec3(0, 0, 1));
				model = glm::scale(model, glm::vec3(size));
				setMatrices();
				cube->render();
			}
		}
	}

	glDisable(GL_CULL_FACE);
	vec4 kd = colors[n++ % colors.size()];
	prog.send_uniform("Kd", kd);
	model = glm::translate(mat4(1.0f), vec3((MAX / 2) * 2, ((MAX / 2)) * 1.5, (MAX / 2) * 2));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
	//model = glm::rotate(model, glm::radians(-angle), glm::vec3(0, 0, 1));
	model = glm::scale(model, glm::vec3(size*5.5f));
	setMatrices();
	plane->render();
	glEnable(GL_CULL_FACE);

	for (int i = (MAX / 2) + 1; i < MAX + 1; i++) {
		for (int j = 0; j < MAX; j++) {
			for (int k = 0; k < MAX; k++) {
				vec4 kd = colors[n++ % colors.size()];
				prog.send_uniform("Kd", kd);
				//model = glm::translate(mat4(1.0f), vec3(i * 2, -200, k * 2));
				model = glm::translate(mat4(1.0f), vec3(i * 2, j * 2, k * 2));
				//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
				model = glm::rotate(model, glm::radians(-angle), glm::vec3(0, 0, 1));
				model = glm::scale(model, glm::vec3(size));
				setMatrices();
				cube->render();
			}
		}
	}
#endif // SCENE2
#ifdef SCENE3
	size = 2.0f;
	vec4 kd = colors[n++ % colors.size()];
	prog.send_uniform("Kd", kd);
	model = glm::translate(mat4(1.0f), vec3(0, -200, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(-angle), glm::vec3(0, 0, 1));
	model = glm::scale(model, glm::vec3(size));
	setMatrices();
	cube->render();
#endif // SCENE3
#ifdef SCENE4
	size = 4.0f;
	prog.send_uniform("Kd", glm::vec4(0.0, 0.0, 0.0, 0.4));
	model = glm::translate(mat4(1.0f), vec3(0, 0, 0));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::scale(model, glm::vec3(size));
	setMatrices();
	cube->render();
#endif // SCENE4
#ifdef SCENE5
	glDisable(GL_CULL_FACE);
	size = 15.0f;
	vec4 kd = colors[n++ % colors.size()];
	prog.send_uniform("Kd", kd);
	model = glm::translate(mat4(1.0f), vec3(0, -200, 0));
	//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
	model = glm::scale(model, glm::vec3(size));
	setMatrices();
	cube->render();
#endif // SCENE5
}

void TransparencyScene::initShaderStorage() {
	maxNodes = 20 * width * height;
	GLint nodeSize = 5 * sizeof(GLfloat) + 1 * sizeof(GLuint); // Size of LLNode

	// Atomic counter
	glGenBuffers(1, &counterBuffer);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, counterBuffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);

	// HeadPointer texture
	glGenTextures(1, &headPtrTex);
	glBindTexture(GL_TEXTURE_2D, headPtrTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, width, height);
	glBindImageTexture(0, headPtrTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

	// LinkedList Buffer
	glGenBuffers(1, &llBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, llBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxNodes * nodeSize, NULL, GL_DYNAMIC_DRAW);

	std::vector<GLuint> headPtrClearBuf(width*height, 0xffffffff);
	glGenBuffers(1, &clearBuf);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, clearBuf);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, headPtrClearBuf.size() * sizeof(GLuint), &headPtrClearBuf[0], GL_STATIC_COPY);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

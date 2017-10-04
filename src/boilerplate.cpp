// ==========================================================================
// An Object-Oriented Boilerplate Code for GLFW
//
// Author:  Kamyar Allahverdi, University of Calgary. Minor tweaks by Haysn Hornbeck.
// Date:    January 2017
// ==========================================================================


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <algorithm>    // std::reverse

#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#include <unistd.h>

// Get int max
#include <limits>

#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>



using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;

// Screen Height and Width
int SHEIGHT = 512;
int SWIDTH = 512;

// This is the base case representation of a Hilbert Curve
// Looks like:
//  _
// | |
//
vector<float> BaseVertex = {	
	-1.0, -1.0, 
	-1.0, 1.0,
	 1.0, 1.0,
	 1.0, -1.0
};

vector<float> BaseVertexTriangles = {	
	-1.0, -1.0,
	-1.0, 1.0,
	 1.0, 1.0,
	 1.0, -1.0
};


GLfloat g_color_buffer_data[999] = {
    1.0f,  0.0f,  0.0f, 1.0f,
    0.609f,  0.115f,  0.436f, 1.0f,
    0.327f,  0.483f,  0.844f, 1.0f,
    0.822f,  0.569f,  0.201f, 1.0f
};

// number of times to recurse
int n = 1;

// Generate color gradient
void genColor()
{
    int numberOfLines = (pow(2,n) * pow(2,n)*4);
	for (int i = 0; i < numberOfLines; i += 4)
	{
		g_color_buffer_data[i] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		g_color_buffer_data[i+1] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		g_color_buffer_data[i+2] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		g_color_buffer_data[i+3] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	}

}


// Callback for resizing the window
// Scales the curve to the window size... Somehow
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

class Program {
	GLuint vertex_shader;
	GLuint fragment_shader;
public:
	GLuint id;
	Program() {
		vertex_shader = 0;
		fragment_shader = 0;
		id = 0;
	}
	Program(string vertex_path, string fragment_path) {
		init(vertex_path, fragment_path);
	}
	void init(string vertex_path, string fragment_path) {
		id = glCreateProgram();
		vertex_shader = addShader(vertex_path, GL_VERTEX_SHADER);
		fragment_shader = addShader(fragment_path, GL_FRAGMENT_SHADER);
		if (vertex_shader)
			glAttachShader(id, vertex_shader);
		if (fragment_shader)
			glAttachShader(id, fragment_shader);

		glLinkProgram(id);
	}
	GLuint addShader(string path, GLuint type) {
		std::ifstream in(path);
		string buffer = [&in] {
			std::ostringstream ss {};
			ss << in.rdbuf();
			return ss.str();
		}();
		const char *buffer_array[] = { buffer.c_str() };

		GLuint shader = glCreateShader(type);

		glShaderSource(shader, 1, buffer_array, 0);
		glCompileShader(shader);

		// Compile results
		GLint status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) {
			GLint length;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			string info(length, ' ');
			glGetShaderInfoLog(shader, info.length(), &length, &info[0]);
			cerr << "ERROR compiling shader:" << endl << endl;
			cerr << info << endl;
		}
		return shader;
	}
	~Program() {
		glUseProgram(0);
		glDeleteProgram(id);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}
};

class VertexArray {
	std::map<string, GLuint> buffers;
	std::map<string, int> indices;
public:
	GLuint id;
	unsigned int count;
	VertexArray(int c) {
		glGenVertexArrays(1, &id);
		count = c;
	}

	VertexArray(const VertexArray &v) {
		glGenVertexArrays(1, &id);

		// Copy data from the old object
		this->indices = std::map<string, int>(v.indices);
		count = v.count;

		vector<GLuint> temp_buffers(v.buffers.size());

		// Allocate some temporary buffer object handles
		glGenBuffers(v.buffers.size(), &temp_buffers[0]);


		// Copy each old VBO into a new VBO
		int i = 0;
		for (auto &ent : v.buffers) {
			int size = 0;
			glBindBuffer(GL_ARRAY_BUFFER, ent.second);
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

			glBindBuffer(GL_COPY_READ_BUFFER, temp_buffers[i]);
			glBufferData(GL_COPY_READ_BUFFER, size, NULL, GL_STATIC_COPY);

			glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_READ_BUFFER, 0, 0,
					size);
			i++;
		}

		// Copy those temporary buffer objects into our VBOs
		GLuint colorbuffer;
		glGenBuffers(1, &colorbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
	
		i = 0;
		for (auto &ent : v.buffers) {
			GLuint buffer_id;
			int size = 0;
			int index = indices[ent.first];

			glGenBuffers(1, &buffer_id);

			glBindVertexArray(this->id);
			glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
			glBindBuffer(GL_COPY_READ_BUFFER, temp_buffers[i]);
			glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &size);

			// Allocate VBO memory and copy
			glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0,
					size);
			string indexs = ent.first;

			buffers[ent.first] = buffer_id;
			indices[ent.first] = index;

			// Setup the attributes
			size = size / (sizeof(float) * this->count);
			glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, 0, 0);

			
			// 2nd attribute buffer : colors
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
			glVertexAttribPointer(
				1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);


			glEnableVertexAttribArray(index);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			i++;
		}

		// Delete temporary buffers
		glDeleteBuffers(v.buffers.size(), &temp_buffers[0]);
	}

	void addBuffer(string name, int index, vector<float> buffer) {
		GLuint colorbuffer;
		glGenBuffers(1, &colorbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
	
		GLuint buffer_id;
		glBindVertexArray(id);

		glGenBuffers(1, &buffer_id);
		glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
		glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float),
				buffer.data(), GL_STATIC_DRAW);
		buffers[name] = buffer_id;
		indices[name] = index;

		int components = buffer.size() / count;
		glVertexAttribPointer(index, components, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(index);


		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);



		// unset states
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void updateBuffer(string name, vector<float> buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, buffers[name]);
		glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float),
				buffer.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	~VertexArray() {
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &id);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		for (auto &ent : buffers)
			glDeleteBuffers(1, &ent.second);
	}
};



void render(Program &program, VertexArray &va) {
	// clear screen to a dark grey colour
	
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//glLineWidth(4.0f);
	glPointSize(5.0f);

	glUseProgram(program.id);
	glBindVertexArray(va.id);
	glDrawArrays(GL_LINE_STRIP, 0, va.count);

	glBindVertexArray(0);
	glUseProgram(0);
}


void hilbertCalc()
{

	vector<float> finalVector = {}; // Store New Vertices


	glm::mat4 identityMatrix = glm::mat4(1.0f); // ID matrix

	// Scaling
	glm::mat4 scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.5 ,0.5 ,0.5));
	
	// Transformations for topLeft
	glm::mat4 translateTopLeftMatrix = glm::translate(identityMatrix, glm::vec3(-2.0f, 2.0f, 0.0f));
	
	// Transformations for TopRight
	glm::mat4 translateTopRightMatrix = glm::translate(identityMatrix, glm::vec3(2.0f, 2.0f, 0.0f));

	// Transformations for bottom left
	glm::mat4 translateBottomLeftMatrix = glm::translate(identityMatrix, glm::vec3(-2.0f, -2.0f, 0.0f));
	glm::mat4 rotateBottomLeftMatrix = glm::rotate(identityMatrix, glm::radians(270.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // rotate
	glm::mat4 flipBottomLeftMatrix = glm::scale(identityMatrix, glm::vec3(-1 ,1 ,1)); // flip in x Kurwa!
	
	// Transformations for bottom right
	glm::mat4 translateBottomRightMatrix = glm::translate(identityMatrix, glm::vec3(2.0f, -2.0f, 0.0f));
	glm::mat4 rotateBottomRightMatrix = glm::rotate(identityMatrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // rotate
	glm::mat4 flipBottomRightMatrix = glm::scale(identityMatrix, glm::vec3(-1 ,1 ,1)); // flip in x 
	
	// Apply transformations for the bottom Left
	// Push Result to finalVector
	for (unsigned int i = 0; i < BaseVertex.size() - 1; i = i + 2)
	{

		glm::vec4 BaseVertexVec4 = glm::vec4(BaseVertex[i] ,BaseVertex[i + 1], 0.0f, 1.0f);

		glm::mat4 transformBottomLeft = scaleMatrix * translateBottomLeftMatrix * rotateBottomLeftMatrix * flipBottomLeftMatrix;

		BaseVertexVec4 = transformBottomLeft * BaseVertexVec4;	

		finalVector.push_back(BaseVertexVec4.x);
		finalVector.push_back(BaseVertexVec4.y);
	}

	// Apply transformations for the Top Left
	// Push Result to finalVector
	for (unsigned int i = 0; i < BaseVertex.size() - 1; i = i + 2)
	{
		
		// BaseVertexVec4 is the vec4 representation of the BaseVertex array
		glm::vec4 BaseVertexVec4 = glm::vec4(BaseVertex[i] ,BaseVertex[i + 1], 0.0f, 1.0f);

		// Apply transformations to this Vec4: transformTopLeft
		glm::mat4 transformTopLeft = scaleMatrix * translateTopLeftMatrix;

		// Apply combined transformations in transformTopLeft to BaseVertexVec4
		BaseVertexVec4 = transformTopLeft * BaseVertexVec4;	

		// Push results to final Vector
		finalVector.push_back(BaseVertexVec4.x);
		finalVector.push_back(BaseVertexVec4.y);
	}

	// Apply transformations for the Top Right
	// Push Result to finalVector
	for (unsigned int i = 0; i < BaseVertex.size() - 1; i = i + 2)
	{

		glm::vec4 BaseVertexVec4 = glm::vec4(BaseVertex[i] ,BaseVertex[i + 1], 0.0f, 1.0f);

		glm::mat4 transformTopRight = scaleMatrix * translateTopRightMatrix;

		BaseVertexVec4 = transformTopRight * BaseVertexVec4;	

		finalVector.push_back(BaseVertexVec4.x);
		finalVector.push_back(BaseVertexVec4.y);
	}

	// Apply transformations for the bottom right
	// Push Result to finalVector
	for (unsigned int i = 0; i < BaseVertex.size() - 1; i = i + 2)
	{
		glm::vec4 BaseVertexVec4 = glm::vec4(BaseVertex[i] ,BaseVertex[i + 1], 0.0f, 1.0f);

		glm::mat4 transformBottomRight = scaleMatrix * translateBottomRightMatrix * rotateBottomRightMatrix * flipBottomRightMatrix;

		BaseVertexVec4 = transformBottomRight * BaseVertexVec4;	

		finalVector.push_back(BaseVertexVec4.x);
		finalVector.push_back(BaseVertexVec4.y);
	}

	//genColor();
	BaseVertex = finalVector;

}



// Pressing up
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
	{
		// Go up a level
		n += 1;
		hilbertCalc();

	} 
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		// Do nothing if n is already at 1
		if (n == 1)
		{
			printf( "%s\n", "At min value already.");
			return;
		}

		// Go Down a Level
		BaseVertex = {	
			-1.0, -1.0, 
			-1.0, 1.0,
			 1.0, 1.0,
			 1.0, -1.0
		};
		
		n -=1;
		
		for (int i = 0; i < n -1; i++)
		{
			hilbertCalc();
		}
	}
		
}



int main(int argc, char *argv[]) {
	// initialize the GLFW windowing system
	if (!glfwInit()) {
		cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
		return -1;
	}

	
	glfwSetErrorCallback([](int error, const char* description) {
		cout << "GLFW ERROR " << error << ":" << endl;
		cout << description << endl;
	});

	// Use OpenGL 4.1
	GLFWwindow *window = 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(SHEIGHT, SWIDTH, "CPSC 453 Assignment 1", 0, 0);
	if (!window) {
		cout << "Program failed to create GLFW window, TERMINATING" << endl;
		glfwTerminate();
		return -1;
	}

	glfwSetKeyCallback(window, key_callback);
	
	glViewport(0, 0, SHEIGHT, SWIDTH);

	glfwMakeContextCurrent(window);

	// Set callback for window size changes
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	Program p("data/vertex.glsl", "data/fragment.glsl");


	std::vector <glm::vec3> vertices;


	// run an event-triggered main loop
	while (!glfwWindowShouldClose(window)) {
		// render
		//render(p, baseCase);

		//usleep(300);
		// Number of vertexes needed = (2^n * 2^n)
		int numberOfVerticesNeeded = (pow(2,n) * pow(2,n));
	
		//printf( "%u\n", numberOfVerticesNeeded);
		
	
		VertexArray baseCase(numberOfVerticesNeeded);

		//genColor();
		
		//hilbertCalc();
		baseCase.addBuffer("v", 0, BaseVertex);


		render(p, baseCase);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	cout << "The End" << endl;
	return 0;
}

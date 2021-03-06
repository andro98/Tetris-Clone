#include<GL/glew.h>
#include <GLFW/glfw3.h>

#include<iostream>
#include<fstream>
#include<string>
#include<sstream>

#define ASSERT(x) if (!(x)) __debugbreak();
#define GlCall(x) GlClearError();\
 x;\
ASSERT(GlLogCall(#x, __FILE__, __LINE__))

static void GlClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

static bool GlLogCall(const char* function, const char*  file, int line)
{
	while (GLenum error = glGetError()) 
	{
		std::cout << "[OpenGL Error] (" << error <<"): "<< function<<
			" "<<file <<":" << line<<std::endl;
		return false;
	}
	return true;
}
struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
	std::fstream stream(filepath);

	enum class ShaderType {
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};
	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
			{
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos)
			{
				type = ShaderType::FRAGMENT;
			}
		}
		else
		{
			ss[(int)type] << line << "\n";
		}
	}

	return{ ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	GlCall(glShaderSource(id, 1, &src, nullptr));
	GlCall(glCompileShader(id));

	//Error Handling
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		GlCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		// alloca allows to allocate on the stack dynamically
		char* message = (char*)alloca(length * sizeof(char));
		GlCall(glGetShaderInfoLog(id, length, &length, message));
		std::cout << "Faild to compile" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
		std::cout << message << std::endl;
		GlCall(glDeleteShader(id));
		return 0;
	}
	return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	GlCall(glAttachShader(program, vs));
	GlCall(glAttachShader(program, fs));
	GlCall(glLinkProgram(program));
	GlCall(glValidateProgram(program));

	GlCall(glDeleteShader(vs));
	GlCall(glDeleteShader(fs));

	return program;
}

void MoveLeft(float* positions, int size, int dimension, float speed = 0.1)
{
	for (int i = 0; i < size; i += dimension)
	{
		positions[i] += speed;
	}
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Tetris", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK) {
		std::cout << "Error!" << std::endl;
	}

	std::cout << glGetString(GL_VERSION) << std::endl;

	float positions[] = {
		-0.5f, -0.5f,
		 0.5f, -0.5f,
		 0.5f, 0.5f,
		-0.5f, 0.5f
	};

	unsigned int indecies[] = {
		0, 1, 2,
		2, 3, 0
	};

	unsigned int buffer;
	GlCall(glGenBuffers(1, &buffer));
	GlCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
	GlCall(glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW));

	GlCall(glEnableVertexAttribArray(0));
	GlCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

	unsigned int ibo;
	GlCall(glGenBuffers(1, &ibo));
	GlCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
	GlCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indecies, GL_STATIC_DRAW));

	ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");

	unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
	GlCall(glUseProgram(shader));

	GlCall(int location = glGetUniformLocation(shader, "u_Color"));
	ASSERT(location != -1);
	GlCall(glUniform4f(location, 0.5f, 0.3f, 0.8f,	1.0f));

	float r = 0.0f;
	float increment = 0.05f;
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		GlCall(glClear(GL_COLOR_BUFFER_BIT));

		GlCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));
		GlCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
		//MoveLeft(positions, 6, 2);

		if (r > 1.0f)
			increment = -0.05f;
		else if (r < 0.0f)
			increment = 0.05f;
		r += increment;

		/* Swap front and back buffers */
		GlCall(glfwSwapBuffers(window));

		/* Poll for and process events */
		GlCall(glfwPollEvents());
	}

	glDeleteProgram(shader);

	glfwTerminate();
	return 0;
}
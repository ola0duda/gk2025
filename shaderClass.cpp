#include "shaderClass.h"
#include <glm/gtc/type_ptr.hpp>

std::string get_file_contents(const char* filename)
{
	std::ifstream in(filename, std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		if (in.tellg() == -1) { //sprawdzenie b³êdu dla tellg
			std::cerr << "B£¥D: Nie mo¿na pobraæ rozmiaru pliku '" << filename << "'!" << std::endl;
			throw std::runtime_error("B³¹d pobierania rozmiaru pliku dla: " + std::string(filename));
		}
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	std::cerr << "B£¥D: Nie mo¿na odczytaæ pliku '" << filename << "'! errno: " << errno << std::endl;
	throw std::runtime_error("B³¹d odczytu pliku: " + std::string(filename));
}

Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
	ID = 0; //inicjalizowanie ID na 0 na wypadek b³êdu
	std::string vertexCode;
	std::string fragmentCode;
	try {
		vertexCode = get_file_contents(vertexFile);
		fragmentCode = get_file_contents(fragmentFile);
	}
	catch (const std::runtime_error& e) {
		std::cerr << "B£¥D::SHADER: Nie uda³o siê wczytaæ plików shadera: " << e.what() << std::endl;
		return; //przerwanie konstruktora, jeœli nie mo¿na wczytaæ plików
	}

	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	compileErrors(vertexShader, "VERTEX");

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	compileErrors(fragmentShader, "FRAGMENT");

	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);
	compileErrors(ID, "PROGRAM");

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void Shader::Activate()
{
	if (ID == 0) {
		//std::cerr << "aktywacja nieprawid³owego shadera (ID=0)" << std::endl;
		return;
	}
	glUseProgram(ID);
}

void Shader::Delete()
{
	if (ID == 0) return;
	glDeleteProgram(ID);
	ID = 0;
}

void Shader::compileErrors(unsigned int shader, const char* type)
{
	GLint hasCompiled;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cerr << "B£¥D_KOMPILACJI_SHADERA dla:" << type << "\n" << infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cerr << "B£¥D_LINKOWANIA_SHADERA dla:" << type << "\n" << infoLog << std::endl;
		}
	}
}

//implementacje funkcji setUniform
void Shader::setBool(const std::string& name, bool value) const
{
	if (ID == 0) return; glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string& name, int value) const
{
	if (ID == 0) return; glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string& name, float value) const
{
	if (ID == 0) return; glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
	if (ID == 0) return; glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const std::string& name, float x, float y) const
{
	if (ID == 0) return; glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
	if (ID == 0) return; glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
	if (ID == 0) return; glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
	if (ID == 0) return; glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
	if (ID == 0) return; glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}
void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
	if (ID == 0) return; glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
	if (ID == 0) return; glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
	if (ID == 0) return; glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
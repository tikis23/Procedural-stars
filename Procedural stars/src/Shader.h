#pragma once
#include <GL/glew.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

class Shader {
public:
	Shader() {
		id = glCreateProgram();
	}
	~Shader() {
		glDeleteProgram(id);
	}
	void Use() {
		glUseProgram(id);
	}
	// shaderType :
	// GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER, GL_COMPUTE_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER.
	void Load(const char* path, GLenum shaderType) {
		std::string code;
		std::ifstream codeFile;
		codeFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		//load
		try {
			std::stringstream shaderStream;

			codeFile.open(path);
			shaderStream << codeFile.rdbuf();
			codeFile.close();
			code = shaderStream.str();
		}
		catch (std::ifstream::failure e) {
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ AT " << path << std::endl;
		}
		const char* cCode = code.c_str();

		unsigned int shader;

		//compile
		shader = glCreateShader(shaderType);
		glShaderSource(shader, 1, &cCode, NULL);
		glCompileShader(shader);
		CheckCompileErrors(shader, 1);
		glAttachShader(id, shader);
		glDeleteShader(shader);
	}
	void Link() {
		glLinkProgram(id);
		CheckCompileErrors(id, 0);
	}
	unsigned int Id() {
		return id;
	}

	// float
	void uniform1f(const std::string& location, float value_1) const {
		glUniform1f(glGetUniformLocation(id, location.c_str()), value_1);
	}
	void uniform2f(const std::string& location, float value_1, float value_2) const {
		glUniform2f(glGetUniformLocation(id, location.c_str()), value_1, value_2);
	}
	void uniform3f(const std::string& location, float value_1, float value_2, float value_3) const {
		glUniform3f(glGetUniformLocation(id, location.c_str()), value_1, value_2, value_3);
	}
	void uniform4f(const std::string& location, float value_1, float value_2, float value_3, float value_4) const {
		glUniform4f(glGetUniformLocation(id, location.c_str()), value_1, value_2, value_3, value_4);
	}

	// int
	void uniform1i(const std::string& location, int value_1) const {
		glUniform1i(glGetUniformLocation(id, location.c_str()), value_1);
	}
	void uniform2i(const std::string& location, int value_1, int value_2) const {
		glUniform2i(glGetUniformLocation(id, location.c_str()), value_1, value_2);
	}
	void uniform3i(const std::string& location, int value_1, int value_2, int value_3) const {
		glUniform3i(glGetUniformLocation(id, location.c_str()), value_1, value_2, value_3);
	}
	void uniform4i(const std::string& location, int value_1, int value_2, int value_3, int value_4) const {
		glUniform4i(glGetUniformLocation(id, location.c_str()), value_1, value_2, value_3, value_4);
	}

	// arrays
	void uniformArrf(const std::string& location, int size, float value[]) const {
		glUniform1fv(glGetUniformLocation(id, location.c_str()), size, value);
	}
	void uniformArri(const std::string& location, int size, int value[]) const {
		glUniform1iv(glGetUniformLocation(id, location.c_str()), size, value);
	}

	// matrix
	void uniformMatrix4(const std::string& location, float value[]) const {
		glUniformMatrix4fv(glGetUniformLocation(id, location.c_str()), 1, false, value);
	}

private:
	unsigned int id;
	void CheckCompileErrors(unsigned int shader, int type) {
		int success;
		char infoLog[1024];
		if (type == 1) {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);

				std::cout << "ERROR::SHADER_COMPILATION_ERROR" << "\n" << infoLog <<
					"\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else {
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);

				std::cout << "ERROR::PROGRAM_LINKING_ERROR" << "\n" << infoLog <<
					"\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};
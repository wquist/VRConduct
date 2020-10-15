#include <heatsink/shader.hpp>

#include <fstream>

hs::shader::stage::name hs::shader::stage::deduce_name(const std::string& path)
{
	// Specify desired extensions names here
	const std::unordered_map<std::string, name> extensions =
	{
		{"vert", name::vert},
		{"frag", name::frag},
		{"tesc", name::tesc},
		{"tese", name::tese},
		{"geom", name::geom},
		{"comp", name::comp},
	};

	/* This could cause unexpected behavior if the passed file has
	* no extension and a linux style directory "." is used elsewhere -
	* however, in this case, nothing changes and it will still be an
	* error in the next check
	*/
	auto ext_pos = path.find_last_of(".");
	assert(ext_pos != std::string::npos);

	auto ext = path.substr(ext_pos + 1);
	return extensions.at(ext);
}

GLenum hs::shader::stage::to_kind(name n)
{
	const std::unordered_map<name, GLenum> kinds =
	{
		{name::vert, GL_VERTEX_SHADER},
		{name::frag, GL_FRAGMENT_SHADER},
		{name::tesc, GL_TESS_CONTROL_SHADER},
		{name::tese, GL_TESS_EVALUATION_SHADER},
		{name::geom, GL_GEOMETRY_SHADER},
		{name::comp, GL_COMPUTE_SHADER}
	};

	return kinds.at(n);
}

hs::shader::stage::stage(const std::string& path, name n)
	: kind{to_kind(n)}
{
	std::ifstream stream(path.c_str());
	assert(stream.is_open());

	std::istreambuf_iterator<char> start{stream};
	std::istreambuf_iterator<char> end{};

	source = std::string(start, end);
	stream.close();
}

std::string hs::shader::get_shader_error(GLuint id)
{
	GLint message_length;
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &message_length);

	assert(message_length);
	std::vector<char> message(message_length + 1);

	glGetShaderInfoLog(id, message_length, nullptr, message.data());
	return std::string(message.begin(), message.end());
}

std::string hs::shader::get_program_error(GLuint id)
{
	GLint message_length;
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &message_length);

	assert(message_length);
	std::vector<char> message(message_length + 1);

	glGetProgramInfoLog(id, message_length, nullptr, message.data());
	return std::string(message.begin(), message.end());
}

hs::shader::shader(context& c, const std::vector<stage>& stages)
	: context::identifier<GL_PROGRAM>(c)
{
	std::vector<GLuint> shaders;
	for (auto& s : stages)
	{
		auto shader = glCreateShader(s.kind);
		assert(shader);

		const auto* raw = s.source.c_str();
		glShaderSource(shader, 1, &raw, nullptr);
		glCompileShader(shader);

		GLint result;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
		assert(result == GL_TRUE);

		shaders.push_back(shader);
	}

	for (const auto& s : shaders)
		glAttachShader(this->m_name, s);

	glLinkProgram(this->m_name);

	GLint result;
	glGetProgramiv(this->m_name, GL_LINK_STATUS, &result);
	assert(result == GL_TRUE);

	/* The shaders could be stored and shared with other programs in a
	* more complicated build system, but just delete them once they are
	* no longer needed, for now.
	*/
	for (auto& s : shaders)
	{
		glDetachShader(this->m_name, s);
		glDeleteShader(s);
	}
}

void hs::shader::set_uniform_raw(GLuint i, GLenum type, const void* p) const
{
	bind();
	switch (type)
	{
		case GL_INT:
			glUniform1iv(i, 1, static_cast<const GLint*>(p));
			break;
		case GL_FLOAT:
			glUniform1fv(i, 1, static_cast<const GLfloat*>(p));
			break;
		case GL_FLOAT_VEC2:
			glUniform2fv(i, 1, static_cast<const GLfloat*>(p));
			break;
		case GL_FLOAT_VEC3:
			glUniform3fv(i, 1, static_cast<const GLfloat*>(p));
			break;
		case GL_FLOAT_VEC4:
			glUniform4fv(i, 1, static_cast<const GLfloat*>(p));
			break;
		case GL_FLOAT_MAT4:
			glUniformMatrix4fv(i, 1, GL_FALSE, static_cast<const GLfloat*>(p));
			break;
	}
}

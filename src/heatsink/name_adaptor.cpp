#include <heatsink/name_adaptor.hpp>

#include <cassert>

namespace hs
{
	template<>
	GLuint name_adaptor<GL_BUFFER>::create()
	{
		GLuint name;
		glGenBuffers(1, &name);
		assert(name);

		return name;
	}

	template<>
	void name_adaptor<GL_BUFFER>::destroy(GLuint name)
	{
		glDeleteBuffers(1, &name);
	}

	template<>
	void name_adaptor<GL_BUFFER>::bind(GLuint name, GLenum mode)
	{
		glBindBuffer(mode, name);
	}

	template<>
	GLuint name_adaptor<GL_FRAMEBUFFER>::create()
	{
		GLuint name;
		glGenFramebuffers(1, &name);
		assert(name);

		return name;
	}

	template<>
	void name_adaptor<GL_FRAMEBUFFER>::destroy(GLuint name)
	{
		glDeleteFramebuffers(1, &name);
	}

	template<>
	void name_adaptor<GL_FRAMEBUFFER>::bind(GLuint name, GLenum mode)
	{
		glBindFramebuffer(mode, name);
	}

	template<>
	GLuint name_adaptor<GL_SAMPLER>::create()
	{
		GLuint name;
		glGenSamplers(1, &name);
		assert(name);

		return name;
	}

	template<>
	void name_adaptor<GL_SAMPLER>::destroy(GLuint name)
	{
		glDeleteSamplers(1, &name);
	}

	template<>
	void name_adaptor<GL_SAMPLER>::bind(GLuint name, size_t unit)
	{
		glBindSampler(unit, name);
	}

	template<>
	GLuint name_adaptor<GL_PROGRAM>::create()
	{
		GLuint name{glCreateProgram()};
		assert(name);

		return name;
	}

	template<>
	void name_adaptor<GL_PROGRAM>::destroy(GLuint name)
	{
		glDeleteProgram(name);
	}

	template<>
	void name_adaptor<GL_PROGRAM>::bind(GLuint name)
	{
		glUseProgram(name);
	}

	template<>
	GLuint name_adaptor<GL_TEXTURE>::create()
	{
		GLuint name;
		glGenTextures(1, &name);
		assert(name);

		return name;
	}

	template<>
	void name_adaptor<GL_TEXTURE>::destroy(GLuint name)
	{
		glDeleteTextures(1, &name);
	}

	template<>
	void name_adaptor<GL_TEXTURE>::bind(GLuint name, GLenum mode, size_t unit)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(mode, name);
	}

	template<>
	GLuint name_adaptor<GL_VERTEX_ARRAY>::create()
	{
		GLuint name;
		glGenVertexArrays(1, &name);
		assert(name);

		return name;
	}

	template<>
	void name_adaptor<GL_VERTEX_ARRAY>::destroy(GLuint name)
	{
		glDeleteVertexArrays(1, &name);
	}

	template<>
	void name_adaptor<GL_VERTEX_ARRAY>::bind(GLuint name)
	{
		glBindVertexArray(name);
	}
}


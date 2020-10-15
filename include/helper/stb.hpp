#pragma once

#include <map>
#include <string>
#include <vector>

#include <GL/glew.h>

#include <glm/glm.hpp>

namespace helper
{
	struct stb_image
	{
	public:
		stb_image(const std::string& filename, GLenum format = GL_NONE);
		~stb_image();

	public:
		glm::uvec2 size() const { return glm::uvec2(width, height); }
		size_t data_size() const { return (width * height * component_size); }

	public:
		unsigned char* data;

		size_t width;
		size_t height;
		size_t component_size;
	};
}
#pragma once

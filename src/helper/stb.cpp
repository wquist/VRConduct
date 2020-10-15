#include <helper/stb.hpp>

#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

helper::stb_image::stb_image(const std::string& filename, GLenum format)
{
	int w, h, bpp;

	size_t cs = ~0;
	switch (format)
	{
		case GL_NONE: cs = 0; break;
		case GL_RED:  cs = 1; break;
		case GL_RG:   cs = 2; break;
		case GL_RGB:  cs = 3; break;
		case GL_RGBA: cs = 4; break;
	}
	assert(cs != size_t(~0));

	data = stbi_load(filename.c_str(), &w, &h, &bpp, cs);
	assert(data);

	width = static_cast<size_t>(w);
	height = static_cast<size_t>(h);
	component_size = static_cast<size_t>((format == GL_NONE) ? bpp : cs);
}

helper::stb_image::~stb_image()
{
	stbi_image_free(static_cast<void*>(data));
}

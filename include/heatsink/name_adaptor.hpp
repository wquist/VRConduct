#pragma once

#include <cstdlib>

#include <GL/glew.h>

namespace hs
{
	template<GLenum>
	struct name_adaptor
	{
		// Call the appropriate glCreate*() function
		static GLuint create();
		// Call the appropriate glDestroy*() function
		static void destroy(GLuint name);

		// A name may require a mode or a unit
		static void bind(GLuint name);
		static void bind(GLuint name, GLenum mode);
		static void bind(GLuint name, size_t unit);
		static void bind(GLuint name, GLenum mode, size_t unit);
	};
}

#pragma once

#include <type_traits>

#include <GL/glew.h>

namespace hs
{
	// Defined if the GL type requires another enum specified when binding
	template<GLenum>
	struct has_bind_mode : std::false_type {};

	// Defined if the GL type requires a "slot" index when binding
	template<GLenum>
	struct has_bind_unit : std::false_type {};
}

namespace hs
{
	template<>
	struct has_bind_mode<GL_BUFFER> : std::true_type {};

	template<>
	struct has_bind_mode<GL_FRAMEBUFFER> : std::true_type {};

	template<>
	struct has_bind_mode<GL_TEXTURE> : std::true_type {};

	template<>
	struct has_bind_unit<GL_SAMPLER> : std::true_type {};

	template<>
	struct has_bind_unit<GL_TEXTURE> : std::true_type {};
}

#pragma once

#include <type_traits>

#include <GL/glew.h>

namespace hs
{
	// Convert a type to a GL enumeration, or GL_NONE if one does not exist
	template<class T>
	struct make_enum;

	// Convert a GL enumeration to a respresentative type, or void if one does not exist
	template<GLenum T>
	struct make_type;
	template<GLenum T>
	using make_type_t = typename make_type<T>::type;

	inline size_t enum_sizeof(GLenum type);

	// Determine if a GL enumeration type is "opaque" in GLSL
	// When a type is opaque, the uniform value is set as a GLint
	inline bool is_opaque(GLenum type);

	inline GLenum remove_size(GLenum format);
	inline size_t enum_rank(GLenum format);
}

namespace hs
{
	template<class T>
	struct make_enum : std::integral_constant<GLenum, GL_NONE> {};

	template<>
	struct make_enum<GLbyte> : std::integral_constant<GLenum, GL_BYTE> {};
	template<>
	struct make_enum<GLshort> : std::integral_constant<GLenum, GL_SHORT> {};
	template<>
	struct make_enum<GLint> : std::integral_constant<GLenum, GL_INT> {};

	template<>
	struct make_enum<GLubyte> : std::integral_constant<GLenum, GL_UNSIGNED_BYTE> {};
	template<>
	struct make_enum<GLushort> : std::integral_constant<GLenum, GL_UNSIGNED_SHORT> {};
	template<>
	struct make_enum<GLuint> : std::integral_constant<GLenum, GL_UNSIGNED_INT> {};

	template<>
	struct make_enum<GLfloat> : std::integral_constant<GLenum, GL_FLOAT> {};
	template<>
	struct make_enum<GLdouble> : std::integral_constant<GLenum, GL_DOUBLE> {};

	template<>
	struct make_enum<GLfloat[2]> : std::integral_constant<GLenum, GL_FLOAT_VEC2> {};
	template<>
	struct make_enum<GLfloat[3]> : std::integral_constant<GLenum, GL_FLOAT_VEC3> {};
	template<>
	struct make_enum<GLfloat[4]> : std::integral_constant<GLenum, GL_FLOAT_VEC4> {};

	template<>
	struct make_enum<GLfloat[3][3]> : std::integral_constant<GLenum, GL_FLOAT_MAT3> {};
	template<>
	struct make_enum<GLfloat[4][4]> : std::integral_constant<GLenum, GL_FLOAT_MAT4> {};

	template<GLenum T>
	struct make_type { using type = void; };

	template<>
	struct make_type<GL_BYTE> { using type = GLbyte; };
	template<>
	struct make_type<GL_SHORT> { using type = GLshort; };
	template<>
	struct make_type<GL_INT> { using type = GLint; };

	template<>
	struct make_type<GL_UNSIGNED_BYTE> { using type = GLubyte; };
	template<>
	struct make_type<GL_UNSIGNED_SHORT> { using type = GLushort; };
	template<>
	struct make_type<GL_UNSIGNED_INT> { using type = GLuint; };

	template<>
	struct make_type<GL_FLOAT> { using type = GLfloat; };
	template<>
	struct make_type<GL_DOUBLE> { using type = GLdouble; };

	template<>
	struct make_type<GL_FLOAT_VEC2> { using type = GLfloat[2]; };
	template<>
	struct make_type<GL_FLOAT_VEC3> { using type = GLfloat[3]; };
	template<>
	struct make_type<GL_FLOAT_VEC4> { using type = GLfloat[4]; };

	template<>
	struct make_type<GL_FLOAT_MAT4> { using type = GLfloat[4][4]; };

	inline size_t enum_sizeof(GLenum type)
	{
		switch (type)
		{
			case GL_BYTE:
			case GL_UNSIGNED_BYTE:
				return 1;
			case GL_SHORT:
			case GL_UNSIGNED_SHORT:
				return 2;
			case GL_INT:
			case GL_UNSIGNED_INT:
				return 4;

			case GL_HALF_FLOAT:
				return 2;
			case GL_FLOAT:
				return 4;
			case GL_DOUBLE:
				return 8;

			default:
				return 0;
		}
	}

	inline bool is_opaque(GLenum type)
	{
		switch (type)
		{
			case GL_SAMPLER_1D:
				return true;
			case GL_SAMPLER_2D:
				return true;
			case GL_SAMPLER_3D:
				return true;

			case GL_SAMPLER_CUBE:
				return true;

			default:
				return false;
		}
	}

	inline GLenum remove_size(GLenum format)
	{
		switch (format)
		{
			case GL_RED:
			case GL_R8:
			case GL_R8_SNORM:
			case GL_R16:
			case GL_R16_SNORM:
			case GL_R16F:
			case GL_R32F:
			case GL_R8I:
			case GL_R8UI:
			case GL_R16I:
			case GL_R16UI:
			case GL_R32I:
			case GL_R32UI:
				return GL_RED;

			case GL_RG:
			case GL_RG8:
			case GL_RG8_SNORM:
			case GL_RG16:
			case GL_RG16_SNORM:
			case GL_RG16F:
			case GL_RG32F:
			case GL_RG8I:
			case GL_RG8UI:
			case GL_RG16I:
			case GL_RG16UI:
			case GL_RG32I:
			case GL_RG32UI:
				return GL_RG;

			case GL_RGB:
			case GL_R3_G3_B2:
			case GL_RGB4:
			case GL_RGB5:
			case GL_RGB8:
			case GL_RGB8_SNORM:
			case GL_RGB10:
			case GL_RGB12:
			case GL_RGB16_SNORM:
			case GL_RGBA2:
			case GL_RGBA4:
			case GL_SRGB8:
			case GL_RGB16F:
			case GL_RGB32F:
			case GL_R11F_G11F_B10F:
			case GL_RGB9_E5:
			case GL_RGB8I:
			case GL_RGB8UI:
			case GL_RGB16I:
			case GL_RGB16UI:
			case GL_RGB32I:
			case GL_RGB32UI:
				return GL_RGB;

			case GL_RGBA:
			case GL_RGB5_A1:
			case GL_RGBA8:
			case GL_RGBA8_SNORM:
			case GL_RGB10_A2:
			case GL_RGB10_A2UI:
			case GL_RGBA12:
			case GL_RGBA16:
			case GL_SRGB8_ALPHA8:
			case GL_RGBA16F:
			case GL_RGBA32F:
			case GL_RGBA8I:
			case GL_RGBA8UI:
			case GL_RGBA16I:
			case GL_RGBA16UI:
			case GL_RGBA32I:
			case GL_RGBA32UI:
				return GL_RGBA;

			case GL_DEPTH_COMPONENT:
			case GL_DEPTH_COMPONENT16:
			case GL_DEPTH_COMPONENT24:
			case GL_DEPTH_COMPONENT32:
			case GL_DEPTH_COMPONENT32F:
				return GL_DEPTH_COMPONENT;

			case GL_DEPTH_STENCIL:
			case GL_DEPTH24_STENCIL8:
			case GL_DEPTH32F_STENCIL8:
				return GL_DEPTH_STENCIL;

			default:
				return GL_NONE;
		}
	}

	inline size_t enum_rank(GLenum format)
	{
		switch (format)
		{
			case GL_RED:
			case GL_RED_INTEGER:
			case GL_DEPTH_COMPONENT:
				return 1;
			case GL_RG:
			case GL_RG_INTEGER:
			case GL_DEPTH_STENCIL:
				return 2;
			case GL_RGB:
			case GL_BGR:
			case GL_RGB_INTEGER:
			case GL_BGR_INTEGER:
				return 3;
			case GL_RGBA:
			case GL_BGRA:
			case GL_RGBA_INTEGER:
			case GL_BGRA_INTEGER:
				return 4;

			default:
				return 0;
		}
	}
}

#pragma once

#include <GL/glew.h>
#include "name_adaptor.hpp"
#include "name_traits.hpp"

namespace hs
{
	// Maintain the parameters needed to bind an OpenGL type
	// Can vary between types; may need a bind mode or bind unit
	template<GLenum T, bool M = has_bind_mode<T>::value, bool U = has_bind_unit<T>::value>
	class bind_state
	{
	public:
		// Only defined if U is false
		void bind();
		// Only defined if U is true
		void bind_to(size_t unit);
	};
}

namespace hs
{
	template<GLenum T>
	class bind_state<T, true, true>
	{
	public:
		bind_state(GLuint name, GLenum mode)
			: m_name{name}, m_mode{mode}
		{
		}

	public:
		void bind_to(size_t unit) const { name_adaptor<T>::bind(m_name, m_mode, unit); }

	protected:
		GLuint m_name;
		GLenum m_mode;
	};

	template<GLenum T>
	class bind_state<T, true, false>
	{
	public:
		bind_state(GLuint name, GLenum mode)
			: m_name{name}, m_mode{mode}
		{
		}

	public:
		void bind() const { name_adaptor<T>::bind(m_name, m_mode); }

	protected:
		GLuint m_name;
		GLenum m_mode;
	};

	template<GLenum T>
	class bind_state<T, false, true>
	{
	public:
		bind_state(GLuint name)
			: m_name{name}
		{
		}

	public:
		void bind_to(size_t unit) const { name_adaptor<T>::bind(m_name, unit); }

	protected:
		GLuint m_name;
	};

	template<GLenum T>
	class bind_state<T, false, false>
	{
	public:
		bind_state(GLuint name)
			: m_name{name}
		{
		}

	public:
		void bind() const { name_adaptor<T>::bind(m_name); }

	protected:
		GLuint m_name;
	};
}

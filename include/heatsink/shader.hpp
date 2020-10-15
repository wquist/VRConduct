#pragma once

#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>

#include <GL/glew.h>
#include "context.hpp"
#include "component_traits.hpp"
#include "enum_traits.hpp"

namespace hs
{
	// Manages the state of an OpenGL shader program
	// The class is called shader, but is actually what OpenGL calls a "program"
	class shader : public context::identifier<GL_PROGRAM>
	{
	public:
		// Represents a single compilation unit of an OpenGL shader program
		// What OpenGL actually refers to as a "shader"
		struct stage
		{
		public:
			// The available types of shaders
			// These names are equivalent to the expected shader file extensions
			enum class name { vert, frag, tesc, tese, geom, comp };

		private:
			// Use the file extension to get the name from just the filepath
			static name deduce_name(const std::string& path);
			// Convert a name to its GL_ analog (eg, vert to GL_VERTEX_SHADER)
			static GLenum to_kind(name n);

		public:
			// Load the shader source from the file path and store it
			stage(const std::string& path)
				: stage(path, deduce_name(path))
			{
			}

			// Specify the shader type; the file extension will not be checked
			stage(const std::string& path, name n);

		public:
			std::string source;
			GLenum kind;
		};

	private:
		struct uniform
		{
			GLenum type;
			GLuint location;
		};

	private:
		// Should only be called when GL_COMPILE_STATUS is not true
		static std::string get_shader_error(GLuint id);
		// Should only be called when GL_LINK_STATUS is not true
		static std::string get_program_error(GLuint id);

	public:
		/* Stages can be implicitly constructed from std::string, but to create
		* a vector of them, the literal operator "..."s from
		* std::string_literals to allow for implicit construction
		*/
		shader(context& c, const std::vector<stage>& stages);

	public:
		template<typename T>
		void declare_uniform(const std::string& name);

		/* Two separate functions must be used, rather than providing a
		* default initialization for "T value" since a shader uniform
		* may be set to a default value - always setting the uniform
		* to a value of T could override that default
		*/
		template<typename T>
		void declare_uniform(const std::string& name, const T& value);

		/* Uniform functions must be defined as individual specializations,
		* as each glUniform* function is too different to be combined
		*/
		template<typename T>
		void set_uniform(const std::string& name, const T& v) const;

	private:
		void set_uniform_raw(GLuint i, GLenum type, const void* p) const;

	private:
		std::unordered_map<std::string, uniform> m_uniforms;
	};
}

namespace hs
{
	template<class T>
	void shader::declare_uniform(const std::string& name)
	{
		GLint loc{glGetUniformLocation(this->m_name, name.c_str())};
		assert(loc != -1);

		auto uloc = static_cast<GLuint>(loc);
		auto utype = make_enum<component_decay_t<T>>{};

		/* Uniform locations and uniform indices are not the same thing; every name
		* has an index, but it may not have a location - therefore, the index
		* must be retrieved separately to pass to glGetActiveUniform
		*/
		GLuint index;
		auto* raw = name.c_str();
		glGetUniformIndices(this->m_name, 1, &raw, &index);

		GLint size;
		GLenum type;
		glGetActiveUniform(this->m_name, index, 0, nullptr, &size, &type, nullptr);

		// FIXME: uniform array types are not yet supported
		assert(size == 1);
		// Opaque types have their own enum, but are set as GLints
		assert(is_opaque(type) ? utype == GL_INT : utype == type);

		m_uniforms.emplace(name, uniform{utype, uloc});
	}

	template<class T>
	void shader::declare_uniform(const std::string& name, const T& value)
	{
		declare_uniform<T>(name);
		set_uniform(name, value);
	}

	template<class T>
	void shader::set_uniform(const std::string& name, const T& v) const
	{
		const auto& u = m_uniforms.at(name);
		assert(u.type == make_enum<component_decay_t<T>>{});

		// FIXME: not the best way to acquire a pointer to the data
		set_uniform_raw(u.location, u.type, reinterpret_cast<const void*>(&v));
	}
}

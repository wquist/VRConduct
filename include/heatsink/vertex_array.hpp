#pragma once

#include <cassert>

#include <GL/glew.h>
#include "context.hpp"
#include "array_buffer.hpp"

namespace hs
{
	class vertex_array : public context::identifier<GL_VERTEX_ARRAY>
	{
	public:
		struct attribute
		{
		public:
			// Provide a single-argument constructor to allow implicit construction
			attribute(size_t i)
				: attribute(i, false)
			{
			}
			// Component will be automatically inferred if GL_NONE
			attribute(size_t i, bool n, GLenum c = GL_NONE)
				: index{i}, normalize{n}, component{c}
			{
			}

		public:
			size_t index;
			bool normalize;
			GLenum component;
		};

	public:
		vertex_array(context& c)
			: context::identifier<GL_VERTEX_ARRAY>(c) {}

	public:
		// Set part of an array_buffer as an attribute
		// Member pointer class must be same as what was stored in the buffer
		template<class T, class C>
		void set_attribute(attribute a, const array_buffer& b, T C::*field);
		// Set an entire array_buffer as an attribute
		void set_attribute(attribute a, const array_buffer& b);

		// Set an array_buffer as the indexing attribute
		void set_elements(const array_buffer& b);

		// Return indexing mode if set_elements has been called, or GL_NONE
		GLenum indexing() const { return m_indexing; }

	private:
		GLenum m_indexing;
	};
}

namespace hs
{
	template<class T, class C>
	void vertex_array::set_attribute(attribute a, const array_buffer& b, T C::*field)
	{
		assert(b.mode() == GL_ARRAY_BUFFER);
		assert(b.get_type() == std::type_index(typeid(C)));

		if (a.component == GL_NONE)
			a.component = make_enum<remove_all_component_extents_t<T>>{};

		// Component must be specified if it cannot be inferred
		assert(a.component != GL_NONE);

		auto component_size = enum_sizeof(a.component);
		auto size = sizeof(T) / component_size;
		// FIXME: support for sequential matrix attributes
		assert(size <= 4);

		/* Dereferencing a null pointer is undefined behavior, but it will work
		* here - C should be a POD type, so the null pointer dereference is
		* working in the same vein as offsetof(), just with a member pointer
		*/
		// FIXME: Could be made compliant by specifying a temporary C?
		auto offset = (GLvoid*)((char*)&(((C*)nullptr)->*field) - (char*)nullptr);

		bind();
		glEnableVertexAttribArray(a.index);

		b.bind();
		glVertexAttribPointer(a.index, size, a.component, a.normalize, sizeof(C), offset);
	}
}

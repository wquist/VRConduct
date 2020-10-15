#pragma once

#include <iterator>
#include <typeindex>

#include <GL/glew.h>
#include "context.hpp"
#include "component_traits.hpp"
#include "enum_traits.hpp"

namespace hs
{
	class array_buffer : public context::identifier<GL_BUFFER>
	{
	public:
		// Create a new array_buffer with no data
		// Needed because constructor template arguments cannot be explicit
		template<class T>
		static array_buffer empty(context& c, GLenum mode, GLenum usage);

	public:
		// Buffers constructed with data always have GL_STATIC_DRAW usage
		template<class Iterator>
		array_buffer(context& c, GLenum mode, Iterator begin, Iterator end);
		template<class Container>
		array_buffer(context& c, GLenum mode, Container data)
			: array_buffer(c, mode, std::begin(data), std::end(data)) {}

	private:
		// Utility for array_buffer::empty
		// Create a type-annotated array_buffer without specifying valid data
		template<class T>
		array_buffer(context& c, GLenum mode, GLenum usage, T*);

	public:
		template<class Iterator>
		void update(Iterator begin, Iterator end);
		template<class Container>
		void update(Container c) { update(std::begin(c), std::end(c)); }

		size_t size() const { return m_size; }
		size_t stride() const { return m_stride; }
		GLenum usage() const { return m_usage; }

		const std::type_index get_type() const { return m_type; }
		// If the struct consists of a uniform type, component is non-GL_NONE
		GLenum get_component() const { return m_component; }

	private:
		size_t m_size;
		size_t m_stride;
		GLenum m_usage;

		std::type_index m_type;
		GLenum m_component;
	};
}

namespace hs
{
	template<class T>
	array_buffer array_buffer::empty(context& c, GLenum mode, GLenum usage)
	{
		return array_buffer(c, mode, usage, (T*)0);
	}

	template<class Iterator>
	array_buffer::array_buffer(context& c, GLenum mode, Iterator begin, Iterator end)
		: array_buffer(c, mode, GL_STATIC_DRAW, (typename std::iterator_traits<Iterator>::value_type*)0)
	{
		update(begin, end);
	}

	template<class T>
	array_buffer::array_buffer(context& c, GLenum mode, GLenum usage, T*)
		: context::identifier<GL_BUFFER>(c, mode), m_size{0}, m_type{typeid(T)}, m_usage{usage}
	{
		m_component = make_enum<remove_all_component_extents_t<T>>{};
		m_stride = sizeof(T);
	}

	template<class Iterator>
	void array_buffer::update(Iterator begin, Iterator end)
	{
		/* TODO: being random access gives a good chance the the data is
		* located sequentially, but does not ensure it - improve by using
		* a contiguous iterator trait as proposed for C++17
		*/
		using tag_type = typename std::iterator_traits<Iterator>::iterator_category;
		assert((std::is_same<tag_type, std::random_access_iterator_tag>::value));

		using value_type = typename std::iterator_traits<Iterator>::value_type;
		assert(std::type_index(typeid(value_type)) == m_type);

		auto old_size = m_size;
		m_size = std::distance(begin, end);

		auto data_size = m_size * sizeof(value_type);

		bind();
		// If the buffer size does not change, just update instead of reallocating
		// FIXME: not the best way to acquire a pointer to the data
		if (m_size == old_size)
			glBufferSubData(this->m_mode, 0, data_size, &(*begin));
		else
			glBufferData(this->m_mode, data_size, &(*begin), m_usage);
	}
}


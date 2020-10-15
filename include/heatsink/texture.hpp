#pragma once

#include <cassert>

#include <glm/glm.hpp>
#include <glm/gtc/vec1.hpp>
#include <glm/gtx/component_wise.hpp>

#include <GL/glew.h>
#include "context.hpp"
#include "enum_traits.hpp"

namespace hs
{
	namespace detail
	{
		template<size_t N>
		struct dimension_type;

		template<size_t N>
		using dimension_type_t = typename dimension_type<N>::type;
	}

	template<size_t N>
	class texture : public context::identifier<GL_TEXTURE>
	{
	public:
		using dimension_type = detail::dimension_type_t<N>;

	public:
		struct source
		{
		public:
			source(GLenum f)
				: format{f}, type{GL_NONE} {}
			source(GLenum f, GLenum t)
				: format{f}, type{t} {}

		public:
			GLenum format;
			GLenum type;
		};

		struct target
		{
		public:
			target(GLenum s)
				: storage{s}, level{0}, mode{GL_NONE} {}
			target(GLenum s, size_t l, GLenum m = GL_NONE)
				: storage{s}, level{l}, mode{m} {}

		public:
			GLenum storage;

			size_t level;
			GLenum mode;
		};

	public:
		static texture empty(context& c, GLenum mode) { return texture(c, mode); }

	public:
		texture(context& c, GLenum mode, dimension_type size)
			: context::identifier<GL_TEXTURE>(c, mode), m_size{size} {}

	private:
		texture(context& c, GLenum mode)
			: context::identifier<GL_TEXTURE>(c, mode), m_size{} {}

	public:
		void resize(dimension_type size) { m_size = size; }
		void reserve(target t);

		template<class Iterator>
		void update(source s, target t, Iterator begin, Iterator end);
		template<class Container>
		void update(source s, target t, const Container& c);

		template<class T>
		void fill(target t, const T& value);

		dimension_type size(size_t level = 0) const;
		size_t data_size(GLenum format, size_t level = 0) const;

		void set_filter(GLenum min, GLenum mag);

	private:
		void set_data(source s, target t, const void* data);

	private:
		dimension_type m_size;
	};

	using texture1 = texture<1>;
	using texture2 = texture<2>;
	using texture3 = texture<3>;
}

namespace hs
{
	namespace detail
	{
		template<>
		struct dimension_type<1> { using type = glm::uvec1; };
		template<>
		struct dimension_type<2> { using type = glm::uvec2; };
		template<>
		struct dimension_type<3> { using type = glm::uvec3; };
	}

	template<size_t N>
	void texture<N>::reserve(target t)
	{
		auto format = remove_size(t.storage);
		assert(format != GL_NONE);

		source s(format, GL_UNSIGNED_BYTE);
		set_data(s, t, nullptr);
	}

	template<size_t N> template<class Iterator>
	void texture<N>::update(source s, target t, Iterator begin, Iterator end)
	{
		// Check if the data is contiguous
		// A contiguous_iterator_tag would be ideal (C++17?)
		using tag_type = typename std::iterator_traits<Iterator>::iterator_category;
		assert((std::is_same<tag_type, std::random_access_iterator_tag>{}));

		using value_type = typename std::iterator_traits<Iterator>::value_type;
		if (s.type == GL_NONE)
			s.type = make_enum<remove_all_component_extents_t<value_type>>{};

		assert(s.type != GL_NONE);
		auto components = sizeof(value_type) / enum_sizeof(s.type);

		auto dim_size = std::distance(begin, end);
		auto byte_size = data_size(s.format, t.level);
		assert(dim_size * components == byte_size);

		// FIXME: not the best way to get a data pointer
		auto* data = static_cast<void*>(&(*begin));
		set_data(s, t, data);
	}

	template<size_t N> template<class Container>
	void texture<N>::update(source s, target t, const Container& c)
	{
		update(s, t, std::begin(c), std::end(c));
	}

	template<size_t N> template<class T>
	void texture<N>::fill(target t, const T& value)
	{
		auto format = remove_size(t.storage);
		assert(format != GL_NONE);

		source s(format);
		s.type = make_enum<remove_all_component_extents_t<T>>{};
		assert(s.type != GL_NONE);

		auto components = sizeof(T) / enum_sizeof(s.type);
		/* It would not make sense if a fill value was not a single value,
		* common across all channels, or a multi-component value describing each
		* channel of a pixel
		*/
		assert(components == 1 || components == enum_rank(s.format));

		auto byte_size = data_size(t.storage, t.level);
		// A multi-component value type requires fewer array elements to fill
		std::vector<T> fill_data(byte_size / components, value);

		set_data(s, t, fill_data.data());
	}

	template<size_t N>
	typename texture<N>::dimension_type texture<N>::size(size_t level) const
	{
		auto divisor = glm::pow(2, level);
		return (m_size / dimension_type(divisor));
	}

	template<size_t N>
	size_t texture<N>::data_size(GLenum format, size_t level) const
	{
		return (glm::compMul(size(level)) * enum_rank(format));
	}

	template<size_t N>
	void texture<N>::set_filter(GLenum min, GLenum mag)
	{
		bind_to(0);
		glTexParameteri(this->m_mode, GL_TEXTURE_MIN_FILTER, min);
		glTexParameteri(this->m_mode, GL_TEXTURE_MAG_FILTER, mag);
	}

	template<size_t N>
	void texture<N>::set_data(source s, target t, const void* data)
	{
		auto d = size(t.level);
		if (t.mode == GL_NONE)
			t.mode = this->m_mode;

		bind_to(0);
		switch (N)
		{
			case 1:
				glTexImage1D(t.mode, t.level, t.storage, d[0], 0, s.format, s.type, data);
				break;
			case 2:
				glTexImage2D(t.mode, t.level, t.storage, d[0], d[1], 0, s.format, s.type, data);
				break;
			case 3:
				glTexImage3D(t.mode, t.level, t.storage, d[0], d[1], d[2], 0, s.format, s.type, data);
				break;
		}
	}
}

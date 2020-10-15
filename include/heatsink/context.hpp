#pragma once

#include <functional>
#include <unordered_map>
#include <type_traits>

#include <GL/glew.h>
#include "bind_state.hpp"
#include "name_adaptor.hpp"
#include "name_traits.hpp"

namespace hs
{
	// An OpenGL surface context; reference counts GL objects created under it
	class context
	{
	public:
		// Contain all information needed for a unique GL identifier
		template<GLenum T>
		class identifier : protected bind_state<T>
		{
		public:
			// Only one method will be synthesizable depending on has_bind_mode<T>
			// Thanks to bind_state only defining a single constructor
			static identifier get_default() { return identifier(0); }
			static identifier get_default(GLenum mode) { return identifier(0, mode); }

		public:
			identifier(context& c);
			identifier(context& c, GLenum mode);

			identifier(const identifier& other);
			identifier(identifier&& other);

			~identifier();

		protected:
			// Allow for internal creation of arbitrary identifiers
			identifier(GLuint name)
				: bind_state<T>(name), m_context{context::empty_context} {}
			identifier(GLuint name, GLenum mode)
				: bind_state<T>(name, mode), m_context{context::empty_context} {}

		public:
			// Expose the bind methods of bind_state
			// Only one exposed in similar fashion to above
			void bind() const { bind_state<T>::bind(); }
			void bind_to(size_t unit) const { bind_state<T>::bind_to(unit); }

			// Retrieve the private parameters of the bind_state
			/* Take care when using name(), subclasses may store additional
			* state that will not be updated using a bare name
			*/
			GLuint name() const { return this->m_name; }
			GLenum mode() const { return this->m_mode; }
			operator GLuint() const { return name(); }

			context& get_context() { return m_context; }
			const context& get_context() const { return m_context; }

			// The "default" GL identifier for a type is always defined as name 0
			bool is_default() const { return (this->m_name == 0); }

		protected:
			std::reference_wrapper<context> m_context;
		};

	public:
		friend class window;

	private:
		// Not all GLuint IDs are unique; T may be required to differentiate
		struct unique_name
		{
			bool operator ==(const unique_name& other) const;

			GLenum type;
			GLuint name;
		};

		// unordered_map requires a comparison and hash function for custom types
		struct unique_name_hash
		{
			size_t operator ()(const unique_name& n) const;
		};

	public:
		static context empty_context;

	public:
		context(const context&) = delete;
		context(context&&) = delete;

	private:
		context() = default;

	private:
		// Called by identifier after construction/allocation of name
		template<GLenum T>
		void count_references(const identifier<T>& i);

		// Add to the reference count of an identifier
		template<GLenum T>
		void increment_references(const identifier<T>& i);
		// Remove a reference to an identifier and delete it if now none
		template<GLenum T>
		void decrement_references(const identifier<T>& i);

	private:
		std::unordered_map<unique_name, size_t, unique_name_hash> m_references;
	};
}

namespace hs
{
	template<GLenum T>
	context::identifier<T>::identifier(context& c)
		: bind_state<T>(name_adaptor<T>::create()), m_context{c}
	{
		m_context.get().count_references(*this);
	}

	template<GLenum T>
	context::identifier<T>::identifier(context& c, GLenum mode)
		: bind_state<T>(name_adaptor<T>::create(), mode), m_context{c}
	{
		m_context.get().count_references(*this);
	}

	template<GLenum T>
	context::identifier<T>::identifier(const identifier& other)
		: bind_state<T>(other), m_context{other.m_context}
	{
		m_context.get().increment_references(*this);
	}

	template<GLenum T>
	context::identifier<T>::identifier(identifier&& other)
		: bind_state<T>(other), m_context{other.m_context}
	{
		other.m_name = 0;
	}

	template<GLenum T>
	context::identifier<T>::~identifier()
	{
		m_context.get().decrement_references(*this);
	}

	template<GLenum T>
	void context::count_references(const identifier<T>& i)
	{
		if (i.is_default())
			return;

		m_references.emplace(unique_name{T, i.name()}, 1);
	}

	template<GLenum T>
	void context::increment_references(const identifier<T>& i)
	{
		if (i.is_default())
			return;

		auto& count = m_references.at({T, i.name()});
		count += 1;
	}

	template<GLenum T>
	void context::decrement_references(const identifier<T>& i)
	{
		if (i.is_default())
			return;

		unique_name key{T, i.name()};

		auto& count = m_references.at(key);
		count -= 1;

		if (!count)
		{
			m_references.erase(key);
			name_adaptor<T>::destroy(key.name);
		}
	}
}

#pragma once

#include <string>

#include <glm/glm.hpp>

#include "context.hpp"

namespace hs
{
	// Represent a backend-agnostic window
	// Contains a unique context for creating OpenGL identifiers
	class window
	{
	public:
		window(glm::uvec2 size)
			: m_size{size}, m_context() {}

		window(const window&) = delete;
		window(window&&) = delete;

		virtual ~window() {}

	public:
		// Make this window the active target for OpenGL
		virtual void make_active() const = 0;
		// Swap the display buffers, and return whether the window must close
		virtual bool refresh() const = 0;

		const glm::uvec2& size() const { return m_size; }
		// The framebuffer size is not necessarily the same as size on all computers
		// For example, in laptops with HiDPI displays
		virtual const glm::uvec2& framebuffer_size() const { return size(); }

		context& get_context() { return m_context; }
		const context& get_context() const { return m_context; }

	private:
		glm::uvec2 m_size;
		context m_context;
	};
}

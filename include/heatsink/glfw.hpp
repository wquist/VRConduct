#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "init.hpp"
#include "window.hpp"

namespace hs
{
	namespace backend
	{
		void init(const settings& s);

		class glfw_window : public hs::window
		{
		public:
			glfw_window(const std::string& name, glm::uvec2 size);

			~glfw_window();

		public:
			void make_active() const;
			bool refresh() const;

			const glm::uvec2& framebuffer_size() const { return m_fbsize; }

			GLFWwindow* get_handle() const { return m_handle; }
			operator GLFWwindow*() const { return m_handle; }

		private:
			GLFWwindow* m_handle;
			glm::uvec2 m_fbsize;
		};

		using window = glfw_window;
	}
}

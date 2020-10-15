#include <heatsink/glfw.hpp>

#include <cassert>

void hs::backend::init(const settings& s)
{
	auto glfw_result = glfwInit();
	assert(glfw_result);

	glfwWindowHint(GLFW_SAMPLES, s.samples);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, s.version_major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, s.version_minor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (s.forward_only)
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Create a dummy window to allow glew to be initialized
	auto* dummy = glfwCreateWindow(100, 100, "heatsink", nullptr, nullptr);
	assert(dummy);

	glfwMakeContextCurrent(dummy);
	glewExperimental = true;

	auto glew_result = glewInit();
	assert(glew_result == GLEW_OK);

	glfwDestroyWindow(dummy);
}

hs::backend::glfw_window::glfw_window(const std::string& name, glm::uvec2 size)
	: window(size)
{
	m_handle = glfwCreateWindow(size.x, size.y, name.c_str(), nullptr, nullptr);
	assert(m_handle);

	int w, h;
	glfwGetFramebufferSize(m_handle, &w, &h);
	m_fbsize = glm::uvec2(static_cast<size_t>(w), static_cast<size_t>(h));

	make_active();
}

hs::backend::glfw_window::~glfw_window()
{
	glfwDestroyWindow(m_handle);
}

void hs::backend::glfw_window::make_active() const
{
	glfwMakeContextCurrent(m_handle);
}

bool hs::backend::glfw_window::refresh() const
{
	glfwSwapBuffers(m_handle);
	glfwPollEvents();

	return (!glfwWindowShouldClose(m_handle));
}

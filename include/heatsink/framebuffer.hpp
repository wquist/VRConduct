#pragma once

#include <unordered_map>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "context.hpp"
#include "texture.hpp"

namespace hs
{
	// Manage an OpenGL framebuffer and its texture attachments
	class framebuffer : public context::identifier<GL_FRAMEBUFFER>
	{
	public:
		// Framebuffers have three bind modes for read, draw or both
		enum class access { read, write, readwrite };

	public:
		framebuffer(context& c, const glm::uvec2& size);

		framebuffer(framebuffer&&) = default;

	public:
		// Set all texture attachments for this framebuffer
		// Will destroy any old attachments
		void set_attachments(const std::vector<GLenum>& cs, GLenum d = GL_NONE);

		// Select which buffers will be written to in the fragment shader
		void make_drawable(const std::vector<size_t>& indices);
		// Use all buffers as drawable in the fragment shader
		void make_all_drawable();
		// Select a buffer to read from - used for glReadPixels, glCopyTex*, etc
		void make_readable(size_t index);

		const glm::uvec2& size() const { return m_size; }

		texture2& get_color(size_t i);
		texture2& get_depth();

		// Set the access mode to be used when bind() is called
		/* A bind point created from the framebuffer will use the last mode
		* specified here - previously created bind points will not be updated
		* when the access is changed again
		*/
		void set_access(access mode);

	private:
		glm::uvec2 m_size;
		std::unordered_map<GLenum, texture2> m_textures;
	};
}

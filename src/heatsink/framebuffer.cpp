#include <heatsink/framebuffer.hpp>

#include <vector>

hs::framebuffer::framebuffer(context& c, const glm::uvec2& size)
	: identifier(c, GL_FRAMEBUFFER), m_size{size}
{
	assert(m_size.x > 0 && m_size.y > 0);
}

void hs::framebuffer::set_attachments(const std::vector<GLenum>& cs, GLenum d)
{
	// FIXME: does m_id.mode need to be GL_FRAMEBUFFER or GL_DRAW_FRAMEBUFFER?

	// First remove all existing attachments (if any)
	bind();
	for (auto& tex : m_textures)
		glFramebufferTexture(this->m_mode, tex.first, 0, 0);

	m_textures.clear();

	// Determine what type of textures should be created
	// FIXME: would be better to pass this information with attachments
	GLenum texture_mode{GL_TEXTURE_2D};

	// Then bind all specified color textures
	GLenum next_loc{GL_COLOR_ATTACHMENT0};
	for (auto& color : cs)
	{
		// The dimension sizing works the same in texture as here
		texture2 tex(this->m_context, texture_mode, m_size);
		tex.reserve(color);

		m_textures.emplace(next_loc++, std::move(tex));
	}

	// Check if a depth attachment was also specified
	if (d != GL_NONE)
	{
		texture2 tex(this->m_context, texture_mode, m_size);
		tex.reserve(d);

		GLenum depth_base{remove_size(d)};
		assert(depth_base != GL_NONE);

		// DEPTH and DEPTH_STENCIL should be the only possible cases
		if (depth_base == GL_DEPTH_COMPONENT)
			m_textures.emplace(GL_DEPTH_ATTACHMENT, std::move(tex));
		else if (depth_base == GL_DEPTH_STENCIL)
			m_textures.emplace(GL_DEPTH_STENCIL_ATTACHMENT, std::move(tex));
	}

	// Attach all of the created textures to the framebuffer
	for (auto& tex : m_textures)
	{
		GLuint name{tex.second.name()};
		glFramebufferTexture(this->m_mode, tex.first, name, 0);
	}

	// Ensure the framebuffer was properly constructed
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	make_all_drawable();
}

void hs::framebuffer::make_drawable(const std::vector<size_t>& indices)
{
	// A framebuffer can make no attachments drawable by passing GL_NONE
	if (!indices.size())
	{
		glDrawBuffer(GL_NONE);
		return;
	}

	std::vector<GLenum> attachments;
	for (auto& i : indices)
	{
		auto attachment = GL_COLOR_ATTACHMENT0 + i;

		assert(m_textures.count(attachment));
		attachments.push_back(attachment);
	}

	glDrawBuffers(attachments.size(), attachments.data());
}

void hs::framebuffer::make_all_drawable()
{
	std::vector<GLenum> attachments;
	// Find all attachments that are GL_COLOR_ATTACHMENTs
	for (auto& tex : m_textures)
	{
		bool is_depth{tex.first == GL_DEPTH_ATTACHMENT};
		bool is_stencil{tex.first == GL_DEPTH_STENCIL_ATTACHMENT};

		if (!(is_depth || is_stencil))
			attachments.push_back(tex.first);
	}

	glDrawBuffers(attachments.size(), attachments.data());
}

void hs::framebuffer::make_readable(size_t index)
{
	auto attachment = GL_COLOR_ATTACHMENT0 + index;
	assert(m_textures.count(attachment));

	glReadBuffer(attachment);
}

hs::texture2& hs::framebuffer::get_color(size_t i)
{
	return m_textures.at(GL_COLOR_ATTACHMENT0 + i);
};

hs::texture2& hs::framebuffer::get_depth()
{
	return m_textures.at(GL_DEPTH_ATTACHMENT);
}

void hs::framebuffer::set_access(access mode)
{
	switch (mode)
	{
		case access::read:
			this->m_mode = GL_READ_FRAMEBUFFER;
			break;
		case access::write:
			this->m_mode = GL_DRAW_FRAMEBUFFER;
			break;
		case access::readwrite:
			this->m_mode = GL_FRAMEBUFFER;
			break;
	}
}

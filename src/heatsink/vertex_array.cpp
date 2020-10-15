#include <heatsink/vertex_array.hpp>

void hs::vertex_array::set_attribute(attribute a, const array_buffer& b)
{
	assert(b.mode() == GL_ARRAY_BUFFER);

	// Component can be deduced as entire array_buffer representation
	if (a.component == GL_NONE)
		a.component = b.get_component();

	assert(a.component != GL_NONE);

	auto component_size = enum_sizeof(a.component);
	auto size = b.stride() / component_size;
	// FIXME: support for sequential matrix attributes
	assert(size <= 4);

	bind();
	glEnableVertexAttribArray(a.index);

	b.bind();
	glVertexAttribPointer(a.index, size, a.component, a.normalize, b.stride(), nullptr);
}

void hs::vertex_array::set_elements(const array_buffer& b)
{
	assert(b.mode() == GL_ELEMENT_ARRAY_BUFFER);

	GLenum type = b.get_component();
	assert(type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT || type == GL_UNSIGNED_INT);

	m_indexing = type;

	bind();
	b.bind();
}

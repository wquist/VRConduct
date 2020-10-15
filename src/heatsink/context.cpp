#include <heatsink/context.hpp>

hs::context hs::context::empty_context{};

bool hs::context::unique_name::operator ==(const unique_name& other) const
{
	return ((type == other.type) && (name == other.name));
}

size_t hs::context::unique_name_hash::operator ()(const unique_name& n) const
{
	return (std::hash<GLenum>()(n.type) ^ std::hash<GLuint>()(n.name));
}

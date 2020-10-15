#pragma once

#include <set>
#include <vector>

#include <glm/glm.hpp>

#include <heatsink/array_buffer.hpp>
#include <heatsink/vertex_array.hpp>
#include <heatsink/shader.hpp>

#include <helper/assimp.hpp>

class model
{
public:
	// Vertex Struct
	struct vertex
	{
	public:
		vertex(const aiMesh& mesh, size_t index) 
		{
			auto& v = mesh.mVertices[index];
			position = glm::vec3(v.x, v.y, v.z);

			auto& n = mesh.mNormals[index];
			normal = glm::vec3(n.x, n.y, n.z);
		}

	public:
		glm::vec3 position;
		glm::vec3 normal;
	};

	// Mesh Struct
	struct mesh
	{
		using material_data = helper::assimp_model::material_data;

		hs::array_buffer vertices;
		hs::vertex_array state;
		material_data    material;

		mesh(hs::context& con, const std::vector<vertex>& verts, const material_data& mat) 
		: vertices(con, GL_ARRAY_BUFFER, verts), state(con), material(mat)
		{
			state.set_attribute(0, vertices, &vertex::position);
			state.set_attribute(1, vertices, &vertex::normal);
		}
	};

private:
	std::vector<mesh> m_meshes;
	glm::vec3 m_position;

public:
	model() = delete;
	model(hs::context& con, const std::string& modelPath);
	
	model(const model&& other) = delete;
	model(model&& other);

	model&& operator =(const model&& other) = delete;
	model&& operator =(model&& other);

	~model() = default;

public:
	static std::unordered_map<std::string, std::shared_ptr<model>> g_model_cache;
	static std::shared_ptr<model> load_model(hs::context& context, const std::string& path);
	static std::shared_ptr<model> get_model(const std::string& path);

public:
	// Accessors
	glm::vec3 position() const { return m_position; }

	// Setters
	void position(const glm::vec3& pos) { m_position = pos; }

	// Rendering methods
	void draw(hs::shader& shad, const std::set<std::string>& unis, const glm::mat4& M = glm::mat4(1.f));
};
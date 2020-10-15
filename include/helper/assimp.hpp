#pragma once

#include <map>
#include <string>
#include <vector>

#include <iostream>
#include <stdexcept>

#include <GL/glew.h>

#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/glm.hpp>

namespace helper
{
	class assimp_model
	{
	public:
		template<typename V, typename I>
		struct vertex_data
		{
		public:
			vertex_data(const std::vector<V>& vs, const std::vector<I>& is)
				: vertices{vs}, indices{is} {}

		public:
			std::vector<V> vertices;
			std::vector<I> indices;
		};

		template<typename V, typename I>
		struct mesh
		{
		public:
			mesh() = default;
			mesh(const std::vector<V>& vs, const std::vector<I>& is, size_t m)
				: data(vs, is), material_index{m} {}

		public:
			vertex_data<V, I> data;
			size_t material_index;
		};

		struct material_data
		{
		public:
			using texture_map = std::map<aiTextureType, std::string>;

		public:
			material_data() = default;
			material_data(size_t i, const glm::vec3& c, const texture_map& ts)
				: index{i}, color{c}, textures{ts} {}

		public:
			size_t index;

			glm::vec3 color;
			texture_map textures;
		};

	public:
		assimp_model(const std::string& filename);

	public:
		template<typename V, typename I = GLuint>
		std::vector<mesh<V, I>> get_meshes() const;

		std::vector<material_data> get_materials() const;

	private:
		const aiScene* m_scene;
		std::string m_filename;
	};
}

template<typename V, typename I>
auto helper::assimp_model::get_meshes() const -> std::vector<mesh<V, I>>
{
	if (!m_scene) {
		std::cout << "WTF -> " << m_filename << std::endl;
		return std::vector<mesh<V, I>>{};
	}

	std::vector<mesh<V, I>> meshes;
	meshes.reserve(m_scene->mNumMeshes);

	for (auto i = 0; i != m_scene->mNumMeshes; ++i)
	{
		auto* src = m_scene->mMeshes[i];

		std::vector<V> vertices;
		//vertices.reserve(src->mNumVertices);

		//for (auto j = 0; j != src->mNumVertices; ++j)
		//	vertices.emplace_back(*src, j);

		std::vector<I> indices(src->mNumFaces * 3);
		for (auto j = 0; j != src->mNumFaces; ++j)
		{
			auto& f = src->mFaces[j];
			for (auto k = 0; k != f.mNumIndices; ++k)
				vertices.emplace_back(*src, f.mIndices[k]);
			//std::copy(f.mIndices, f.mIndices + 3, indices.begin() + (j * 3));
		}

		meshes.emplace_back(vertices, indices, src->mMaterialIndex);
	}

	return meshes;
}

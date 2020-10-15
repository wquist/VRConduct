#include <helper/assimp.hpp>

#include <cassert>

helper::assimp_model::assimp_model(const std::string& filename)
: m_filename{ filename } {
	auto flags = aiProcess_Triangulate | aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_GenNormals;
	m_scene = aiImportFile(filename.c_str(), flags);

	assert(((unsigned int)m_scene) != 0u);
}

auto helper::assimp_model::get_materials() const -> std::vector<material_data>
{
	const std::vector<aiTextureType> texture_types =
	{
		aiTextureType_DIFFUSE,
		aiTextureType_SPECULAR,
		aiTextureType_AMBIENT,
		aiTextureType_EMISSIVE,
		aiTextureType_HEIGHT,
		aiTextureType_NORMALS,
		aiTextureType_SHININESS,
		aiTextureType_OPACITY,
		aiTextureType_DISPLACEMENT,
		aiTextureType_LIGHTMAP,
		aiTextureType_REFLECTION
	};

	std::vector<material_data> materials;
	materials.reserve(m_scene->mNumMaterials);

	for (auto i = 0; i != m_scene->mNumMaterials; ++i)
	{
		auto* src = m_scene->mMaterials[i];

		material_data::texture_map textures;
		for (auto& type : texture_types)
		{
			if (!aiGetMaterialTextureCount(src, type))
				continue;

			aiString path;
			auto res = aiGetMaterialTexture(src, type, 0, &path);
			assert(res == AI_SUCCESS);

			textures.emplace(type, std::string(path.data));
		}

		aiColor4D c{1.f, 1.f, 1.f, 1.f};
		aiGetMaterialColor(src, AI_MATKEY_COLOR_DIFFUSE, &c);

		materials.emplace_back(i, glm::vec3{c.r, c.g, c.b}, textures);
	}

	return materials;
}
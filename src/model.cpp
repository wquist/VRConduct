#include "model.hpp"

#include <map>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

// Static map in order to only import a model once
std::unordered_map<std::string, std::shared_ptr<model>> model::g_model_cache = {};

model::model(hs::context& con, const std::string& modelPath)
: m_position{}
{
	helper::assimp_model mod(modelPath);
	auto mshs = mod.get_meshes<vertex>();
	auto mats = mod.get_materials();

	for (auto& msh : mshs)
		m_meshes.emplace_back(con, msh.data.vertices, mats[msh.material_index]);
}

void model::draw(hs::shader& shad, const std::set<std::string>& unis, const glm::mat4& M) 
{
	shad.bind();

	for (auto& msh : m_meshes)
	{
		if (unis.count("M"))
			shad.set_uniform("M", glm::translate(glm::mat4(1.f), m_position) * M);
		if (unis.count("Color"))
			shad.set_uniform("Color", msh.material.color);
		if (unis.count("Selected"))
			shad.set_uniform("Selected", glm::vec3(0.3f, 0.f, 0.f));

		msh.state.bind();
		glDrawArrays(GL_TRIANGLES, 0, msh.vertices.size());

		if (unis.count("Selected"))
			shad.set_uniform("Selected", glm::vec3(0.f, 0.f, 0.f));
	}
}

std::shared_ptr<model> model::load_model(hs::context& contex, const std::string& path)
{
	std::shared_ptr<model> m;
	if (g_model_cache.count(path) == 0) {
		m = std::make_shared<model>(contex, path);
		g_model_cache.emplace(path, m);
	} else {
		m = g_model_cache.at(path);
	}

	return m;
}

std::shared_ptr<model> model::get_model(const std::string& path) {
	return g_model_cache.at(path);
}
#pragma once

#include <set>
#include <string>

#include <heatsink/context.hpp>
#include <heatsink/shader.hpp>

#include <audio.hpp>
#include <model.hpp>

class instrument
{
public:
	enum class family
	{
		piano,
		hammered,
		organ,
		guitar,
		bass_guitar,
		violin,
		viola,
		cello,
		contrabass,
		strings,
		harp,
		timpani,
		voice,
		percussion,
		trumpet,
		trombone,
		tuba,
		french_horn,
		brass,
		saxophone,
		oboe,
		english_horn,
		bassoon,
		clarinet,
		piccolo,
		flute,
		recorder,
		wind,
		effect,
		ethnic
	};

public:
	static family get_family(size_t voice);

	static std::string get_model_path(family fam);
	static glm::vec3 get_location(family fam);

public:
	instrument(hs::context& c, size_t voice, std::shared_ptr<audio::source> src);

public:
	void draw(hs::shader& s, const std::set<std::string>& unis);

public:
	glm::vec3 position() const { return m_position; }
	void position(const glm::vec3& p) {
		m_position = p;

		m_source->set(AL_POSITION, p);
	}

	bool selected() const { return m_selected; }
	void selected(bool s) { m_selected = s; }

	static void init(hs::context& context, const std::vector<char>& channels);

private:
	family m_family;
	std::shared_ptr<model> m_model;
	std::shared_ptr<audio::source> m_source;

	bool m_selected;
	glm::vec3 m_position;
};

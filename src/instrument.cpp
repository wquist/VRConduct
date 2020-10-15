#include <instrument.hpp>

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

instrument::family instrument::get_family(size_t voice)
{
	if (voice <= 8)
		return family::piano;
	if (voice <= 15)
		return family::hammered;
	if (voice <= 23)
		return family::organ;
	if (voice <= 31)
		return family::guitar;
	if (voice <= 39)
		return family::bass_guitar;
	if (voice == 40)
		return family::violin;
	if (voice == 41)
		return family::viola;
	if (voice == 42)
		return family::cello;
	if (voice == 43)
		return family::contrabass;
	if (voice <= 45)
		return family::strings;
	if (voice == 46)
		return family::harp;
	if (voice == 47)
		return family::timpani;
	if (voice <= 51)
		return family::strings;
	if (voice <= 54)
		return family::voice;
	if (voice == 55)
		return family::percussion;
	if (voice == 56)
		return family::trumpet;
	if (voice == 57)
		return family::trombone;
	if (voice == 58)
		return family::tuba;
	if (voice == 59)
		return family::trumpet;
	if (voice == 60)
		return family::french_horn;
	if (voice <= 63)
		return family::brass;
	if (voice <= 67)
		return family::saxophone;
	if (voice == 68)
		return family::oboe;
	if (voice == 69)
		return family::english_horn;
	if (voice == 70)
		return family::bassoon;
	if (voice == 71)
		return family::clarinet;
	if (voice == 72)
		return family::piccolo;
	if (voice == 73)
		return family::flute;
	if (voice == 74)
		return family::recorder;
	if (voice <= 79)
		return family::wind;
	if (voice <= 104)
		return family::effect;
	if (voice <= 114)
		return family::ethnic;
	if (voice <= 120)
		return family::percussion;

	return family::effect;
}

std::string instrument::get_model_path(family fam)
{
	switch (fam)
	{
	case family::piano:
		return "data/model/piano_corrected.obj";
	case family::strings:
	case family::violin:
		return "data/model/violin_corrected.obj";
	case family::cello:
		return "data/model/cello_corrected.obj";
	case family::contrabass:
		return "data/model/double_bass_corrected.obj";
	case family::trumpet:
		return "data/model/trumpet_corrected.obj";
	case family::trombone:
		return "data/model/trombone.obj";
	case family::french_horn:
		return "data/model/french_horn_corrected.obj";
	case family::clarinet:
		return "data/model/clarinet_corrected.obj";
	case family::flute:
		return "data/model/flute_corrected.obj";
	case family::harp:
		return "data/model/harp_corrected.obj";
	case family::bassoon:
		return "data/model/bassoon_corrected.obj";
	case family::tuba:
		return "data/model/tuba_corrected.obj";
	default:
		// std::cout << "UNKNOWN: " << (int) fam << std::endl;
		return "data/model/speaker_corrected.obj";
	}
}

instrument::instrument(hs::context& c, size_t voice, std::shared_ptr<audio::source> src)
: m_selected(false), m_source{src}
{
	m_model = model::get_model(get_model_path(get_family(voice)));
}

void instrument::draw(hs::shader& s, const std::set<std::string>& unis)
{
	// HACK: For use with cached versions of same model
	m_model->position(m_position);

	auto sunis = unis;
	if (m_selected && sunis.count("Color"))
		sunis.emplace("Selected");

	auto angle = std::atan2(m_model->position().z, -1 * m_model->position().x) + glm::radians(180.0f);
	auto look = glm::rotate(glm::mat4(1.f), angle, glm::vec3{ 0,1,0 });
	m_model->draw(s, sunis, look);
}

void instrument::init(hs::context& context, const std::vector<char>& channels) {
	for (const auto& c : channels)
	{
		//std::cout << "Loading voice: " << (size_t)c << " with family: " << (int)get_family((size_t)c) << std::endl;

		family fam = get_family(c);

		// Try to load. Will cache on load
		model::load_model(context, get_model_path(fam));
	}
}
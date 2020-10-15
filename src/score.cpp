#include <score.hpp>

#include <memory>
#include <stdexcept>

#include <iostream>
#include <vector>

//##############################################################################
// Channel
//##############################################################################
score::channel::channel(tml_message* m, tsf* r, bool* score_playing, size_t frequency)
: m_start{m}, m_current{m}, m_end{m}, m_renderer{tsf_copy(r)}, m_time{0.0}, m_score_playing{score_playing}, m_freq{frequency} {

	// For knowing what program is used...
	if (m->type == TML_PROGRAM_CHANGE) {
		m_preset_number = m->program;
	}

	tsf_set_output(m_renderer, TSF_MONO, m_freq, 0);
}

score::channel::~channel() {
	if (m_renderer)
		tsf_close(m_renderer);
}

score::channel::channel(channel&& other)
: m_start{other.m_start}, m_current{other.m_current}, m_renderer{other.m_renderer}, m_time{0} {
	other.m_renderer = nullptr;
}

score::channel& score::channel::operator =(channel&& other) {
	if (m_renderer)
		tsf_close(m_renderer);

	m_start    = other.m_start;
	m_current  = other.m_current;
	m_renderer = other.m_renderer;

	other.m_renderer = nullptr;
	return *this;
}

void score::channel::push_event(tml_message* m) {
	m_end->next = m;
	m_end = m;

	// For knowing what program is used...
	if (m->type == TML_PROGRAM_CHANGE) {
		m_preset_number = m->program;
	}
}

const tml_message* score::channel::peek_event() const {
	return m_current;
}

tml_message* score::channel::pop_event() {
	auto* m = m_current;
	if (m_current)
		m_current = m_current->next;

	return m;
}

const char score::channel::get_preset_number() const {
	return m_preset_number;
}

const std::string score::channel::get_preset_name() const {
	// return tsf_get_presetname(m_renderer, m_preset_number);
	return "UNKNOWN";
}

size_t score::channel::read(void* buf, size_t count) {
	if (*m_score_playing == false) return 0;

	std::vector<channel_sample> d(count);

	auto* lmf = m_current;
	auto* lsf = m_renderer;
	auto delta = d.size() * (1000.0 / (double)m_freq) / 4;

	for (auto i = 0u; i != 4; ++i) {
		while (lmf && lmf->time < m_time + delta) {
			switch (lmf->type) {
				case TML_PROGRAM_CHANGE:
					tsf_channel_set_presetnumber(lsf, lmf->channel, lmf->program, (lmf->channel == 9));
					break;
				case TML_NOTE_ON:
					tsf_channel_note_on(lsf, lmf->channel, lmf->key, lmf->velocity / 127.f);
					break;
				case TML_NOTE_OFF:
					tsf_channel_note_off(lsf, lmf->channel, lmf->key);
					break;
				case TML_PITCH_BEND:
					tsf_channel_set_pitchwheel(lsf, lmf->channel, lmf->pitch_bend);
					break;
				case TML_CONTROL_CHANGE:
					tsf_channel_midi_control(lsf, lmf->channel, lmf->control, lmf->control_value);
					break;
			}

			lmf = lmf->next;
		}

		m_current = lmf;
		m_time += delta;

		if constexpr (std::is_same_v<channel_sample, float>)
			tsf_render_float(lsf, (float*)d.data() + i * d.size() / 4, d.size() / 4, 0);
		else
			tsf_render_short(lsf, (short*)d.data() + i * d.size() / 4, d.size() / 4, 0);
	}

	std::copy(d.begin(), d.end(), (channel_sample*)buf);
	return d.size();
}

void score::channel::reset() {
	m_current = m_start;
	m_time = 0;
}

bool score::channel::end_of_stream() const {
	return (m_current == nullptr);
}

//##############################################################################
// Score
//##############################################################################
score::score(const std::string& midi_path, const std::string& soundfont_path, size_t frequency)
: m_playing{false} {
	tsf* sf = tsf_load_filename(soundfont_path.c_str());
	if (!sf)
		throw std::runtime_error("Could not load SoundFont file: " + soundfont_path);

	tml_message* mf = m_messages = tml_load_filename(midi_path.c_str());
	if (!mf)
		throw std::runtime_error("Could not load MIDI file: " + midi_path);

	while (mf) {
		channel_index chi(mf->channel);
		if (!m_channels.count(chi))
			m_channels.try_emplace(chi, std::make_shared<channel>(mf, sf, &m_playing, frequency));
		else
			m_channels.at(chi)->push_event(mf);

		mf = mf->next;
	}

	// Extract extra channel info from channel midi streams
	for (auto& c : m_channels) {
		std::cout << "Channel '" << c.first << "' is of voice type '" << c.second->get_preset_name() << "'." << std::endl;
	}

	tsf_close(sf);
}

score::~score() {
	if (m_messages)
		tml_free(m_messages);
}

score::channel::ptr score::get_channel(channel_index i) {
	return m_channels.at(i);
}

std::map<score::channel_index, score::channel::ptr> score::get_channels() {
	return m_channels;
}

const std::vector<score::channel_index> score::get_channel_indices() const {
	std::vector<channel_index> res;

	for (const auto& c : m_channels)
		res.push_back(c.first);

	return res;
}

const std::vector<char> score::get_channel_presets() const {
	std::vector<char> res;

	for (const auto& c : m_channels)
		res.push_back(c.second->get_preset_number());

	return res;
}

bool score::is_playing() const {
	return m_playing;
}

void score::play() {
	m_playing = true;
}

void score::pause() {
	m_playing = false;
}

void score::stop() {
	m_playing = false;

	for (auto c : m_channels)
		c.second->reset();
}

void score::toggle() {
	m_playing = !m_playing;
}
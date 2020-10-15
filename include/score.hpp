#pragma once

#include <map>
#include <string>
#include <TinySoundFont/tsf.h>
#include <TinySoundFont/tml.h>

#include <audio.hpp>

class score {
public:
	using channel_index = size_t;
	using channel_sample = short;

	class channel : public audio::stream {
	public:
		using ptr = std::shared_ptr<channel>;

	public:
		channel(tml_message* m, tsf* r, bool* score_playing, size_t frequency);
		~channel();

		channel(const channel&) = delete;
		channel(channel&& other);

		channel& operator =(const channel&) = delete;
		channel& operator =(channel&& other);

	public:
		void push_event(tml_message* m);

		const tml_message* peek_event() const;
		tml_message* pop_event();
	
	public:
		const char get_preset_number() const;
		const std::string get_preset_name() const;

	public:
		/**
		 * Read the requisite number of bytes from the stream, stopping at the end of the file. Advances
		 * the read pointer.
		 *
		 * @param[in]	buf		Pre-allocated buffer to read the data into.
		 * @param[in]	count	Number of bytes to read.
		 * @return				Number of bytes actually read.
		 * 			
		 * @note	Stream must be created with READ access mode.
		 */
		size_t read(void* buf, size_t count);
		void reset();

		/** Returns true if the stream has reached the end. */
		bool end_of_stream() const;

	private:
		tml_message* m_start;
		tml_message* m_current;
		tml_message* m_end;

		tsf* m_renderer;
		double m_time;

		// FIXME: This is kind of nasty
		bool* m_score_playing;

		// Program info
		char m_preset_number;

		size_t m_freq;
	};

public:
	// score(const std::string& midi_path, ...);
	score(const std::string& midi_path, const std::string& soundfont_path, size_t frequency);
	~score();

public:
	channel::ptr get_channel(channel_index i);
	std::map<channel_index, channel::ptr> get_channels();
	
	const std::vector<channel_index> get_channel_indices() const;
	const std::vector<char> get_channel_presets() const;

public:
	bool is_playing() const;

	void play();
	void pause();
	void stop();
	void toggle();

private:
	std::map<channel_index, channel::ptr> m_channels;
	tml_message* m_messages;

	bool m_playing;
};
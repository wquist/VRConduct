#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx-presets.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <atomic>
#include <map>
#include <queue>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

class audio {
public:
	static const size_t AUDIO_SIZE = 1024;
	
public:
	class stream {
	public:
		virtual size_t read(void* buf, size_t count) = 0;
		virtual void reset() = 0;

		virtual bool end_of_stream() const = 0;
	};

public:
	class device {
	private:
		ALCdevice* m_device;
	
	public:
		device();
		device(const std::string& dev);
		~device();

		device(const device&) = delete;
		device(device&& other);

		device& operator =(const device&) = delete;
		device& operator =(device&& other);

		operator ALCdevice* ();
	};

	class source {
	public:
		source(int pool_size = 8);
		source(std::shared_ptr<stream> str, int pool_size = 8);
		~source();

		source(const source&) = delete;
		source(source&& other);

		source& operator =(const source&) = delete;
		source& operator =(source&& other);

		operator ALuint ();

		template<class T>
		std::tuple<T, ALenum> get(ALenum e) const;
		template<class T>
		ALenum set(ALenum e, const T& t);

		template<class InsertFn>
		ALenum queue_buffer(InsertFn ifn);
		std::tuple<size_t, ALenum> dequeue_buffers(bool should_wait = true);
	
	private:
		ALuint m_source;
		std::queue<ALuint> m_buffer_queue;
		std::vector<ALuint> m_buffers;

		std::shared_ptr<stream> m_stream;
	};

public:
	static ALenum get_error();

	audio(device& d);
	~audio();

	audio(const audio&) = delete;
	audio(audio&& other);

	audio& operator =(const audio&) = delete;
	audio& operator =(audio&& other);

	bool bind();
	bool unbind();

	//... make_effect(...);

public:
	static std::vector<std::string> enumerate_devices();

public:
	std::shared_ptr<source> attach_source(int pool_size = 8);
	std::shared_ptr<source> attach_source(std::shared_ptr<stream> str, int pool_size = 8);

	size_t get_frequency() const;

private:
	ALCcontext* m_context;
	std::vector<std::shared_ptr<source>> m_sources;

	bool m_should_run;
	std::thread m_filler;

	size_t m_freq;
};

template<class T>
std::tuple<T, ALenum> audio::source::get(ALenum e) const {
	audio::get_error();
	
	T t;
	if constexpr (std::is_scalar_v<T>) {
		if constexpr (std::is_integral_v<T>)
			alGetSourcei(m_source, e, &t);
		else
			alGetSourcef(m_source, e, &t);
	} else {
		if constexpr (std::is_integral_v<decltype(t[0])>)
			alGetSourceiv(m_source, e, glm::value_ptr(t));
		else
			alGetSourcefv(m_source, e, glm::value_ptr(t));
	}

	return std::make_tuple(t, audio::get_error());
}

template<class T>
ALenum audio::source::set(ALenum e, const T& t) {
	audio::get_error();

	if constexpr (std::is_scalar_v<T>) {
		if constexpr (std::is_integral_v<T>)
			alSourcei(m_source, e, t);
		else
			alSourcef(m_source, e, t);
	} else {
		if constexpr (std::is_integral_v<decltype(t[0])>)
			alSourceiv(m_source, e, glm::value_ptr(t));
		else
			alSourcefv(m_source, e, glm::value_ptr(t));
	}

	return audio::get_error();
}

template <class InsertFn>
ALenum audio::source::queue_buffer(InsertFn ifn) {
	static_assert(std::is_same_v<void, std::invoke_result_t<InsertFn, ALuint>>);
	if (m_buffer_queue.size() == 0) return AL_INVALID_VALUE;

	auto buf = m_buffer_queue.front(); m_buffer_queue.pop();
	ifn(buf);

	audio::get_error();
	alSourceQueueBuffers(m_source, 1, &buf);

	return audio::get_error();
}

#include <audio.hpp>

//------------------------------------------------------------------------------
// Device
//------------------------------------------------------------------------------
audio::device::device() {
	m_device = alcOpenDevice(nullptr);
	assert(m_device);
}

audio::device::device(const std::string& dev) {
	m_device = alcOpenDevice(dev.c_str());
	assert(m_device);
}

audio::device::~device() {
	if (m_device)
		alcCloseDevice(m_device);
}

audio::device::device(device&& other)
: m_device{other.m_device} {
	other.m_device = nullptr;
}

audio::device& audio::device::operator =(device&& other) {
	if (m_device)
		alcCloseDevice(m_device);

	m_device = other.m_device;
	other.m_device = nullptr;

	return *this;
}

audio::device::operator ALCdevice* () {
	return m_device;
}

//------------------------------------------------------------------------------
// Source
//------------------------------------------------------------------------------
audio::source::source(int pool_size)
: m_stream{nullptr} {
	audio::get_error();
	alGenSources(1, &m_source);
	auto e = audio::get_error();
	assert(e == AL_NO_ERROR);
	assert(alIsSource(m_source));

	// Buffer pool
	m_buffers.resize(pool_size);

	audio::get_error();
	alGenBuffers(pool_size, m_buffers.data());
	e = audio::get_error();
	assert(e == AL_NO_ERROR);

	m_buffer_queue = std::queue<ALuint>({m_buffers.begin(), m_buffers.end()});
}

audio::source::source(std::shared_ptr<audio::stream> str, int pool_size)
: source(pool_size) {
	m_stream = str;
}

audio::source::~source() {
	if (m_source) {
		audio::get_error();

		alDeleteBuffers(m_buffers.size(), m_buffers.data());
		auto e = audio::get_error();
		assert(e == AL_NO_ERROR);

		alDeleteSources(1, &m_source);
		e = audio::get_error();
		assert(e == AL_NO_ERROR);
	}
}

audio::source::source(source&& other)
: m_source{other.m_source}, m_buffer_queue(std::move(other.m_buffer_queue)), m_buffers(std::move(other.m_buffers)) {
	other.m_source = 0;
}

audio::source& audio::source::operator =(audio::source&& other) {
	if (m_source) {
		audio::get_error();

		alDeleteBuffers(m_buffers.size(), m_buffers.data());
		auto e = audio::get_error();
		assert(e == AL_NO_ERROR);
	
		alDeleteSources(1, &m_source);
		e = audio::get_error();
		assert(e == AL_NO_ERROR);
	}
	
	m_source = other.m_source;
	m_buffer_queue = std::move(other.m_buffer_queue);
	m_buffers = std::move(other.m_buffers);

	other.m_source = 0;
	return *this;
}

audio::source::operator ALuint () {
	return m_source;
}

std::tuple<size_t, ALenum> audio::source::dequeue_buffers(bool should_wait) {
	ALint processed = 0;

	do {
		// In case this is offput to another thread
		std::this_thread::yield();

		auto [proc, error] = this->get<ALint>(AL_BUFFERS_PROCESSED);
		processed = proc;
		if (error != AL_NO_ERROR)
			return std::make_tuple(0, error);
	} while (processed == 0 && should_wait);

	ALuint done;
	for (int i = 0; i < processed; ++i) {
		audio::get_error();
		alSourceUnqueueBuffers(m_source, 1, &done);
		auto err = audio::get_error();

		if (err != AL_NO_ERROR)
			return std::make_tuple(0, err);

		m_buffer_queue.push(done);
	}

	return std::make_tuple(processed, AL_NO_ERROR);
}

//------------------------------------------------------------------------------
// Audio
//------------------------------------------------------------------------------
ALenum audio::get_error() {
	return alGetError();
}

audio::audio(device& d)
: m_sources{}, m_should_run{true} {
	// Clear previous errors
	audio::get_error();

	alDistanceModel(AL_EXPONENT_DISTANCE);

	// Create context
	m_context = alcCreateContext(d, nullptr);
	assert(m_context);

	auto result = this->bind();
	assert(result);

	ALCint nattrs;
	alcGetIntegerv(d, ALC_ATTRIBUTES_SIZE, 1, &nattrs);

	std::vector<ALCint> attrs(nattrs * 2 + 1);
	alcGetIntegerv(d, ALC_ALL_ATTRIBUTES, nattrs * 2, attrs.data());
	attrs.back() = 0;

	m_freq = 0;
	for (auto i = 0u; i != nattrs; ++i)
		if (attrs[i*2] == ALC_FREQUENCY)
			m_freq = attrs[i*2+1];
	
	assert(m_freq != 0);

	// Watch for refills
	m_filler = std::thread([&]() {
		while (m_should_run) {
			for (auto& s : m_sources) {
				s->dequeue_buffers();
			}

			std::this_thread::yield();
		}
	});
}

audio::~audio() {
	if (m_context) {
		this->unbind();
		alcDestroyContext(m_context);

		m_should_run = false;
	}
}

audio::audio(audio&& other)
: m_context{other.m_context}, m_sources{other.m_sources}, m_should_run{other.m_should_run} {
	other.m_context = nullptr;
	other.m_should_run = false;

	m_filler = std::thread([&]() {
		while (m_should_run) {
			for (auto& s : m_sources) {
				s->dequeue_buffers();
			}

			std::this_thread::yield();
		}
	});
}

audio& audio::operator =(audio&& other) {
	auto was_bound = false;
	if (m_context) {
		was_bound = this->unbind();
		alcDestroyContext(m_context);
	}

	m_context = other.m_context;
	m_sources = other.m_sources;
	m_should_run = other.m_should_run;

	other.m_context = nullptr;
	other.m_should_run = false;

	m_filler = std::thread([&]() {
		while (m_should_run) {
			for (auto& s : m_sources) {
				s->dequeue_buffers();
			}

			std::this_thread::yield();
		}
	});

	if (was_bound)
		this->bind();

	return *this;
}

bool audio::bind() {
	return alcMakeContextCurrent(m_context);
}

bool audio::unbind() {
	if (alcGetCurrentContext() != m_context)
		return false;

	alcMakeContextCurrent(nullptr);
	return true;
}

std::vector<std::string> audio::enumerate_devices() {
	auto allowed = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
	if (allowed) {
		auto* raw = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
		std::string dev(raw);
		std::vector<std::string> res;

		while (dev.length() != 0) {
			res.push_back(dev);

			raw += dev.length() + 1;
			dev = std::string(raw);
		}

		return res;
	} else {
		throw std::runtime_error("Enumeration is not allowed.");
	}
}

std::shared_ptr<audio::source> audio::attach_source(int pool_size) {
	auto s = std::make_shared<source>(pool_size);
	m_sources.push_back(s);

	return s;
}

std::shared_ptr<audio::source> audio::attach_source(std::shared_ptr<audio::stream> str, int pool_size) {
	auto s = std::make_shared<source>(str, pool_size);
	m_sources.push_back(s);

	return s;
}

size_t audio::get_frequency() const {
	return m_freq;
}

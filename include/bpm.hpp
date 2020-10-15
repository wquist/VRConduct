#pragma once

#include <vector>
#include <glm/glm.hpp>

class bpm {
private:
	const int BUF_SIZE = 10;
	const int BPM_SIZE = 4;

	std::vector<float> m_slopes;
	std::vector<float> m_bpms;

	glm::vec3 m_prev_pos;
	float     m_prev_slope;
	float     m_prev_time;
	float     m_delta;
	int       m_index;
	int       m_bpm_index;

	size_t m_bpm;

public:
	bpm(const size_t bpm) : m_bpm(bpm) {
		// Resize the vectors
		m_slopes.resize(BUF_SIZE);
		m_bpms.resize(BPM_SIZE);

		// Initialize values needed
		m_prev_pos   = glm::vec3{0.0f};
		m_prev_slope = 0.0f;
		m_prev_time  = 0.0f;
		m_delta      = 0.0f;
		m_index      = 0;
		m_bpm_index  = 0;
	}

	~bpm() = default;

	// Register the new bpm based on a given position and time
	void update(const glm::vec3& pos, const float time) {
		m_delta += time - m_prev_time;
		m_prev_time = time;
		glm::vec3 t_pos = pos;

		if (m_delta < 0.005f)
			return;

		if (glm::distance(t_pos, m_prev_pos) < 0.05f)
			t_pos = m_prev_pos;

		m_slopes[m_index] = (t_pos.y - m_prev_pos.y) / m_delta;
		
		// Get the new averaged slope
		float slope = 0;
		for (auto& sl : m_slopes)
			slope += sl;
		slope /= BUF_SIZE;

		// Trigger bpm on negative to positive slope
		if (m_prev_slope < 0.0f && slope > 0.0f) {
			m_bpms[m_bpm_index] = 60.0f / m_delta;

			float bpm = 0.0f;
			for (auto& bp : m_bpms)
				bpm += bp;
			m_bpm = (size_t) (bpm / BPM_SIZE);

			// Update the bpm index
			m_bpm_index = (m_bpm_index + 1) % BPM_SIZE;
			m_delta = 0.0f;
		}

		// Update references to past values
		m_prev_pos   = t_pos;
		m_prev_slope = slope;
		m_index      = (m_index + 1) % BUF_SIZE;
	}

	size_t get_bpm() const { return m_bpm; }
};
#include <helper/vr_controller.hpp>

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace {
	glm::mat4 make_mat4(const vr::HmdMatrix34_t& mat)
	{
		return glm::mat4{
			mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0,
			mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0,
			mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0,
			mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f
		};
	}
}

namespace helper {
	vr_controller::vr_controller(vr::IVRSystem* hmd, vr::ETrackedControllerRole role)
	: m_hmd{hmd}, m_role{role}, m_index{vr::k_unTrackedDeviceIndexInvalid} {
		this->update({});
	}

	void vr_controller::update(const std::vector<vr::TrackedDevicePose_t>& poses) {
		if (m_index == vr::k_unTrackedDeviceIndexInvalid) {
			for (int i = vr::k_unTrackedDeviceIndex_Hmd + 1; i < vr::k_unMaxTrackedDeviceCount; ++i) {
				if (m_hmd->GetTrackedDeviceClass(i) != vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
					continue;
				if (m_hmd->GetControllerRoleForTrackedDeviceIndex(i) == m_role) {
					m_index = i;
					break;
				}
			}
		}

		if (m_index == vr::k_unTrackedDeviceIndexInvalid)
			return;
		if (!poses.size())
			return;
		if (!m_hmd->IsTrackedDeviceConnected(m_index))
			return;

		vr::VRControllerState_t state;
		m_hmd->GetControllerState(m_index, &state, sizeof(state));

		for (auto& h : m_handlers) {
			bool old = h.second.is_pressed;
			h.second.is_pressed = (h.first & state.ulButtonPressed) || (h.first & state.ulButtonTouched);
			if (h.second.is_pressed != old)
				h.second.callback(h.second.is_pressed);
		}

		if (poses[m_index].bPoseIsValid)
			m_model = make_mat4(poses[m_index].mDeviceToAbsoluteTracking);
	}

	uint64_t vr_controller::get_index() const {
		return m_index;
	}

	glm::mat4 vr_controller::get_model_matrix() const {
		return m_model;
	}

	void vr_controller::attach_handler(vr::EVRButtonId id, handler h) {
		m_handlers.try_emplace(vr::ButtonMaskFromId(id), h, false);
	}

	std::optional<glm::vec2> vr_controller::ray_intersect(const glm::vec3& origin, const glm::vec3& dir, const glm::vec3& pos, const glm::vec3& norm) {
		auto df = pos - origin;
		auto d = glm::dot(norm, df);
		auto e = glm::dot(norm, dir);

		if (std::abs(e) < std::numeric_limits<float>::epsilon())
			return {};
		
		auto point = origin + dir * d / e;

		glm::vec3 basis[3] = {{}, {}, {}};
		basis[0] = norm;

		int mi = 0; float mv = 0.f;
		for (auto i = 0u; i != 3; ++i) {
			auto v = std::abs(basis[0][i]);
			if (v > mv) {
				mi = i;
				mv = v;
			}
		}

		basis[1][(mi + 1) % 3] = 1.f;
		basis[2][(mi + 2) % 3] = 1.f;

		for (auto i = 0u; i != 3; ++i) {
			for (auto j = 0u; j != i; ++j) {
				float f = glm::dot(basis[j], basis[i]) / glm::dot(basis[j], basis[j]);

				glm::vec3 p = basis[j] * f;
				basis[i] -= p;
			}

			basis[i] = glm::normalize(basis[i]);
		}

		return glm::vec2(glm::dot(point, basis[1]), glm::dot(point, basis[2]));
	}
}
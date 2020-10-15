#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <map>
#include <openvr.h>
#include <optional>

namespace helper {
	class vr_controller {
	public:
		using handler = std::function<void(bool)>;

		struct button {
			button(handler c, bool p)
			: callback{c}, is_pressed{p} {}

			handler callback;
			bool is_pressed;
		};

	public:
		vr_controller(vr::IVRSystem* hmd, vr::ETrackedControllerRole role);
		~vr_controller() = default;

	public:
		void update(const std::vector<vr::TrackedDevicePose_t>& poses);
	
	public:
		uint64_t get_index() const;
		glm::mat4 get_model_matrix() const;
	
	public:
		void attach_handler(vr::EVRButtonId id, handler h);
		std::optional<glm::vec2> ray_intersect(const glm::vec3& origin, const glm::vec3& dir, const glm::vec3& pos, const glm::vec3& norm);

	private:
		vr::IVRSystem* m_hmd;
		vr::ETrackedControllerRole m_role;

		vr::TrackedDeviceIndex_t m_index;
		std::map<uint64_t, button> m_handlers;

		glm::mat4 m_model;
	};
}
#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <queue>
#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

#include <openvr.h>

#include <heatsink/glfw.hpp>
#include <heatsink/window.hpp>
#include <heatsink/array_buffer.hpp>
#include <heatsink/shader.hpp>
#include <heatsink/texture.hpp>
#include <heatsink/vertex_array.hpp>
#include <heatsink/framebuffer.hpp>

#include <helper/stb.hpp>
#include <helper/assimp.hpp>
#include <helper/vr_controller.hpp>

#include "model.hpp"
#include "instrument.hpp"
#include "bpm.hpp"

#include <audio.hpp>
#include <score.hpp>

using namespace std::string_literals;

glm::mat4 make_mat4(const vr::HmdMatrix34_t& mat)
{
	return glm::mat4{
		mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0,
		mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0,
		mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0,
		mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f
	};
}

glm::mat4 make_mat4(const vr::HmdMatrix44_t& mat)
{
	return glm::mat4{
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	};
}

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::clog << "usage: " << argv[0] << " path/to/data/dir" << std::endl;
		return 1;
	}

	auto path = std::string(argv[1]);

	std::cout << "Devices:" << std::endl;
	for (const auto& d : audio::enumerate_devices())
		std::cout << "\tAUD: " << d << std::endl;

	// Set up OpenAL
	auto dev = audio::device("OpenAL Soft on HTC-VIVE-C (NVIDIA High Definition Audio)");
	auto aud = audio(dev);
	std::cout << "Using frequency: " << aud.get_frequency() << std::endl;

	// Set up the TSF library
	score s(
		path + "/music/autumn/autumn.mid",
		path + "/soundfont/fluid.sf2",
		aud.get_frequency()
	);

	auto cs = s.get_channels();
	std::map<size_t, std::pair<std::shared_ptr<score::channel>, std::shared_ptr<audio::source>>> mappings;
	for (auto& c : cs) {
		mappings.emplace(c.first, std::make_pair(c.second, aud.attach_source(c.second)));
	}

	bool should_run(true);

	auto get_channel = [](auto a) { return a.second.first;  };
	auto get_source  = [](auto a) { return a.second.second; };

	// Fill buffers
	auto f = std::thread([&]() {
		while (should_run) {
			if (!s.is_playing()) {
				std::this_thread::yield();
				continue;
			}

			for (auto& m : mappings) {
				get_source(m)->queue_buffer([&](auto b) {
					static score::channel_sample data[audio::AUDIO_SIZE];
					get_channel(m)->read(&data, audio::AUDIO_SIZE);

					audio::get_error();
					alBufferData(b, AL_FORMAT_MONO16, data, audio::AUDIO_SIZE * sizeof(score::channel_sample), aud.get_frequency());

					auto e = audio::get_error();
					if (e != AL_NO_ERROR) std::cout << alGetString(e) << ": " << (int)alIsBuffer(b) << std::endl;
					assert(e == AL_NO_ERROR);
				});
			}

			std::this_thread::yield();
		}
	});

	// Generate audio
	auto w = std::thread([&]() {
		while (should_run) {
			if (!s.is_playing()) {
				std::this_thread::yield();
				continue;
			}

			for (auto& m : mappings) {
				auto[state, error] = get_source(m)->get<ALint>(AL_SOURCE_STATE);
				assert(error == AL_NO_ERROR);

				if (state != AL_PLAYING) {
					audio::get_error();
					alSourcePlay(*get_source(m));
					auto e = audio::get_error();
					assert(e == AL_NO_ERROR);
				}
			}

			std::this_thread::yield();
		}
	});

	vr::EVRInitError vr_error;
	auto* hmd = vr::VR_Init(&vr_error, vr::EVRApplicationType::VRApplication_Scene);
	assert(vr_error == vr::VRInitError_None);

	auto* vr_compositor = vr::VRCompositor();
	assert(vr_compositor);

	unsigned int hmd_width, hmd_height;
	hmd->GetRecommendedRenderTargetSize(&hmd_width, &hmd_height);

	// Get controllers
	helper::vr_controller left_controller(hmd, vr::TrackedControllerRole_LeftHand);
	helper::vr_controller right_controller(hmd, vr::TrackedControllerRole_RightHand);

	hs::settings settings(4, 0);
	hs::backend::init(settings);

	hs::backend::window window("conductor", {hmd_width/2.f, hmd_height/2.f});
	auto& context = window.get_context();

	std::vector<char> presets(256);
	std::iota(presets.begin(), presets.end(), 0);
	instrument::init(context, presets);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	hs::shader shader(context,
	{
		path + "/shader/simple.vert"s,
		path + "/shader/simple.frag"s
	});

	shader.declare_uniform("M", glm::mat4(1.f));
	shader.declare_uniform<glm::mat4>("VP");
	shader.declare_uniform<glm::mat4>("ShadowVP");
	shader.declare_uniform<glm::vec3>("Color");
	shader.declare_uniform<glm::vec3>("Selected");
	shader.declare_uniform("Shadow", 0);

	hs::shader shadow_shader(context,
	{
		path + "/shader/shadow.vert"s,
		path + "/shader/shadow.frag"s
	});

	shadow_shader.declare_uniform("M", glm::mat4(1.0f));
	shadow_shader.declare_uniform<glm::mat4>("VP");

	hs::shader skybox_shader(context,
	{
		path + "/shader/skybox.vert"s,
		path + "/shader/skybox.frag"s
	});

	skybox_shader.declare_uniform<glm::mat4>("VP");
	skybox_shader.declare_uniform("Skybox", 0);

	hs::shader quad_shader(context,
	{
		path + "/shader/quad.vert"s,
		path + "/shader/quad.frag"s
	});

	quad_shader.declare_uniform("Texture", 0);

	hs::shader page_shader(context,
	{
		path + "/shader/page.vert"s,
		path + "/shader/page.frag"s
	});

	page_shader.declare_uniform<glm::mat4>("M");
	page_shader.declare_uniform<glm::mat4>("VP");
	page_shader.declare_uniform("Texture", 0);
	page_shader.declare_uniform<glm::mat4>("ShadowVP");
	page_shader.declare_uniform("Shadow", 1);

	hs::framebuffer left_eye_buffer(context, {hmd_width, hmd_height});
	left_eye_buffer.set_attachments({GL_RGB}, GL_DEPTH_COMPONENT24);
	left_eye_buffer.get_color(0).set_filter(GL_LINEAR, GL_LINEAR);

	hs::framebuffer right_eye_buffer(context, {hmd_width, hmd_height});
	right_eye_buffer.set_attachments({GL_RGB}, GL_DEPTH_COMPONENT24);
	right_eye_buffer.get_color(0).set_filter(GL_LINEAR, GL_LINEAR);

	hs::framebuffer shadow_buffer(context, {4096, 4096});
	shadow_buffer.set_attachments({}, GL_DEPTH_COMPONENT24);
	shadow_buffer.get_depth().set_filter(GL_LINEAR, GL_LINEAR);

	auto stage = model::load_model(context, path + "/model/stage2.obj");
	stage->position({0.f, 0.f, 0.f});

	auto baton = model::load_model(context, path + "/model/baton2.obj");
	auto select_baton = model::load_model(context, path + "/model/baton3.obj");
	auto skybox = model::load_model(context, path + "/model/cube.obj");

	auto stand = model::load_model(context, path + "/model/stand.obj");
	stand->position({0.0f, 0.0f, 0.0f});

	std::vector<glm::vec2> quad_verts =
	{
		{ -1.f,  1.f },
		{  1.f, -1.f },
		{ -1.f, -1.f },
		{ -1.f,  1.f },
		{  1.f,  1.f },
		{  1.f, -1.f }
	};

	// 8.5 x 11 inch paper
	std::vector<glm::vec3> page_verts = {
		{ -0.10795f, 0.f,  0.1397f },
		{  0.10795f, 0.f, -0.1397f },
		{ -0.10795f, 0.f, -0.1397f },
		{ -0.10795f, 0.f,  0.1397f },
		{  0.10795f, 0.f,  0.1397f },
		{  0.10795f, 0.f, -0.1397f }
	};

	std::vector<glm::vec2> page_coords = {
		{ 0, 1 },
		{ 1, 0 },
		{ 0, 0 },
		{ 0, 1 },
		{ 1, 1 },
		{ 1, 0 }
	};

	std::vector<glm::vec3> page_norms = {
		{0, 1, 0},
		{0, 1, 0},
		{0, 1, 0},
		{0, 1, 0},
		{0, 1, 0},
		{0, 1, 0}
	};

	hs::array_buffer quad_buf(context, GL_ARRAY_BUFFER, quad_verts);

	hs::vertex_array quad(context);
	quad.set_attribute(0, quad_buf);

	hs::array_buffer page_buf(context, GL_ARRAY_BUFFER, page_verts);
	hs::array_buffer page_coord(context, GL_ARRAY_BUFFER, page_coords);
	hs::array_buffer page_norm(context, GL_ARRAY_BUFFER, page_norms);

	hs::vertex_array page(context);
	page.set_attribute(0, page_buf);
	page.set_attribute(1, page_coord);
	page.set_attribute(2, page_norm);

	auto page_img = helper::stb_image(path + "/music/con1/con1-1.png", GL_RGBA);
	auto page_texture = std::make_shared<hs::texture2>(context, GL_TEXTURE_2D, page_img.size());
	page_texture->set_filter(GL_LINEAR, GL_LINEAR);

	hs::texture2::target page_target(GL_RGBA);
	page_texture->update(GL_RGBA, page_target, page_img.data, page_img.data + page_img.data_size());

	const std::vector<std::string> skybox_images =
	{
		path + "/texture/sky/right.tga",
		path + "/texture/sky/left.tga",
		path + "/texture/sky/top.tga",
		path + "/texture/sky/bottom.tga",
		path + "/texture/sky/back.tga",
		path + "/texture/sky/front.tga"
	};

	hs::texture2 skybox_texture(context, GL_TEXTURE_CUBE_MAP, {1024, 1024});
	skybox_texture.set_filter(GL_LINEAR, GL_LINEAR);

	auto face = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
	for (auto& i : skybox_images)
	{
		auto img = helper::stb_image(i, GL_RGB);

		hs::texture2::target target(GL_RGB, 0, face++);
		skybox_texture.update(GL_RGB, target, img.data, img.data + img.data_size());
	}

	std::map<size_t, instrument> instruments;
	for (auto& m : mappings)
	{
		instrument inst(context, get_channel(m)->get_preset_number(), get_source(m));
		auto t = 2 * 3.14f * instruments.size() / mappings.size();
		inst.position(glm::vec3{2 * std::sinf(t), 0.0f, 2 * std::cosf(t)});

		instruments.emplace(get_channel(m)->get_preset_number(), inst);
	}

	bpm current_bpm(80);
	bool playing = false;

	auto camera_proj = glm::perspective(glm::radians(60.f), 1024.f / 768.f, 0.1f, 20.f);
	glm::vec3 camera_position{-1.f, 1.f, 0.f};
	float camera_angle{0.f};

	auto shadow_proj = glm::ortho(-15.f, 15.f, -15.f, 15.f, 0.f, 30.f);
	auto shadow_position = glm::vec3(15.f) * glm::normalize(glm::vec3{-0.5f, 1.f, -1.f});
	auto shadow_view = glm::lookAt(shadow_position, glm::vec3{}, glm::vec3{0.f, 1.f, 0.f});

	auto flip_model = glm::scale(glm::mat4(1.f), glm::vec3{-1.f, 1.f, -1.f});

	glm::vec3 camera_up{ 0.f, 1.f, 0.f };
	glm::vec3 camera_forward{ 0.f, 0.f, 1.f };

	std::vector<vr::TrackedDevicePose_t> poses(vr::k_unMaxTrackedDeviceCount);

	bool select_mode = false;
	glm::mat4 hand_view;
	glm::vec3 hand_pos{};

	glm::mat4 world_scale(1.f);

	// Callbacks
	instrument* selected = nullptr;
	right_controller.attach_handler(vr::EVRButtonId::k_EButton_SteamVR_Trigger, [&](bool pressed) {
		if (pressed) {
			if (!select_mode) {
				s.play();

				return;
			}

			for (auto& inst : instruments)
			{
				auto r = glm::distance(hand_pos + glm::mat3(hand_view) * glm::vec3(0.f, 0.f, -0.3f), glm::vec3(world_scale * glm::vec4(inst.second.position(), 1)));
				if (r < 0.1f)
				{
					selected = &inst.second;

					inst.second.selected(true);
					break;
				}
			}
		} else {
			if (selected) {
				selected->selected(false);
				selected = nullptr;
			}
		}
	});

	right_controller.attach_handler(vr::EVRButtonId::k_EButton_ApplicationMenu, [&](bool pressed) {
		if (pressed)
			select_mode = !select_mode;
	});

	constexpr float scale_factor = 15.f;
	while (window.refresh())
	{
		vr_compositor->WaitGetPoses(poses.data(), vr::k_unMaxTrackedDeviceCount, nullptr, 0);

		auto camera_view = glm::mat4(1.f);
		if (poses[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
			camera_view = glm::inverse(make_mat4(poses[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking));

		left_controller.update(poses);
		right_controller.update(poses);

		hand_view = right_controller.get_model_matrix();
		hand_pos = glm::vec3(hand_view * glm::vec4{ 0,0,0,1 });

		world_scale = glm::mat4(1.0f);
		auto inverse_scale = glm::mat4(1.f);
		if (select_mode) {
			auto standp = stand->position();
			standp.y += 0.85f;

			world_scale = glm::scale(glm::rotate(glm::translate(standp), glm::radians(20.f), glm::vec3{1.f, 0.f, 0.f}), glm::vec3(1.f / scale_factor));
			inverse_scale = glm::inverse(world_scale);
		}

		if (selected) {
			auto h = hand_pos * scale_factor + glm::mat3(hand_view) * glm::vec3(0.f, 0.f, -0.3f) * scale_factor;
			auto p = selected->position();

			selected->position(glm::vec3{h.x, p.y, h.z});
		} else {
			current_bpm.update(hand_pos + glm::mat3(hand_view) * glm::vec3(0.f, 0.f, -0.3f), glfwGetTime());
			// Inform channel streams
		}

		auto left_proj = make_mat4(hmd->GetProjectionMatrix(vr::EVREye::Eye_Left, 0.1f, 20.f));
		auto left_view = glm::inverse(make_mat4(hmd->GetEyeToHeadTransform(vr::EVREye::Eye_Left))) * camera_view;
		auto right_proj = make_mat4(hmd->GetProjectionMatrix(vr::EVREye::Eye_Right, 0.1f, 20.f));
		auto right_view = glm::inverse(make_mat4(hmd->GetEyeToHeadTransform(vr::EVREye::Eye_Right))) * camera_view;

		auto cam_pos = glm::vec3(glm::inverse(camera_view) * glm::vec4{0,0,0,1});
		auto cam_look = glm::mat3(camera_view) * camera_forward;
		auto cam_norm = glm::mat3(camera_view) * camera_up;

		ALfloat o[] = {cam_look.x, cam_look.y, -cam_look.z, -cam_norm.x, cam_norm.y, -cam_norm.z};

		alListener3f(AL_POSITION, cam_pos.x, cam_pos.y, cam_pos.z);
		alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
		alListenerfv(AL_ORIENTATION, o);

		glClearColor(0.f, 0.f, 0.f, 1.f);

		shadow_shader.bind();
		{
			shadow_buffer.bind();
			glViewport(0, 0, 4096, 4096);

			glClear(GL_DEPTH_BUFFER_BIT);
			glCullFace(GL_FRONT);

			shadow_shader.set_uniform("VP", shadow_proj * shadow_view);
			stage->draw(shadow_shader, { "M" });
			if (!select_mode)
				stand->draw(shadow_shader, { "M" });

			shadow_shader.set_uniform("M", hand_view * flip_model);
			baton->draw(shadow_shader, {});

			shadow_shader.set_uniform("VP", shadow_proj * shadow_view);
			for (auto& inst : instruments)
				inst.second.draw(shadow_shader, { "M" });

			if (select_mode) {
				shadow_shader.set_uniform("VP", shadow_proj * shadow_view * world_scale);

				stage->draw(shadow_shader, { "M" });
				stand->draw(shadow_shader, { "M" });

				for (auto& inst : instruments)
					inst.second.draw(shadow_shader, { "M" });

				shadow_shader.set_uniform("M", hand_view * flip_model);
				shadow_shader.set_uniform("VP", shadow_proj * shadow_view * inverse_scale);
				baton->draw(shadow_shader, {});
			}

			glCullFace(GL_BACK);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0.f, 0.f, window.framebuffer_size().x, window.framebuffer_size().y);
		}

		auto render_view = [&](const glm::mat4& proj, const glm::mat4& view)
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			skybox_shader.bind();
			{
				glDepthMask(GL_FALSE);

				skybox_shader.set_uniform("VP", proj * glm::mat4(glm::mat3(view)));
				skybox_texture.bind_to(0);

				skybox->draw(skybox_shader, {});

				glDepthMask(GL_TRUE);
			}

			shader.bind();
			{
				shader.set_uniform("VP", proj * view);
				shader.set_uniform("ShadowVP", shadow_proj * shadow_view);
				shadow_buffer.get_depth().bind_to(0);

				stage->draw(shader, { "M", "Color" });
				if (!select_mode)
					stand->draw(shader, { "M", "Color" });

				shader.set_uniform("M", hand_view * flip_model);
				baton->draw(shader, { "Color" });

				shader.set_uniform("VP", proj * view);
				shader.set_uniform("ShadowVP", shadow_proj * shadow_view);
				for (auto& inst : instruments)
					inst.second.draw(shader, { "M", "Color" });

				if (select_mode) {
					shader.set_uniform("VP", proj * view * world_scale);
					shader.set_uniform("ShadowVP", shadow_proj * shadow_view * world_scale);

					stage->draw(shader, { "M", "Color" });
					stand->draw(shader, { "M", "Color" });

					for (auto& inst : instruments)
						inst.second.draw(shader, { "M", "Color" });

					shader.set_uniform("M", hand_view * flip_model);
					shader.set_uniform("VP", proj * view * inverse_scale);
					shader.set_uniform("ShadowVP", shadow_proj * shadow_view * inverse_scale);
					baton->draw(shader, { "Color" });
				}
			}

			page_shader.bind();
			{
				auto standp = stand->position();
				standp.y += 0.85f;

				auto m = glm::rotate(glm::translate(standp), glm::radians(20.f), glm::vec3{1.f, 0.f, 0.f});
				page_shader.set_uniform("M", m);
				page_shader.set_uniform("VP", proj * view * world_scale);
				page_shader.set_uniform("ShadowVP", shadow_proj * shadow_view * world_scale);

				page_texture->bind_to(0);
				shadow_buffer.get_depth().bind_to(1);

				page.bind();
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		};

		glViewport(0.f, 0.f, hmd_width, hmd_height);
		left_eye_buffer.bind();
		render_view(left_proj, left_view);
		right_eye_buffer.bind();
		render_view(right_proj, right_view);

		glViewport(0, 0, hmd_width/2.f, hmd_height/2.f);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		quad_shader.bind();
		{
			left_eye_buffer.get_color(0).bind_to(0);

			quad.bind();
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		vr::Texture_t left_info{(void*)left_eye_buffer.get_color(0).name(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma};
		vr::Texture_t right_info{(void*)right_eye_buffer.get_color(0).name(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma};

		vr_compositor->Submit(vr::EVREye::Eye_Left, &left_info);
		vr_compositor->Submit(vr::EVREye::Eye_Right, &right_info);
	}

	should_run = false;
	f.join();
	w.join();

	// Stop playback
	s.stop();

	vr::VR_Shutdown();
	return 0;
}
#include "klib/string/fixed_string.hpp"
#include "kvf/aspect_resize.hpp"
#include "le2d/console/junction.hpp"
#include "le2d/console/terminal.hpp"
#include "le2d/console/terminal_builder.hpp"
#include "le2d/context.hpp"
#include "le2d/drawable/shape.hpp"
#include "le2d/drawable/text.hpp"
#include "le2d/file_data_loader.hpp"
#include "le2d/input/action_mapping.hpp"
#include <imgui.h>
#include <print>

namespace example {
namespace {
auto draw_stats(le::Context& context) -> bool {
	auto const& frame_stats = context.get_frame_stats();
	auto const& render_stats = context.get_renderer().get_stats();
	auto ret = true;
	ImGui::SetNextWindowSize({300.0f, 200.0f});
	ImGui::Begin("Stats", &ret);
	ImGui::TextUnformatted(klib::FixedString{"fps: {}", frame_stats.framerate}.c_str());
	ImGui::TextUnformatted(klib::FixedString{"total dt: {:.1f}ms", 1000.0f * frame_stats.total_dt.count()}.c_str());
	ImGui::TextUnformatted(klib::FixedString{"frame dt: {:.1f}ms", 1000.0f * frame_stats.frame_dt.count()}.c_str());
	ImGui::TextUnformatted(klib::FixedString{"frames: {}", frame_stats.total_frames}.c_str());
	ImGui::TextUnformatted(klib::FixedString{"draws: {}", render_stats.draw_calls}.c_str());
	ImGui::TextUnformatted(klib::FixedString{"tris: {}", render_stats.triangles}.c_str());
	ImGui::Separator();

	auto const supported_vsync = context.get_supported_vsync();
	auto const vsync = context.get_vsync();
	if (ImGui::BeginCombo("vsync", le::vsync_name_map.to_name(vsync).data())) {
		for (auto const v : supported_vsync) {
			if (ImGui::Selectable(le::vsync_name_map.to_name(v).data(), v == vsync)) { context.set_vsync(v); }
		}
		ImGui::EndCombo();
	}
	ImGui::End();
	return ret;
}

void run() {
	// all members are constexpr-friendly.
	static constexpr auto context_create_info_v = le::Context::CreateInfo{
		.window = le::WindowInfo{.size = {800, 600}, .title = "My Window"},
	};
	// create a Context instance.
	auto context = le::Context::create(context_create_info_v);

	// create a DeltaTime instance.
	auto delta_time = kvf::DeltaTime{};

	// create a FileDataLoader instance, mounting the assets directory.
	auto const data_loader = le::FileDataLoader{"assets"};

	// create an AssetLoader instance to load shared resources.
	auto const asset_loader = context->create_asset_loader(&data_loader);

	auto const font = asset_loader.load<le::IFont>("fonts/Vera.ttf");
	if (!font) { throw std::runtime_error{"Failed to load Font."}; }

	auto const texture = asset_loader.load<le::ITexture>("images/awesomeface.png");
	if (!texture) { throw std::runtime_error{"Failed to load Texture."}; }

	auto const audio_buffer = asset_loader.load<le::IAudioBuffer>("audio/explode.wav");
	if (!audio_buffer) { throw std::runtime_error{"Failed to load Audio Buffer."}; }

	auto const mono_font = asset_loader.load<le::IFont>("fonts/mono.ttf");

	// the waiter blocks on destruction until the context is idle,
	// after which the loaded resources can be safely destroyed.
	auto const waiter = context->create_waiter();

	auto terminal = std::unique_ptr<le::console::ITerminal>{};
	auto show_stats = le::Tweakable<bool>{};
	if (mono_font) {
		terminal = le::console::TerminalBuilder{}.build(mono_font.get());
		terminal->add_tweakable("example.show_stats", &show_stats);
	}

	auto input_router = le::input::Router{};
	auto junction = le::console::Junction{&input_router, terminal.get()};

	auto action_mapping = le::input::ActionMapping{};
	input_router.push_mapping(&action_mapping);

	auto exit_action = le::input::action::KeyDigital{GLFW_KEY_ESCAPE};
	action_mapping.bind_action(&exit_action, [&](le::input::action::Value const& v) {
		if (v.get<bool>()) { context->set_window_should_close(); }
	});

	// store playback trigger data.
	auto audio_wait = kvf::Seconds{2.0f};
	auto audio_played = false;

	// create a quad.
	auto quad = le::drawable::Quad{};
	quad.geometry.create({100.0f, 100.0f});
	// reposition it and set the loaded texture.
	quad.instance.transform.position.y -= 30.0f;
	quad.texture = texture.get();

	// create a Text instance.
	auto text = le::drawable::Text{};
	text.set_string(*font, "hello from le2d!");
	// reposition and tint it.
	text.instance.transform.position.y += 30.0f;
	text.instance.tint = kvf::yellow_v;

	// loop while context is running (ie, window is open).
	while (context->is_running()) {
		// start the next frame. This polls events and waits for any
		// previous renders on the virtual frame to complete.
		context->next_frame();

		// compute the delta time (in float seconds).
		auto const dt = delta_time.tick();

		// update audio playback.
		audio_wait -= dt;
		if (audio_wait < 0s && !audio_played) {
			// play the loaded audio buffer.
			context->get_audio_mixer().play_sfx(audio_buffer.get());
			audio_played = true;
		}

		junction.dispatch(context->event_queue(), context->framebuffer_size());

		if (show_stats.get_value()) { show_stats.set_value(draw_stats(*context)); }

		if (terminal) { terminal->tick(dt); }

		// begin the primary render pass.
		auto& renderer = context->begin_render();
		// draw quad.
		quad.draw(renderer);
		// draw text.
		text.draw(renderer);

		if (terminal) { terminal->draw(renderer); }

		// end pass and submit the frame for presentation.
		context->present();
	}
}
} // namespace
} // namespace example

auto main() -> int {
	try {
		example::run();
	} catch (std::exception const& e) {
		std::println(stderr, "PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println(stderr, "PANIC!");
		return EXIT_FAILURE;
	}
}

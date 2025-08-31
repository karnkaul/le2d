#include <le2d/context.hpp>
#include <le2d/drawable/shape.hpp>
#include <le2d/drawable/text.hpp>
#include <le2d/file_data_loader.hpp>
#include <print>

namespace example {
namespace {
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

	// the waiter blocks on destruction until the context is idle,
	// after which the loaded resources can be safely destroyed.
	auto const waiter = context->create_waiter();

	// store playback trigger data.
	auto audio_wait = kvf::Seconds{2.0f};
	auto audio_played = false;

	// create a quad.
	auto quad = le::drawable::Quad{};
	quad.create({100.0f, 100.0f});
	// reposition it and set the loaded texture.
	quad.transform.position.y -= 30.0f;
	quad.texture = texture.get();

	// create a Text instance.
	auto text = le::drawable::Text{};
	text.set_string(*font, "hello from le2d!");
	// reposition and tint it.
	text.transform.position.y += 30.0f;
	text.tint = kvf::yellow_v;

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

		for (auto const& event : context->event_queue()) {
			// handle events here.
			// for example, if you want to close the window on Escape key press:
			if (auto const* key_event = std::get_if<le::event::Key>(&event)) {
				if (key_event->key == GLFW_KEY_ESCAPE && key_event->action == GLFW_PRESS) {
					context->set_window_close(); // set the close flag.
				}
			}
		}

		// begin the primary render pass.
		auto& renderer = context->begin_render();
			// draw quad.
			quad.draw(renderer);
			// draw text.
			text.draw(renderer);

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

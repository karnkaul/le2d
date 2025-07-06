#include <le2d/asset_loader.hpp>
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
		.window = le::WindowInfo{.size = {400, 300}, .title = "My Window"},
	};
	// create a Context instance.
	auto context = le::Context{context_create_info_v};

	// create a DeltaTime instance.
	auto delta_time = kvf::DeltaTime{};

	// create a FileDataLoader instance, mounting the assets directory.
	auto const data_loader = le::FileDataLoader{"assets"};

	// create an AssetLoader instance.
	// This is a cheap, (mostly) stateless, utility wrapper.
	auto asset_loader = le::AssetLoader{&data_loader, &context};

	// load the Font.
	auto font = asset_loader.load_font("fonts/Vera.ttf");
	// load the image as a Texture.
	auto texture = asset_loader.load_texture("images/awesomeface.png");

	// load the audio file as a capo::Buffer.
	auto audio_buffer = asset_loader.load_audio_buffer("audio/explode.wav");
	// store playback trigger data.
	auto audio_wait = kvf::Seconds{2.0f};
	auto audio_played = false;

	// create a quad.
	auto quad = le::drawable::Quad{};
	quad.create({100.0f, 100.0f});
	// reposition it and set the loaded texture.
	quad.transform.position.y -= 30.0f;
	if (texture) { quad.texture = &texture.value(); }

	// create a Text instance.
	auto text = le::drawable::Text{};
	if (font) { text.set_string(*font, "hello from le2d!"); }
	// reposition and tint it.
	text.transform.position.y += 30.0f;
	text.tint = kvf::yellow_v;

	// loop while context is running (ie, window is open).
	while (context.is_running()) {
		// start the next frame. This polls events and waits for any
		// previous renders on the virtual frame to complete.
		context.next_frame();

		// compute the delta time (in float seconds).
		auto const dt = delta_time.tick();

		// update audio playback.
		audio_wait -= dt;
		if (audio_buffer && audio_wait < 0s && !audio_played) {
			// play the loaded audio buffer.
			context.get_audio().play_sfx(&audio_buffer.value());
			audio_played = true;
		}

		for (auto const& event : context.event_queue()) {
			// handle events here.
			// for example, if you want to close the window on Escape key press:
			if (auto const* key_event = std::get_if<le::event::Key>(&event)) {
				if (key_event->key == GLFW_KEY_ESCAPE && key_event->action == GLFW_PRESS) {
					context.shutdown(); // set the close flag.
				}
			}
		}

		// begin the primary render pass.
		if (auto renderer = context.begin_render()) {
			// draw quad.
			quad.draw(renderer);
			// draw text.
			text.draw(renderer);
		} // Renderer's destructor will call end_render().
		// It can also be explicitly/redundantly called inside the if block.

		// submit the frame for presentation.
		context.present();
	}

	// wait for the context to be idle before destroying any resources.
	context.wait_idle();
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

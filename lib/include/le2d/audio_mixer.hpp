#pragma once
#include <capo/source.hpp>
#include <klib/base_types.hpp>
#include <le2d/resource/audio_buffer.hpp>
#include <gsl/pointers>

namespace le {
/// \brief Opaque audio mixer interface.
/// The concrete type is not publicly accesssible.
/// Passed IAudioBuffers must outlive playback. Use stop_*() or
/// wait_idle() to unbind buffers from audio sources.
class IAudioMixer : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_sfx_gain() const -> float = 0;
	virtual void set_sfx_gain(float gain) = 0;
	virtual void play_sfx(gsl::not_null<IAudioBuffer const*> buffer) = 0;
	virtual void stop_sfx() = 0;

	[[nodiscard]] virtual auto get_music_gain() const -> float = 0;
	virtual void set_music_gain(float gain) = 0;
	virtual void loop_music(gsl::not_null<IAudioBuffer const*> buffer, kvf::Seconds fade = 1s) = 0;
	virtual void pause_music() = 0;
	virtual void resume_music() = 0;
	virtual void stop_music() = 0;

	/// \brief Create a custom audio source managed outide the mixer.
	[[nodiscard]] virtual auto create_source() const -> std::unique_ptr<capo::ISource> = 0;

	/// \returns true if any SFX or music is playing.
	[[nodiscard]] virtual auto is_playing() const -> bool = 0;
	/// \brief Waits for playing SFX to finish, stops music.
	virtual void wait_idle() = 0;
	/// \brief Stops all SFX and music.
	virtual void stop_all() = 0;
};
} // namespace le

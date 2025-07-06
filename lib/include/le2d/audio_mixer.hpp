#pragma once
#include <capo/source.hpp>
#include <klib/base_types.hpp>
#include <le2d/resource/audio_buffer.hpp>
#include <gsl/pointers>

namespace le {
/// \brief Opaque audio mixer interface.
/// The concrete type is not publicly accesssible.
class IAudioMixer : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_sfx_gain() const -> float = 0;
	virtual void set_sfx_gain(float gain) = 0;

	virtual void play_sfx(gsl::not_null<IAudioBuffer const*> buffer) = 0;

	virtual void wait_idle() = 0;
};
} // namespace le

#pragma once
#include <capo/capo.hpp>
#include <klib/polymorphic.hpp>

namespace le {
class IAudio : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_sfx_gain() const -> float = 0;
	virtual void set_sfx_gain(float gain) = 0;
	virtual void play_sfx(capo::Clip const& clip) = 0;

	[[nodiscard]] virtual auto create_stream_source() const -> capo::StreamSource = 0;
	virtual void start_music(capo::StreamSource& source, capo::Clip const& clip) const = 0;
};
} // namespace le

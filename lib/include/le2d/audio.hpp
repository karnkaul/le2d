#pragma once
#include <capo/source.hpp>
#include <klib/base_types.hpp>
#include <gsl/pointers>

namespace le {
/// \brief Opaque Audio Engine interface.
/// The concrete type is not publicly accesssible.
class IAudio : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_sfx_gain() const -> float = 0;
	virtual void set_sfx_gain(float gain) = 0;
	virtual void play_sfx(gsl::not_null<capo::Buffer const*> buffer) = 0;
	virtual void play_sfx(std::shared_ptr<capo::Buffer const> buffer) = 0;

	[[nodiscard]] virtual auto create_source() const -> std::unique_ptr<capo::ISource> = 0;
	virtual void start_music(capo::ISource& source, gsl::not_null<capo::Buffer const*> buffer) const = 0;
	virtual void start_music(capo::ISource& source, std::shared_ptr<capo::Buffer const> buffer) const = 0;

	virtual void wait_idle() = 0;
};
} // namespace le

#pragma once
#include <glm/vec2.hpp>
#include <klib/random.hpp>

namespace le {
/// \brief Stateful random number generator.
/// \tparam Gen Random generator / engine.
template <typename Gen>
class BasicRandom {
  public:
	/// \brief Initialize generator with a custom seed.
	explicit BasicRandom(std::uint32_t const seed) : m_gen(seed) {}

	/// \brief Initialize generator with a standard random device.
	explicit(false) BasicRandom() : BasicRandom(std::random_device{}()) {}

	/// \returns Random integer in [lo, hi].
	template <std::integral Type = int>
	[[nodiscard]] auto next_int(Type const lo, Type const hi) -> Type {
		return klib::random_int(m_gen, lo, hi);
	}

	/// \returns Random index in [0, size).
	[[nodiscard]] auto next_index(std::size_t const size) -> std::size_t { return klib::random_index(m_gen, size); }

	/// \returns Random float in [lo, hi].
	[[nodiscard]] auto next_float(float const lo, float const hi) { return klib::random_float(m_gen, lo, hi); }

	/// \returns true or false.
	[[nodiscard]] auto next_bool() -> bool { return next_int(0, 1) == 1; }

	/// \returns -1.0f or 1.0f.
	[[nodiscard]] auto next_sign() -> float { return next_bool() ? -1.0f : 1.0f; }

	/// \returns Random vec2 in [lo, hi].
	[[nodiscard]] auto next_vec2(glm::vec2 const lo, glm::vec2 const hi) -> glm::vec2 { return glm::vec2{next_float(lo.x, hi.x), next_float(lo.y, hi.y)}; }
	/// \returns Random vec2 in [lo, hi].
	[[nodiscard]] auto next_vec2(float const lo, float const hi) -> glm::vec2 { return next_vec2(glm::vec2{lo}, glm::vec2{hi}); }

	/// \returns Random signed float with magnitude between [positive_lo, positive_hi].
	[[nodiscard]] auto next_signed_float(float const positive_lo, float const positive_hi) -> float {
		return next_sign() * next_float(positive_lo, positive_hi);
	}

	/// \returns Random signed vec2 with magnitude between [positive_lo, positive_hi].
	[[nodiscard]] auto next_signed_vec2(glm::vec2 const positive_lo, glm::vec2 const positive_hi) -> glm::vec2 {
		return glm::vec2{next_signed_float(positive_lo.x, positive_hi.x), next_signed_float(positive_lo.y, positive_hi.y)};
	}
	/// \returns Random signed vec2 with magnitude between [positive_lo, positive_hi].
	[[nodiscard]] auto next_signed_vec2(float const positive_lo, float const positive_hi) -> glm::vec2 {
		return next_signed_vec2(glm::vec2{positive_lo}, glm::vec2{positive_hi});
	}

  private:
	Gen m_gen;
};

using Random = BasicRandom<std::mt19937>;
} // namespace le

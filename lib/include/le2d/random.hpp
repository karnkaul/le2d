#pragma once
#include <glm/vec2.hpp>
#include <klib/random.hpp>

namespace le {
template <typename Gen>
class BasicRandom {
  public:
	explicit BasicRandom(std::uint32_t const seed) : m_gen(seed) {}

	explicit(false) BasicRandom() : BasicRandom(std::random_device{}()) {}

	template <std::integral Type = int>
	[[nodiscard]] auto next_int(Type const lo, Type const hi) -> Type {
		return klib::random_int(m_gen, lo, hi);
	}

	[[nodiscard]] auto next_index(std::size_t const size) -> std::size_t { return klib::random_index(m_gen, size); }

	[[nodiscard]] auto next_float(float const lo, float const hi) { return klib::random_float(m_gen, lo, hi); }

	[[nodiscard]] auto next_bool() -> bool { return next_int(0, 1) == 1; }

	[[nodiscard]] auto next_sign() -> float { return next_bool() ? -1.0f : 1.0f; }

	[[nodiscard]] auto next_vec2(glm::vec2 const lo, glm::vec2 const hi) -> glm::vec2 { return glm::vec2{next_float(lo.x, hi.x), next_float(lo.y, hi.y)}; }
	[[nodiscard]] auto next_vec2(float const lo, float const hi) -> glm::vec2 { return next_vec2(glm::vec2{lo}, glm::vec2{hi}); }

	[[nodiscard]] auto next_signed_float(float const positive_lo, float const positive_hi) -> float {
		return next_sign() * next_float(positive_lo, positive_hi);
	}

	[[nodiscard]] auto next_signed_vec2(glm::vec2 const positive_lo, glm::vec2 const positive_hi) -> glm::vec2 {
		return glm::vec2{next_signed_float(positive_lo.x, positive_hi.x), next_signed_float(positive_lo.y, positive_hi.y)};
	}
	[[nodiscard]] auto next_signed_vec2(float const positive_lo, float const positive_hi) -> glm::vec2 {
		return next_signed_vec2(glm::vec2{positive_lo}, glm::vec2{positive_hi});
	}

  private:
	Gen m_gen;
};

using Random = BasicRandom<std::mt19937>;
} // namespace le

#pragma once
#include "le2d/anim/animation.hpp"
#include "le2d/anim/sampler.hpp"

namespace le {
namespace anim {
/// \brief Class template for Animator types.
template <typename PayloadT, typename SamplerT = SamplerFloor<PayloadT>>
class Animator {
  public:
	using Payload = PayloadT;
	using Sampler = SamplerT;
	using AnimationT = Animation<PayloadT>;

	[[nodiscard]] auto has_animation() const -> bool { return get_animation() != nullptr; }

	[[nodiscard]] auto get_animation() const -> AnimationT const* { return m_animation; }

	void set_animation(AnimationT const* animation) {
		m_animation = animation;
		elapsed = {};
		if (animation != nullptr) {
			repeat = animation->repeat;
			update_payload();
		}
	}

	[[nodiscard]] auto get_duration() const -> kvf::Seconds { return has_animation() ? m_animation->get_timeline().duration : 0s; }

	[[nodiscard]] auto get_progress() const -> float {
		auto const duration = get_duration();
		if (duration == 0s) { return 0.0f; }
		return elapsed / duration;
	}

	void tick(kvf::Seconds dt) {
		if (!has_animation()) { return; }
		auto const duration = get_duration();
		if (!repeat && elapsed > duration) { return; }
		elapsed += dt;
		if (repeat && elapsed > duration) { elapsed = {}; }
		update_payload();
	}

	[[nodiscard]] auto get_payload() const -> Payload const& { return m_payload; }

	kvf::Seconds elapsed{};
	bool repeat{true};

  private:
	void update_payload() { m_payload = SamplerT{}.sample(m_animation->get_timeline().keyframes, elapsed); }

	AnimationT const* m_animation{};
	Payload m_payload{};
};
} // namespace anim

using TransformAnimator = anim::Animator<Transform, anim::TransformSampler>;
using FlipbookAnimator = anim::Animator<TileId, anim::FlipbookSampler>;
} // namespace le

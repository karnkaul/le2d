#pragma once
#include <le2d/anim/animation.hpp>
#include <le2d/anim/sampler.hpp>

namespace le::anim {
template <typename PayloadT, typename SamplerT = SamplerNearest<PayloadT>>
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

using TransformAnimator = Animator<Transform, TransformSampler>;
using FlipbookAnimator = Animator<TileId, FlipbookSampler>;
} // namespace le::anim

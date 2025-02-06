#pragma once
#include <kvf/rect.hpp>
#include <kvf/time.hpp>
#include <le2d/transform.hpp>
#include <algorithm>
#include <gsl/pointers>
#include <string>

namespace le {
template <typename PayloadT>
struct AnimKeyframe {
	kvf::Seconds timestamp{};
	PayloadT payload{};
};

template <typename PayloadT>
class BasicAnimation {
  public:
	using Payload = PayloadT;
	using Keyframe = AnimKeyframe<PayloadT>;

	[[nodiscard]] auto get_duration() const -> kvf::Seconds { return m_duration; }

	[[nodiscard]] auto get_timeline() const -> std::span<Keyframe const> { return m_timeline; }

	void set_timeline(std::vector<Keyframe> keyframes) {
		m_timeline = std::move(keyframes);
		std::ranges::sort(m_timeline, [](Keyframe const& a, Keyframe const& b) { return a.timestamp < b.timestamp; });
		m_duration = m_timeline.empty() ? 0s : m_timeline.back().timestamp;
	}

	std::string name{};
	bool repeat{true};

  private:
	std::vector<Keyframe> m_timeline{};
	kvf::Seconds m_duration{};
};

class Animation : public BasicAnimation<Transform> {
  public:
	[[nodiscard]] auto sample(kvf::Seconds time) const -> Transform;
};

class Flipbook : public BasicAnimation<kvf::UvRect> {
  public:
	[[nodiscard]] auto sample(kvf::Seconds time) const -> kvf::UvRect;
};

template <typename AnimationT>
class Animator {
  public:
	using Payload = typename AnimationT::Payload;

	[[nodiscard]] auto has_animation() const -> bool { return get_animation() != nullptr; }

	[[nodiscard]] auto get_animation() const -> AnimationT const* { return m_animation; }
	void set_animation(AnimationT const* animation) {
		m_animation = animation;
		elapsed = {};
		if (animation != nullptr) {
			repeat = animation->repeat;
			m_payload = animation->sample(elapsed);
		}
	}

	[[nodiscard]] auto get_duration() const -> kvf::Seconds { return has_animation() ? m_animation->get_duration() : 0s; }

	void tick(kvf::Seconds dt) {
		if (!has_animation()) { return; }
		if (!repeat && elapsed > get_duration()) { return; }
		elapsed += dt;
		if (repeat && elapsed > get_duration()) { elapsed = {}; }
		m_payload = m_animation->sample(elapsed);
	}

	[[nodiscard]] auto get_payload() const -> Payload const& { return m_payload; }

	kvf::Seconds elapsed{};
	bool repeat{true};

  private:
	AnimationT const* m_animation{};
	Payload m_payload{};
};
} // namespace le

#pragma once
#include <kvf/rect.hpp>
#include <kvf/time.hpp>
#include <le2d/transform.hpp>
#include <algorithm>
#include <gsl/pointers>

namespace le {
template <typename PayloadT>
struct AnimKeyframe {
	kvf::Seconds timestamp{};
	PayloadT payload{};
};

template <typename SamplerT, typename PayloadT>
concept AnimSamplerT = requires(SamplerT const& s, std::span<AnimKeyframe<PayloadT> const> k, kvf::Seconds t) {
	{ s(k, t) } -> std::same_as<PayloadT>;
};

template <typename PayloadT>
class BasicAnimation {
  public:
	using Payload = PayloadT;
	using Keyframe = AnimKeyframe<PayloadT>;

	[[nodiscard]] auto get_duration() const -> kvf::Seconds { return m_duration; }

	void set_timeline(std::vector<Keyframe> keyframes) {
		m_timeline = std::move(keyframes);
		std::ranges::sort(m_timeline, [](Keyframe const& a, Keyframe const& b) { return a.timestamp < b.timestamp; });
		m_duration = m_timeline.empty() ? 0s : m_timeline.back().timestamp;
		elapsed = {};
	}

	[[nodiscard]] auto get_timeline() const -> std::span<Keyframe const> { return m_timeline; }

	[[nodiscard]] auto is_repeat() const -> bool { return m_repeat; }
	void set_repeat(bool const repeat) {
		m_repeat = repeat;
		if (m_repeat && elapsed > get_duration()) { elapsed = {}; }
	}

	void tick(kvf::Seconds dt) {
		elapsed += dt;
		if (m_repeat && elapsed > get_duration()) { elapsed = {}; }
	}

	template <AnimSamplerT<PayloadT> SamplerT>
	[[nodiscard]] auto sample(SamplerT const& sampler = SamplerT{}) const -> PayloadT {
		return sampler(m_timeline, elapsed);
	}

	kvf::Seconds elapsed{};

  private:
	std::vector<Keyframe> m_timeline{};
	kvf::Seconds m_duration{};
	bool m_repeat{true};
};

class Animation : public BasicAnimation<Transform> {
  public:
	[[nodiscard]] auto get_transform() const -> Transform { return sample<Sampler>(); }

  private:
	struct Sampler {
		auto operator()(std::span<Keyframe const> timeline, kvf::Seconds time) const -> Transform;
	};
};

class Flipbook : public BasicAnimation<kvf::UvRect> {
  public:
	[[nodiscard]] auto get_rect() const -> kvf::UvRect { return sample<Sampler>(); }

  private:
	struct Sampler {
		auto operator()(std::span<Keyframe const> timeline, kvf::Seconds time) const -> kvf::UvRect;
	};
};
} // namespace le

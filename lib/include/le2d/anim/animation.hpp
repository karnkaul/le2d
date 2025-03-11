#pragma once
#include <le2d/anim/timeline.hpp>
#include <le2d/tile.hpp>
#include <le2d/transform.hpp>

namespace le::anim {
template <typename PayloadT>
class Animation {
  public:
	using Payload = PayloadT;

	[[nodiscard]] auto get_timeline() const -> Timeline<Payload> const& { return m_timeline; }

	void set_timeline(Timeline<Payload> timeline) {
		m_timeline = std::move(timeline);
		std::ranges::sort(m_timeline.keyframes, [](Keyframe<Payload> const& a, Keyframe<Payload> const& b) { return a.timestamp < b.timestamp; });
	}

	std::string name{};
	bool repeat{true};

  private:
	Timeline<Payload> m_timeline{};
};

using TransformAnimation = Animation<Transform>;
using FlipbookAnimation = Animation<TileId>;
} // namespace le::anim

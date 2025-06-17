#pragma once
#include <le2d/anim/timeline.hpp>
#include <le2d/tile/tile_id.hpp>
#include <le2d/transform.hpp>
#include <algorithm>

namespace le::anim {
/// \brief Class template for Animation types.
template <typename PayloadT>
class Animation {
  public:
	using Payload = PayloadT;

	[[nodiscard]] auto get_timeline() const -> Timeline<Payload> const& { return m_timeline; }

	void set_timeline(Timeline<Payload> timeline) {
		m_timeline = std::move(timeline);
		std::ranges::sort(m_timeline.keyframes, [](Keyframe<Payload> const& a, Keyframe<Payload> const& b) { return a.timestamp < b.timestamp; });
	}

	[[nodiscard]] auto is_loaded() const -> bool { return !m_timeline.keyframes.empty(); }

	std::string name{};
	bool repeat{true};

  private:
	Timeline<Payload> m_timeline{};
};

using TransformAnimation = Animation<Transform>;
using FlipbookAnimation = Animation<TileId>;
} // namespace le::anim

#pragma once
#include "le2d/anim/interpolator.hpp"
#include "le2d/anim/keyframe.hpp"
#include "le2d/tile/tile_id.hpp"
#include <algorithm>
#include <concepts>
#include <span>

namespace le::anim {
/// \brief Concept for Animation Sampler.
template <typename Type, typename PayloadT>
concept SamplerT = requires(Type const& t, std::span<Keyframe<PayloadT> const> keyframes, kvf::Seconds time) {
	{ t.sample(keyframes, time) } -> std::same_as<PayloadT>;
};

/// \brief Quantized Animation Sampler.
template <typename PayloadT>
struct SamplerFloor {
	[[nodiscard]] constexpr auto sample(std::span<Keyframe<PayloadT> const> keyframes, kvf::Seconds const time) const -> PayloadT {
		if (keyframes.empty()) { return {}; }
		auto const it = std::ranges::upper_bound(keyframes, time, {}, [](Keyframe<PayloadT> const& kf) { return kf.timestamp; });
		if (it == keyframes.end()) { return keyframes.back().payload; }
		if (it == keyframes.begin()) { return it->payload; }
		return (it - 1)->payload;
	}
};

/// \brief Interpolation Animation Sampler.
template <typename PayloadT, typename InterpolatorT = Interpolator<PayloadT>>
struct SamplerLerp {
	[[nodiscard]] constexpr auto sample(std::span<Keyframe<PayloadT> const> keyframes, kvf::Seconds const time) const -> PayloadT {
		if (keyframes.empty()) { return {}; }
		auto const it = std::ranges::upper_bound(keyframes, time, {}, [](Keyframe<PayloadT> const& kf) { return kf.timestamp; });
		if (it == keyframes.begin()) { return it->payload; }
		if (it == keyframes.end()) { return (it - 1)->payload; }
		auto const& a = *(it - 1);
		auto const& b = *it;
		auto const total_t = b.timestamp - a.timestamp;
		auto const t = (time - a.timestamp) / total_t;
		return InterpolatorT{}(a.payload, b.payload, t);
	}
};

/// \brief Interpolated Transform Animation Sampler.
using TransformSampler = SamplerLerp<Transform, Interpolator<Transform>>;
/// \brief Quantized Flipbook Animation Sampler.
using FlipbookSampler = SamplerFloor<TileId>;

static_assert(SamplerT<TransformSampler, Transform>);
static_assert(SamplerT<FlipbookSampler, TileId>);
} // namespace le::anim

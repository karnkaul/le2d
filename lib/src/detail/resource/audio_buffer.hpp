#pragma once
#include "klib/debug/assert.hpp"
#include "le2d/resource/audio_buffer.hpp"
#include <capo/buffer.hpp>

namespace le::detail {
class AudioBuffer : public IAudioBuffer {
  public:
	explicit AudioBuffer() = default;

	auto decode(std::span<std::byte const> bytes, std::optional<capo::Encoding> const encoding) -> bool final { return m_buffer.decode_bytes(bytes, encoding); }

  private:
	[[nodiscard]] auto get_duration() const -> kvf::Seconds final {
		KLIB_ASSERT(m_buffer.is_loaded());
		auto const samples = float(m_buffer.get_samples().size());
		return kvf::Seconds{float(capo::Buffer::sample_rate_v) * (samples / float(m_buffer.get_channels()))};
	}

	auto bind(capo::ISource& source) const -> bool final {
		KLIB_ASSERT(m_buffer.is_loaded());
		return source.bind_to(&m_buffer);
	}

	capo::Buffer m_buffer;
};
} // namespace le::detail

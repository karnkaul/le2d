#pragma once
#include <capo/buffer.hpp>
#include <le2d/resource/audio_buffer.hpp>

namespace le::detail {
class AudioBuffer : public IAudioBuffer {
  public:
	explicit AudioBuffer() = default;

	[[nodiscard]] auto is_ready() const -> bool final { return m_buffer.is_loaded(); }

  private:
	auto decode(std::span<std::byte const> bytes, std::optional<capo::Encoding> const encoding) -> bool final { return m_buffer.decode_bytes(bytes, encoding); }

	[[nodiscard]] auto get_duration() const -> kvf::Seconds final {
		if (!m_buffer.is_loaded()) { return {}; }
		auto const samples = float(m_buffer.get_samples().size());
		return kvf::Seconds{float(capo::Buffer::sample_rate_v) * (samples / float(m_buffer.get_channels()))};
	}

	auto bind(capo::ISource& source) const -> bool final { return source.bind_to(&m_buffer); }

	capo::Buffer m_buffer;
};
} // namespace le::detail

#pragma once
#include <capo/source.hpp>
#include <kvf/time.hpp>
#include <le2d/resource/resource.hpp>

namespace le {
/// \brief Opaque interface for an Audio Buffer.
class IAudioBuffer : public IResource {
  public:
	/// \param bytes Bytes to load.
	/// \param encoding Encoding of audio data, if known.
	/// \returns true if successfully loaded.
	virtual auto decode(std::span<std::byte const> bytes, std::optional<capo::Encoding> encoding = {}) -> bool = 0;

	/// \returns Length of the audio data in seconds.
	[[nodiscard]] virtual auto get_duration() const -> kvf::Seconds = 0;

	/// \brief Bind this audio buffer to a source.
	/// \param source Source to bind to.
	/// \returns true if successfully bound.
	virtual auto bind(capo::ISource& source) const -> bool = 0;
};
} // namespace le

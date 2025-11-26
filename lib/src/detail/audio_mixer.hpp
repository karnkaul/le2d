#pragma once
#include "le2d/audio_mixer.hpp"
#include <capo/engine.hpp>
#include <klib/assert.hpp>
#include <log.hpp>
#include <algorithm>

namespace le::detail {
class AudioMixer : public IAudioMixer {
  public:
	explicit AudioMixer(int sfx_sources) : m_engine(capo::create_engine()) {
		if (!m_engine) {
			log.error("Failed to create Audio Engine");
			return;
		}

		static constexpr auto max_sfx_sources_v{256};
		sfx_sources = std::clamp(sfx_sources, 1, max_sfx_sources_v);
		m_sfx_sources.reserve(std::size_t(sfx_sources));
		for (int i = 0; i < sfx_sources; ++i) {
			auto& sfx_source = m_sfx_sources.emplace_back();
			sfx_source.source = m_engine->create_source();
		}

		m_deck.primary = m_engine->create_source();
		m_deck.secondary = m_engine->create_source();
	}

  private:
	using Clock = std::chrono::steady_clock;

	struct SfxSource {
		std::unique_ptr<capo::ISource> source{};
		Clock::time_point timestamp{};
	};

	struct Deck {
		std::unique_ptr<capo::ISource> primary{};
		std::unique_ptr<capo::ISource> secondary{};
		float gain{1.0f};
	};

	[[nodiscard]] auto get_sfx_gain() const -> float final { return m_sfx_gain; }

	void set_sfx_gain(float const gain) final {
		if (!m_engine || std::abs(gain - m_sfx_gain) < 0.01f) { return; }
		m_sfx_gain = gain;
		for (auto& source : m_sfx_sources) {
			if (!source.source->is_playing()) { continue; }
			source.source->set_gain(m_sfx_gain);
		}
	}

	void play_sfx(gsl::not_null<IAudioBuffer const*> buffer) final {
		if (!m_engine || !buffer->is_ready()) { return; }
		auto* source = get_idle_source();
		if (source == nullptr) { source = &get_oldest_source(); }
		play_sfx(*source, [buffer](capo::ISource& source) { buffer->bind(source); });
	}

	void stop_sfx() final {
		for (auto& source : m_sfx_sources) { source.source->unbind(); }
	}

	[[nodiscard]] auto get_music_gain() const -> float final { return m_deck.gain; }

	void set_music_gain(float const gain) final {
		m_deck.gain = gain;
		m_deck.primary->set_gain(gain);
	}

	void loop_music(gsl::not_null<IAudioBuffer const*> buffer, kvf::Seconds const fade) final {
		if (m_deck.primary->is_playing()) {
			m_deck.secondary->stop();
			m_deck.primary->set_looping(false);
			m_deck.primary->set_fade_out(fade);
			std::swap(m_deck.primary, m_deck.secondary);
		}
		m_deck.primary->stop();
		if (!buffer->bind(*m_deck.primary)) { return; }
		m_deck.primary->set_looping(true);
		m_deck.primary->set_fade_in(fade, m_deck.gain);
		m_deck.primary->play();
	}

	void pause_music() final {
		auto const pause = [](capo::ISource& source) { source.stop(); };
		pause(*m_deck.primary);
		pause(*m_deck.secondary);
	}

	void resume_music() final {
		auto const resume = [](capo::ISource& source) { source.play(); };
		resume(*m_deck.primary);
		resume(*m_deck.secondary);
	}

	void stop_music() final {
		auto const stop = [](capo::ISource& source) { source.unbind(); };
		stop(*m_deck.primary);
		stop(*m_deck.secondary);
	}

	[[nodiscard]] auto create_source() const -> std::unique_ptr<capo::ISource> final {
		if (!m_engine) { return {}; }
		return m_engine->create_source();
	}

	[[nodiscard]] auto is_playing() const -> bool final {
		if (std::ranges::any_of(m_sfx_sources, [](SfxSource const& s) { return s.source->is_playing(); })) { return true; }
		return m_deck.primary->is_playing() || m_deck.secondary->is_playing();
	}

	void wait_idle() final {
		for (auto& source : m_sfx_sources) {
			if (source.source->is_playing()) { source.source->wait_until_ended(); }
			source.source->unbind();
		}
		stop_music();
	}

	void stop_all() final {
		stop_sfx();
		stop_music();
	}

	[[nodiscard]] auto get_idle_source() -> SfxSource* {
		for (auto& source : m_sfx_sources) {
			if (!source.source->is_playing()) { return &source; }
		}
		return nullptr;
	}

	[[nodiscard]] auto get_oldest_source() -> SfxSource& {
		SfxSource* ret{};
		for (auto& source : m_sfx_sources) {
			if (ret == nullptr || source.timestamp < ret->timestamp) { ret = &source; }
		}
		KLIB_ASSERT(ret != nullptr);
		return *ret;
	}

	template <typename F>
	void play_sfx(SfxSource& source, F func) const {
		source.source->stop();
		source.source->set_gain(m_sfx_gain);
		func(*source.source);
		source.source->set_looping(false);
		source.source->play();
		source.timestamp = Clock::now();
	}

	std::unique_ptr<capo::IEngine> m_engine{};

	std::vector<SfxSource> m_sfx_sources{};
	Deck m_deck{};
	float m_sfx_gain{1.0f};
};
} // namespace le::detail

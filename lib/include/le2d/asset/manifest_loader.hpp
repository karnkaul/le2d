#pragma once
#include "klib/enum/map.hpp"
#include "klib/task/queue_create_info.hpp"
#include "le2d/asset/asset_loader.hpp"
#include "le2d/asset/asset_manifest.hpp"
#include "le2d/asset/asset_map.hpp"
#include <cstdint>
#include <gsl/pointers>

namespace le {
/// \brief State enumeration for a manifest loader instance.
enum class ManifestLoaderState : std::int8_t {
	/// \brief No jobs running.
	Idle,
	/// \brief Assets are being loaded.
	Loading,
	/// \brief Assets have finished loading and can be transferred.
	Loaded
};
inline auto const manifest_loader_state_name_map = klib::EnumNameMap<ManifestLoaderState>{
	{ManifestLoaderState::Idle, "idle"},
	{ManifestLoaderState::Loading, "loading"},
	{ManifestLoaderState::Loaded, "loaded"},
};

/// \brief Async load progress
struct ManifestLoadProgress {
	[[nodiscard]] constexpr auto completed() const -> std::int64_t { return loaded + failed; }

	[[nodiscard]] constexpr auto normalized() const -> float {
		if (total <= 0) { return 0.0f; }
		return float(completed()) / float(total);
	}

	std::int64_t loaded{};
	std::int64_t failed{};
	std::int64_t total{};
};

struct ManifestLoaderCreateInfo {
	/// \brief Number of worker threads for internal task queue.
	klib::task::ThreadCount thread_count{klib::task::get_max_threads()};
};

/// \brief Abstract interface for async asset loader.
class IManifestLoader : public klib::Polymorphic {
  public:
	using CreateInfo = ManifestLoaderCreateInfo;

	using State = ManifestLoaderState;
	using Progress = ManifestLoadProgress;

	[[nodiscard]] static auto create(gsl::not_null<AssetLoader const*> loader, CreateInfo const& create_info = {}) -> std::unique_ptr<IManifestLoader>;

	[[nodiscard]] virtual auto get_state() const -> State = 0;

	/// \brief Start async asset loads.
	auto start_loading(AssetManifest manifest) -> Progress {
		if (manifest.entries.empty() || get_state() != State::Idle) { return {}; }
		do_start_loading(std::move(manifest));
		return get_progress();
	}

	[[nodiscard]] virtual auto get_progress() const -> Progress = 0;

	virtual void wait_until_loaded() = 0;

	/// \brief Transfer loaded assets to external AssetMap and reset state.
	auto transfer_assets(AssetMap& out_map) -> Progress {
		wait_until_loaded();
		auto const ret = get_progress();
		do_transfer_assets(out_map);
		return ret;
	}

	/// \brief Drop pending jobs and reset state.
	virtual void cancel() = 0;

  protected:
	virtual void do_start_loading(AssetManifest manifest) = 0;
	virtual void do_transfer_assets(AssetMap& out_map) = 0;
};
} // namespace le

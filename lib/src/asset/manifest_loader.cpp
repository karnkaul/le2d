#include "le2d/asset/manifest_loader.hpp"
#include "klib/task/queue.hpp"
#include <atomic>

namespace le {
namespace {
struct AtomicProgress {
	[[nodiscard]] auto to_progress() const { return ManifestLoadProgress{.loaded = loaded.load(), .failed = failed.load(), .total = total}; }

	void reset_all() { loaded = failed = total = 0; }

	std::atomic<std::int64_t> loaded{};
	std::atomic<std::int64_t> failed{};
	std::int64_t total{};
};

class LoadTask : public klib::task::Task {
  public:
	explicit LoadTask(gsl::not_null<AssetLoader const*> loader, gsl::not_null<AtomicProgress*> progress, AssetManifest::Entry entry)
		: m_loader(loader), m_progress(progress), m_entry(std::move(entry)) {
		++m_progress->total;
	}

	void transfer(AssetMap& out_map) {
		if (!m_asset) { return; }
		out_map.store_asset(std::move(m_entry.uri), std::move(m_asset));
	}

  private:
	void execute() final {
		m_asset = m_loader->load_asset(m_entry.get_type(), m_entry.uri.get_string());
		if (m_asset) {
			++m_progress->loaded;
		} else {
			++m_progress->failed;
		}
	}

	gsl::not_null<AssetLoader const*> m_loader;
	gsl::not_null<AtomicProgress*> m_progress;
	AssetManifest::Entry m_entry;

	std::unique_ptr<IAsset> m_asset{};
};

class ManifestLoader : public IManifestLoader {
  public:
	ManifestLoader(ManifestLoader const&) = delete;
	ManifestLoader(ManifestLoader&&) = delete;
	ManifestLoader& operator=(ManifestLoader const&) = delete;
	ManifestLoader& operator=(ManifestLoader&&) = delete;

	explicit ManifestLoader(gsl::not_null<AssetLoader const*> loader, CreateInfo const& create_info)
		: m_loader(loader), m_queue(klib::task::Queue::CreateInfo{.thread_count = create_info.thread_count}) {}

	~ManifestLoader() { cancel_impl(); }

  private:
	[[nodiscard]] auto get_state() const -> State final {
		if (!m_enqueued_tasks.empty()) {
			auto const progress = get_progress();
			if (progress.loaded + progress.failed == progress.total) { return State::Loaded; }
			return State::Loading;
		}

		return State::Idle;
	}

	[[nodiscard]] auto get_progress() const -> Progress final { return m_progress.to_progress(); }

	void wait_until_loaded() final { m_queue.drain_and_wait(); }

	void cancel() final { cancel_impl(); }

	void do_start_loading(AssetManifest manifest) final {
		KLIB_ASSERT(!manifest.entries.empty() && m_enqueued_tasks.empty());

		m_load_tasks.clear();
		m_load_tasks.reserve(manifest.entries.size());
		for (auto& entry : manifest.entries) { m_load_tasks.push_back(std::make_unique<LoadTask>(m_loader, &m_progress, std::move(entry))); }

		m_enqueued_tasks.clear();
		m_enqueued_tasks.reserve(m_load_tasks.size());
		for (auto const& load_task : m_load_tasks) { m_enqueued_tasks.push_back(load_task.get()); }
		m_queue.enqueue(m_enqueued_tasks);
	}

	void do_transfer_assets(AssetMap& out_map) final {
		for (auto const& load_task : m_load_tasks) { load_task->transfer(out_map); }
		clear_all();
	}

	void cancel_impl() {
		m_queue.drop_enqueued();
		wait_until_loaded();
		clear_all();
	}

	void clear_all() {
		m_enqueued_tasks.clear();
		m_load_tasks.clear();
		m_progress.reset_all();
	}

	gsl::not_null<AssetLoader const*> m_loader;

	std::vector<std::unique_ptr<LoadTask>> m_load_tasks{};
	std::vector<klib::task::Task*> m_enqueued_tasks{};

	klib::task::Queue m_queue{};

	AtomicProgress m_progress{};
};
} // namespace

auto IManifestLoader::create(gsl::not_null<AssetLoader const*> loader, CreateInfo const& create_info) -> std::unique_ptr<IManifestLoader> {
	return std::make_unique<ManifestLoader>(loader, create_info);
}
} // namespace le

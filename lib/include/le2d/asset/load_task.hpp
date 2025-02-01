#pragma once
#include <klib/task/queue_fwd.hpp>
#include <klib/task/task.hpp>
#include <le2d/asset/loader.hpp>
#include <le2d/asset/store.hpp>
#include <atomic>
#include <gsl/pointers>
#include <vector>

namespace le::asset {
struct LoadProgress {
	std::uint64_t total{};
	std::uint64_t completed{};
	std::uint64_t failed{};

	[[nodiscard]] constexpr auto succeeded() const -> std::uint64_t { return completed - failed; }
};

class LoadTask : public klib::task::Task {
  public:
	using Queue = klib::task::Queue;

	explicit LoadTask(gsl::not_null<Queue*> queue) : m_queue(queue) {}

	template <std::derived_from<ILoader> LoaderT>
	void add_loader(std::unique_ptr<LoaderT> loader) {
		add_loader(typeid(typename LoaderT::type), std::move(loader));
	}

	template <typename Type>
	auto enqueue(Uri uri) -> bool {
		return enqueue(typeid(Type), std::move(uri));
	}
	void next_stage();

	[[nodiscard]] auto get_progress() const -> LoadProgress { return m_progress.to_progress(); }

	auto transfer_loaded(Store& store) -> std::uint64_t;

  private:
	struct AtomicProgress {
		std::uint64_t total{};
		std::atomic_uint64_t completed{};
		std::atomic_uint64_t failed{};

		[[nodiscard]] auto to_progress() const -> LoadProgress;

		void reset();
	};

	struct Task;
	struct Deleter {
		void operator()(Task* ptr) const noexcept;
	};

	using Stage = std::vector<std::unique_ptr<Task, Deleter>>;

	void execute() final;

	void add_loader(std::type_index type, std::unique_ptr<ILoader> loader);
	auto enqueue(std::type_index type, Uri uri) -> bool;
	void push_task(ILoader const& loader, Uri uri);

	Queue* m_queue;

	std::unordered_map<std::type_index, std::unique_ptr<ILoader>> m_loaders{};

	std::vector<Stage> m_stages{};
	AtomicProgress m_progress{};
};
} // namespace le::asset

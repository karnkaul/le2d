#pragma once
#include <klib/task/queue_fwd.hpp>
#include <klib/task/task.hpp>
#include <le2d/asset/loader.hpp>
#include <le2d/asset/store.hpp>
#include <gsl/pointers>
#include <vector>

namespace le::asset {
class LoadTask : public klib::task::Task {
  public:
	using Queue = klib::task::Queue;

	explicit LoadTask(gsl::not_null<Queue*> queue) : m_queue(queue) {}

	void add_loader(std::unique_ptr<ILoader> loader);

	template <typename Type>
	void enqueue(Uri uri) {
		enqueue(std::move(uri), typeid(Type));
	}

	void next_stage();

	auto transfer_loaded(Store& store) -> std::uint64_t;

  private:
	struct Task;
	struct Deleter {
		void operator()(Task* ptr) const noexcept;
	};

	using Stage = std::vector<std::unique_ptr<Task, Deleter>>;

	void execute() final;

	void enqueue(Uri uri, std::type_index type);
	[[nodiscard]] auto get_loader(std::type_index type) const -> ILoader const*;
	void push_task(ILoader const& loader, Uri uri);

	Queue* m_queue;

	std::unordered_map<std::type_index, std::unique_ptr<ILoader>> m_loaders{};

	std::vector<Stage> m_stages{};
};
} // namespace le::asset

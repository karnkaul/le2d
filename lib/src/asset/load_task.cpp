#include <klib/assert.hpp>
#include <klib/task/queue.hpp>
#include <le2d/asset/load_task.hpp>
#include <log.hpp>

namespace le::asset {
struct LoadTask::Task : klib::task::Task {
	explicit Task(ILoader const& loader, AtomicProgress& progress, Uri uri, std::type_index const type)
		: result{.uri = std::move(uri), .type = type}, m_loader(loader), m_progress(progress) {}

	void execute() final {
		result.asset = m_loader.load_base(result.uri);
		++m_progress.completed;
		if (!result.asset) { ++m_progress.failed; }
	}

	LoadedAsset result;

  private:
	ILoader const& m_loader;
	AtomicProgress& m_progress;
};

void LoadTask::Deleter::operator()(Task* ptr) const noexcept { std::default_delete<Task>{}(ptr); }

auto LoadTask::transfer_loaded() -> std::vector<LoadedAsset> {
	if (is_busy()) { wait(); }
	auto ret = std::vector<LoadedAsset>{};
	for (auto& stage : m_stages) {
		for (auto& task : stage) {
			if (!task->result.asset) { continue; }
			ret.push_back(std::move(task->result));
		}
	}
	m_stages.clear();
	m_progress.reset();
	return ret;
}

void LoadTask::add_loader(std::type_index const type, std::unique_ptr<ILoader> loader) {
	if (!loader) { return; }
	m_loaders.insert_or_assign(type, std::move(loader));
}

auto LoadTask::enqueue(std::type_index const type, Uri uri) -> bool {
	if (uri.get_string().empty()) { return false; }
	auto const it = m_loaders.find(type);
	if (it == m_loaders.end()) {
		log::warn("No Loader registered for type: {} ({})", type.name(), type.hash_code());
		return false;
	}
	push_task(type, *it->second, std::move(uri));
	return true;
}

void LoadTask::next_stage() { m_stages.emplace_back(); }

void LoadTask::execute() {
	if (m_stages.empty()) { return; }
	auto tasks = std::vector<klib::task::Task*>{};
	for (auto const& stage : m_stages) {
		tasks.reserve(stage.size());
		for (auto const& task : stage) { tasks.push_back(task.get()); }
		m_queue->fork_join(tasks);
		tasks.clear();
	}
}

void LoadTask::push_task(std::type_index const type, ILoader const& loader, Uri uri) {
	if (m_stages.empty()) { m_stages.emplace_back(); }
	auto& current_stage = m_stages.back();
	auto& task = current_stage.emplace_back();
	task.reset(new Task{loader, m_progress, std::move(uri), type}); // NOLINT(cppcoreguidelines-owning-memory)
	++m_progress.total;
}

auto LoadTask::AtomicProgress::to_progress() const -> LoadProgress {
	return LoadProgress{
		.total = total,
		.completed = completed,
		.failed = failed,
	};
}

void LoadTask::AtomicProgress::reset() { total = completed = failed = 0; }
} // namespace le::asset

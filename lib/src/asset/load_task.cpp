#include <klib/assert.hpp>
#include <klib/task/queue.hpp>
#include <le2d/asset/load_task.hpp>
#include <le2d/asset/loaders.hpp>
#include <le2d/font.hpp>
#include <le2d/texture.hpp>
#include <log.hpp>

namespace le::asset {
struct LoadTask::Task : klib::task::Task {
	explicit Task(ILoader const& loader, Uri uri) : uri(std::move(uri)), m_loader(loader) {}

	void execute() final { asset = m_loader.load_base(uri); }

	Uri uri;
	std::unique_ptr<Base> asset{};

  private:
	ILoader const& m_loader;
};

void LoadTask::Deleter::operator()(Task* ptr) const noexcept { std::default_delete<Task>{}(ptr); }

LoadTask::LoadTask(gsl::not_null<Queue*> queue, gsl::not_null<Context*> context) : m_queue(queue) {
	add_loader(std::make_unique<FontLoader>(context));
	add_loader(std::make_unique<TextureLoader>(context));
}

void LoadTask::add_loader(std::unique_ptr<ILoader> loader) {
	if (!loader) { return; }
	auto const type = loader->get_type();
	m_loaders.insert_or_assign(type, std::move(loader));
}

auto LoadTask::transfer_loaded(Store& store) -> std::uint64_t {
	if (is_busy()) { wait(); }
	auto ret = std::uint64_t{};
	for (auto& stage : m_stages) {
		for (auto& task : stage) {
			if (!task->asset) { continue; }
			store.insert_base(task->uri, std::move(task->asset));
			++ret;
		}
	}
	m_stages.clear();
	return ret;
}

auto LoadTask::enqueue(Uri uri, std::type_index const type) -> bool {
	if (uri.get_string().empty()) { return false; }
	auto const it = m_loaders.find(type);
	if (it == m_loaders.end()) {
		log::warn("No Loader registered for type: {} ({})", type.name(), type.hash_code());
		return false;
	}
	push_task(*it->second, std::move(uri));
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

void LoadTask::push_task(ILoader const& loader, Uri uri) {
	if (m_stages.empty()) { m_stages.emplace_back(); }
	auto& current_stage = m_stages.back();
	auto& task = current_stage.emplace_back();
	task.reset(new Task{loader, std::move(uri)}); // NOLINT(cppcoreguidelines-owning-memory)
}
} // namespace le::asset

#include <le2d/tweak/registry.hpp>

namespace le::tweak {
void Registry::add(std::string_view const id, std::shared_ptr<ITweakable> const& tweakable) {
	if (!tweakable) { return; }
	m_tweakables.insert_or_assign(id, tweakable);
}

void Registry::remove(std::string_view const id) {
	if (auto const it = m_tweakables.find(id); it != m_tweakables.end()) { m_tweakables.erase(it); }
}

void Registry::remove_expired() {
	for (auto it = m_tweakables.begin(); it != m_tweakables.end();) {
		if (it->second.expired()) {
			it = m_tweakables.erase(it);
		} else {
			++it;
		}
	}
}

auto Registry::find(std::string_view const id) const -> std::shared_ptr<ITweakable> {
	if (auto const it = m_tweakables.find(id); it != m_tweakables.end()) { return it->second.lock(); }
	return {};
}

void Registry::fill_entries(std::vector<Entry>& out) const {
	for (auto const& [id, ptr] : m_tweakables) {
		auto tweakable = ptr.lock();
		if (!tweakable) { continue; }
		out.push_back(Entry{.id = id, .tweakable = std::move(tweakable)});
	}
}

auto Registry::get_entries() const -> std::vector<Entry> {
	auto ret = std::vector<Entry>{};
	fill_entries(ret);
	return ret;
}
} // namespace le::tweak

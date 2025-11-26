#include "le2d/tweak/registry.hpp"

namespace le::tweak {
void Registry::add_tweakable(std::string_view const id, gsl::not_null<ITweakable*> tweakable) { m_tweakables.insert_or_assign(id, tweakable); }

void Registry::remove_tweakable(std::string_view const id) {
	if (auto const it = m_tweakables.find(id); it != m_tweakables.end()) { m_tweakables.erase(it); }
}

auto Registry::find_tweakable(std::string_view const id) const -> ITweakable* {
	if (auto const it = m_tweakables.find(id); it != m_tweakables.end()) { return it->second; }
	return {};
}

void Registry::fill_entries(std::vector<Entry>& out) const {
	for (auto const& [id, tweakable] : m_tweakables) { out.push_back(Entry{.id = id, .tweakable = tweakable}); }
}

auto Registry::get_entries() const -> std::vector<Entry> {
	auto ret = std::vector<Entry>{};
	fill_entries(ret);
	return ret;
}
} // namespace le::tweak

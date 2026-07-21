#include "cards/scene/factory.hpp"
#include "cards/log.hpp"

namespace cards::scene {
void Factory::store_creator(std::string_view name, Creator creator) {
	if (name.empty() || !creator) { return; }
	m_creators.insert_or_assign(name, std::move(creator));
}

auto Factory::set_next_scene(std::string_view const name) -> bool {
	if (name.empty()) { return false; }
	auto const it = m_creators.find(name);
	if (it == m_creators.end()) {
		log.warn("Failed to enqueue next Scene: '{}'", name);
		return false;
	}

	m_next.creator = &it->second;
	m_next.name = name;
	log.info("Scene '{}' enqueued for creation", name);
	return true;
}

auto Factory::get_next_scene() -> std::unique_ptr<Scene> {
	if (!m_next.creator) { return {}; }
	auto ret = (*m_next.creator)();
	log.info("Scene '{}' created", m_next.name);
	ret->initialize_scene(m_coordinator);
	m_next = {};
	return ret;
}
} // namespace cards::scene

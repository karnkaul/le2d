#pragma once
#include <klib/polymorphic.hpp>
#include <le2d/uri.hpp>
#include <memory>
#include <unordered_map>

namespace le {
class ResourceMap : public klib::Polymorphic {
  public:
	template <typename Type>
	void insert(Uri const& uri, std::shared_ptr<Type> asset) {
		if (uri.get_hash() == 0 || !asset) { return; }
		m_assets.insert({uri.get_hash(), std::move(asset)});
	}

	void erase(Uri const& uri) { m_assets.erase(uri.get_hash()); }

	[[nodiscard]] auto contains(Uri const& uri) const -> bool { return m_assets.contains(uri.get_hash()); }

	template <typename Type>
	[[nodiscard]] auto contains(Uri const& uri) const -> bool {
		return get<Type>(uri) != nullptr;
	}

	template <typename Type>
	[[nodiscard]] auto get(Uri const& uri) const -> std::shared_ptr<Type> {
		auto const it = m_assets.find(uri.get_hash());
		if (it == m_assets.end()) { return {}; }
		return std::dynamic_pointer_cast<Type>(it->second);
	}

  private:
	std::unordered_map<std::size_t, std::shared_ptr<void>> m_assets{};
};
} // namespace le

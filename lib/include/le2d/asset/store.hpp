#pragma once
#include <le2d/asset/wrap.hpp>
#include <le2d/uri.hpp>
#include <concepts>
#include <memory>
#include <unordered_map>

namespace le::asset {
class Store : public klib::Polymorphic {
  public:
	template <std::movable Type>
	auto insert(Uri const& uri, Type asset) -> Type* {
		if (uri.get_hash() == 0) { return {}; }
		auto wrap = std::make_unique<Wrap<Type>>(std::move(asset));
		auto* ret = &wrap->asset;
		m_assets.insert_or_assign(uri.get_hash(), std::move(wrap));
		return ret;
	}

	void insert_base(Uri const& uri, std::unique_ptr<Base> asset) {
		if (uri.get_hash() == 0 || !asset) { return; }
		m_assets.insert_or_assign(uri.get_hash(), std::move(asset));
	}

	void erase(Uri const& uri) { m_assets.erase(uri.get_hash()); }

	[[nodiscard]] auto contains(Uri const& uri) const -> bool { return m_assets.contains(uri.get_hash()); }

	template <typename Type>
	[[nodiscard]] auto contains(Uri const& uri) const -> bool {
		return get<Type>(uri) != nullptr;
	}

	template <typename Type>
	[[nodiscard]] auto get(Uri const& uri) const -> Type* {
		auto const it = m_assets.find(uri.get_hash());
		if (it == m_assets.end()) { return {}; }
		auto& base = *it->second;
		if (base.get_type() != typeid(Type)) { return {}; }
		return &static_cast<Wrap<Type>&>(base).asset;
	}

	[[nodiscard]] auto asset_count() const -> std::size_t { return m_assets.size(); }

	void clear() { m_assets.clear(); }

  private:
	std::unordered_map<std::size_t, std::unique_ptr<Base>> m_assets{};
};
} // namespace le::asset

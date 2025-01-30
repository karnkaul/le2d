#pragma once
#include <klib/polymorphic.hpp>
#include <le2d/uri.hpp>
#include <concepts>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace le::asset {
class ResourceMap : public klib::Polymorphic {
  public:
	template <std::movable Type>
	auto insert(Uri const& uri, Type asset) -> Type* {
		if (uri.get_hash() == 0) { return {}; }
		auto model = std::make_unique<Model<Type>>(std::move(asset));
		auto* ret = &model->t;
		m_assets.insert_or_assign(uri.get_hash(), std::move(model));
		return ret;
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
		if (base != typeid(Type)) { return {}; }
		return &static_cast<Model<Type>&>(base).t;
	}

	[[nodiscard]] auto asset_count() const -> std::size_t { return m_assets.size(); }

	void clear() { m_assets.clear(); }

  private:
	struct Base : klib::Polymorphic {
		explicit Base(std::type_index type) : m_type(type) {}
		[[nodiscard]] auto operator==(std::type_index const type) const -> bool { return m_type == type; }

	  private:
		std::type_index m_type;
	};

	template <typename T>
	struct Model : Base {
		explicit Model(T t) : Base(typeid(T)), t(std::move(t)) {}
		T t;
	};

	std::unordered_map<std::size_t, std::unique_ptr<Base>> m_assets{};
};
} // namespace le::asset

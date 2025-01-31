#pragma once
#include <klib/polymorphic.hpp>
#include <concepts>
#include <typeindex>

namespace le::asset {
class Base : public klib::Polymorphic {
  public:
	explicit Base(std::type_index type) : m_type(type) {}

	[[nodiscard]] auto get_type() const -> std::type_index { return m_type; }

  private:
	std::type_index m_type;
};

template <typename AssetT>
class Wrap : public Base {
  public:
	explicit Wrap(AssetT asset) : Base(typeid(AssetT)), asset(static_cast<AssetT&&>(asset)) {}

	AssetT asset;
};
} // namespace le::asset

#pragma once
#include "cards/catalog/catalog.hpp"
#include "le2d/context.hpp"
#include "le2d/data_loader.hpp"
#include "le2d/input/router.hpp"

namespace cards {
class IServices : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_context() const -> le::Context& = 0;
	[[nodiscard]] virtual auto get_data_loader() const -> le::IDataLoader const& = 0;
	[[nodiscard]] virtual auto get_catalog() const -> Catalog const& = 0;
	[[nodiscard]] virtual auto get_input_router() const -> le::input::Router& = 0;
	[[nodiscard]] virtual auto get_unprojector() const -> le::Unprojector const& = 0;
};
} // namespace cards

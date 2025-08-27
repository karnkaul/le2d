#pragma once
#include <le2d/input/action_mapping.hpp>
#include <le2d/input/router.hpp>

namespace le::input {
/// \brief RAII wrapper for associating a mapping with a router.
template <std::derived_from<IMapping> Type>
class ScopedMapping : public Type {
  public:
	ScopedMapping(ScopedMapping const&) = delete;
	ScopedMapping(ScopedMapping&&) = delete;
	auto operator=(ScopedMapping const&) = delete;
	auto operator=(ScopedMapping&&) = delete;

	explicit ScopedMapping(gsl::not_null<Router*> router) : m_router(router) { router->push_mapping(this); }

	~ScopedMapping() { m_router->remove_mapping(this); }

  private:
	gsl::not_null<Router*> m_router;
};

using ScopedActionMapping = ScopedMapping<ActionMapping>;
} // namespace le::input

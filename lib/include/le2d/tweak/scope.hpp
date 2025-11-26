#pragma once
#include "le2d/tweak/store.hpp"
#include <vector>

namespace le::tweak {
/// \brief RAII wrapper to erase added IDs on destruction.
class Scope {
  public:
	Scope(Scope const&) = delete;
	auto operator=(Scope const&) -> Scope& = delete;

	Scope(Scope&&) = default;
	auto operator=(Scope&&) -> Scope& = default;

	explicit Scope(gsl::not_null<IStore*> store) : m_store(store) {}

	~Scope() {
		for (auto const id : m_ids) { m_store->remove_tweakable(id); }
	}

	void add_tweakable(std::string_view const id, gsl::not_null<ITweakable*> tweakable) { m_store->add_tweakable(id, tweakable); }

  private:
	gsl::not_null<IStore*> m_store;
	std::vector<std::string_view> m_ids{};
};
} // namespace le::tweak

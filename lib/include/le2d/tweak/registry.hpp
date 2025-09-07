#pragma once
#include <le2d/tweak/store.hpp>
#include <map>

namespace le::tweak {
/// \brief Tweakable registry.
class Registry : public IStore {
  public:
	struct Entry {
		std::string_view id{};
		gsl::not_null<ITweakable*> tweakable;
	};

	void add_tweakable(std::string_view id, gsl::not_null<ITweakable*> tweakable) override;
	void remove_tweakable(std::string_view id) override;

	[[nodiscard]] auto find_tweakable(std::string_view id) const -> ITweakable*;

	void fill_entries(std::vector<Entry>& out) const;
	[[nodiscard]] auto get_entries() const -> std::vector<Entry>;

  private:
	std::map<std::string_view, gsl::not_null<ITweakable*>> m_tweakables{};
};
} // namespace le::tweak

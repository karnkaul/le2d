#pragma once
#include <le2d/tweak/tweakable.hpp>
#include <map>
#include <memory>
#include <vector>

namespace le::tweak {
/// \brief Tweakable registry.
class Registry {
  public:
	struct Entry {
		std::string_view id{};
		std::shared_ptr<ITweakable> tweakable{};
	};

	void add(std::string_view id, std::shared_ptr<ITweakable> const& tweakable);
	void remove(std::string_view id);
	void remove_expired();

	[[nodiscard]] auto find(std::string_view id) const -> std::shared_ptr<ITweakable>;

	void fill_entries(std::vector<Entry>& out) const;
	[[nodiscard]] auto get_entries() const -> std::vector<Entry>;

  private:
	std::map<std::string_view, std::weak_ptr<ITweakable>> m_tweakables{};
};
} // namespace le::tweak

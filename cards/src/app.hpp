#pragma once
#include <le2d/context.hpp>
#include <le2d/file_data_loader.hpp>
#include <optional>

#include <drawer.hpp>
#include <le2d/texture.hpp>

namespace cards {
class App {
  public:
	explicit App();

	void run();

  private:
	void tick(kvf::Seconds dt);
	void render(le::Renderer& renderer) const;

	le::Context m_context;

	le::FileDataLoader m_data_loader{};

	std::optional<le::TileSheet> m_deck_sheet{};
	Card m_card{};
	Drawer m_drawer{};

	kvf::DeviceBlock m_blocker{};
};
} // namespace cards

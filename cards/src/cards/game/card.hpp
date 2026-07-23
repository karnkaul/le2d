#pragma once
#include "cards/catalog/catalog.hpp"
#include "cards/types.hpp"
#include "le2d/drawable/sprite.hpp"

namespace cards::game {
class Card : public le::IDrawable {
  public:
	explicit Card(Catalog const& catalog, Denomination denomination);

	void draw(le::IRenderer& renderer) const final;

	void set_height(float height);

	[[nodiscard]] auto get_denomination() const -> Denomination { return m_denomination; }

	[[nodiscard]] auto get_face() const -> Face { return m_face; }
	void set_face(Face face);

	[[nodiscard]] auto get_sprite() const -> le::drawable::Sprite const& { return m_sprite; }
	[[nodiscard]] auto& instance(this auto&& self) { return self.m_sprite.instance; }

  private:
	struct Textures {
		explicit Textures(gsl::not_null<le::ITexture const*> face_up, gsl::not_null<le::ITexture const*> face_down);
		gsl::not_null<le::ITexture const*> face_up;
		gsl::not_null<le::ITexture const*> face_down;
	};

	Textures m_textures;
	Denomination m_denomination;

	le::drawable::Sprite m_sprite{};
	Face m_face{};
};
} // namespace cards::game

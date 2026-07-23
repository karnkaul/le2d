#include "cards/game/card.hpp"

namespace cards {
Card::Card(Catalog const& catalog, Denomination const denomination)
	: m_textures(catalog.get_suit_group(denomination.suit).texture_if(denomination.value), catalog.get_cover_texture()), m_denomination(denomination) {
	m_sprite.set_resize_aspect(kvf::ResizeAspect::FixHeight);
	set_height(200.0f);
	set_face(Face::Up);
}

void Card::draw(le::IRenderer& renderer) const { m_sprite.draw(renderer); }

void Card::set_height(float const height) { m_sprite.set_base_size(glm::vec2{height}); }

void Card::set_face(Face const face) {
	switch (face) {
	case Face::Up: m_sprite.set_texture(m_textures.face_up); break;
	case Face::Down: m_sprite.set_texture(m_textures.face_down); break;
	default: return;
	}
	m_face = face;
}

Card::Textures::Textures(gsl::not_null<le::ITexture const*> face_up, gsl::not_null<le::ITexture const*> face_down) : face_up(face_up), face_down(face_down) {}
} // namespace cards

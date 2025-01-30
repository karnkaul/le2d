#include <le2d/shape/sprite.hpp>

namespace le::shape {
void Sprite::set_base_size(glm::vec2 const size) {
	if (size == m_size) { return; }
	m_size = size;
	update(get_base_size(), get_origin(), get_uv());
}

void Sprite::set_origin(glm::vec2 const origin) {
	if (origin == get_origin()) { return; }
	update(get_base_size(), origin, get_uv());
}

void Sprite::set_uv(kvf::UvRect const& uv) {
	if (uv == get_uv()) { return; }
	update(get_base_size(), get_origin(), uv);
}

void Sprite::set_texture(std::shared_ptr<ITexture const> texture, kvf::UvRect const& uv) {
	if (texture.get() == m_quad.texture.get() && uv == get_uv()) { return; }
	m_quad.texture = std::move(texture);
	update(get_base_size(), get_origin(), uv);
}

void Sprite::set_resize_aspect(kvf::ResizeAspect const aspect) {
	if (aspect == m_aspect) { return; }
	m_aspect = aspect;
	update(get_base_size(), get_origin(), get_uv());
}

void Sprite::update(glm::vec2 const base_size, glm::vec2 const origin, kvf::UvRect const& uv) {
	auto const size = [&] {
		if (!m_quad.texture || m_aspect == kvf::ResizeAspect::None) { return base_size; }
		auto const n_size = uv.rb - uv.lt;
		auto const tile_size = n_size * glm::vec2{m_quad.texture->get_size()};
		return kvf::aspect_resize(base_size, tile_size, m_aspect);
	}();
	auto const quad_params = QuadParams{.rect = kvf::Rect<>::from_size(size, origin), .uv = uv};
	m_quad.create(quad_params);
}
} // namespace le::shape

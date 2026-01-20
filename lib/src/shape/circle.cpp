#include "le2d/shape/circle.hpp"

namespace le::shape {
void Circle::create(float const diameter, Params const& params) {
	auto const sector_params = Sector::Params{
		.color = params.color,
		.resolution = params.resolution,
	};
	m_sector.create(diameter, sector_params);
}
} // namespace le::shape

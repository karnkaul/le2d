#include <le2d/vertex_bounds.hpp>
#include <algorithm>
#include <limits>

auto le::vertex_bounds(std::span<Vertex const> vertices, glm::mat4 const& model) -> kvf::Rect<> {
	if (vertices.empty()) { return {}; }
	auto ret = kvf::Rect<>{};
	ret.lt.x = ret.rb.y = std::numeric_limits<float>::max();
	ret.lt.y = ret.rb.x = -std::numeric_limits<float>::max();
	for (auto const& vertex : vertices) {
		auto const xy = model * glm::vec4{vertex.position, 0.0f, 1.0f};
		ret.lt.x = std::min(ret.lt.x, xy.x);
		ret.rb.y = std::min(ret.rb.y, xy.y);
		ret.rb.x = std::max(ret.rb.x, xy.x);
		ret.lt.y = std::max(ret.lt.y, xy.y);
	}
	return ret;
}

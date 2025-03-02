#pragma once
#include <kvf/color.hpp>
#include <kvf/time.hpp>
#include <le2d/text_height.hpp>

namespace le::console {
struct TerminalCreateInfo {
	struct {
		std::size_t history{32};
		std::size_t buffer{128};
	} storage{};

	struct {
		TextHeight text_height{TextHeight{20}};
		float line_spacing{1.1f};
		float separator_height{2.0f};
		float x_pad{15.0f};
		char caret{'>'};
		char cursor{'|'};
	} style{};

	struct {
		float slide_speed{4000.0f};
		float scroll_speed{30.0f};
		kvf::Seconds blink_period{1s};
	} motion{};

	struct {
		kvf::Color cursor{kvf::magenta_v};
		kvf::Color separator{kvf::white_v};
		kvf::Color input{kvf::yellow_v};
		kvf::Color output{0xccccccff};
		kvf::Color error{kvf::red_v};
	} colors{};
};
} // namespace le::console

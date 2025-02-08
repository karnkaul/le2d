#pragma once
#include <klib/enum_array.hpp>
#include <kvf/color.hpp>
#include <le2d/text_height.hpp>
#include <ui/widget_state.hpp>

namespace hog::ui {
struct ButtonStyle {
	klib::EnumArray<WidgetState, kvf::Color> background_colors{
		kvf::white_v,
		kvf::cyan_v,
		kvf::magenta_v,
		kvf::Color{0x777777ff},
	};
	klib::EnumArray<WidgetState, kvf::Color> text_colors{
		kvf::black_v,
		kvf::black_v,
		kvf::black_v,
		kvf::black_v,
	};
	le::TextHeight text_height{le::TextHeight::Default};
	float text_n_y_offset{-0.4f};
};
} // namespace hog::ui

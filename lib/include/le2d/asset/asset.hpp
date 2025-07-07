#pragma once
#include <klib/base_types.hpp>

namespace le {
/// \brief Interface for asset types.
/// Assets are loadable through data (usually bytes), and may be referenced
/// by multiple other objects simultaneously.
class IAsset : public klib::Polymorphic {};
} // namespace le

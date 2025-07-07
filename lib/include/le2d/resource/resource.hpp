#pragma once
#include <klib/base_types.hpp>

namespace le {
/// \brief Interface for all shared resources in the engine.
class IResource : public klib::Polymorphic {
  public:
	/// \brief Check if resource is ready to use.
	///
	/// Moved resources are expected to return false.
	[[nodiscard]] virtual auto is_ready() const -> bool = 0;
};
} // namespace le

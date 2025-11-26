#pragma once
#include "le2d/tweak/tweakable.hpp"
#include <klib/base_types.hpp>
#include <gsl/pointers>

namespace le::tweak {
class IStore : public klib::Polymorphic {
  public:
	virtual void add_tweakable(std::string_view id, gsl::not_null<ITweakable*> tweakable) = 0;
	virtual void remove_tweakable(std::string_view id) = 0;
};
} // namespace le::tweak

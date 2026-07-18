#pragma once
#include "le2d/input/device.hpp"
#include "le2d/input/mapping.hpp"
#include <memory>
#include <optional>

namespace le::input {
/// \brief Stack of IMapping weak-pointers.
/// An Event is routed in (reverse) order of bound mappings until one consumes it.
class Router {
  public:
	/// \brief Push a mapping to the top of the stack.
	/// \param mapping Mapping to push. Router will own a weak-pointer until it expires.
	/// Disengages existing mappings before pushing.
	void push_mapping(std::shared_ptr<IMapping> const& mapping);

	/// \brief Pop the topmost mapping, if any.
	void pop_mapping();

	/// \brief Remove a particular mapping.
	void remove_mapping(std::shared_ptr<IMapping> const& mapping);

	/// \returns true if mapping is bound (and unexpired).
	[[nodiscard]] auto contains_mapping(std::shared_ptr<IMapping> const& mapping) const -> bool;

	/// \brief Clear all mappings.
	void clear_stack() { m_mappings.clear(); }
	/// \returns true if there are no mappings pushed.
	[[nodiscard]] auto is_stack_empty() const -> bool { return m_mappings.empty(); }

	/// \brief Dispatch events to the topmost mapping, if any.
	/// \param events List of events for this frame.
	void dispatch(std::span<le::Event const> events);

	/// \brief Disengage the topmost mapping, if any.
	void disengage();

	/// \returns Last used input device, if any.
	[[nodiscard]] auto last_used_device() const -> std::optional<Device> { return m_last_used_device; }

	[[nodiscard]] auto gamepad_manager() const -> Gamepad::Manager const& { return m_gamepad_manager; }

	/// \brief Dead zone for setting first/last used Gamepad.
	float nonzero_dead_zone{Gamepad::nonzero_dead_zone_v};

	/// \brief If non-null, always on top.
	std::weak_ptr<IMapping> terminal_mapping{};

  private:
	template <typename F>
	auto cascade_invoke_if(F func) const -> bool;

	void pre_dispatch();
	void dispatch_events(std::span<Event const> events);

	std::vector<std::weak_ptr<IMapping>> m_mappings{};
	std::vector<std::shared_ptr<IMapping>> m_mappings_buffer{};

	Gamepad::Manager m_gamepad_manager{};
	std::optional<Device> m_last_used_device{};
};
} // namespace le::input

#pragma once
#include <le2d/input/mapping.hpp>
#include <optional>

namespace le::input {
/// \brief Input Device enumeration.
enum class Device : std::int8_t { Keyboard, Mouse, PrimaryGamepad };

/// \brief Stack of mappings, routes events to the topmost one.
class Router {
  public:
	/// \brief Push a mapping to the top.
	/// \param mapping Mapping to push.
	/// Disengages the existing topmost mapping before pushing.
	void push_mapping(gsl::not_null<IMapping*> mapping);

	/// \brief Pop the topmost mapping, if any.
	void pop_mapping();

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
	/// \returns Primary gamepad, ie first one that was active, if any.
	[[nodiscard]] auto primary_gamepad() const -> Gamepad const& { return m_primary_gamepad; }

  private:
	template <typename F>
	void do_dispatch(F func);

	std::vector<gsl::not_null<IMapping*>> m_mappings{};

	Gamepad m_primary_gamepad{};
	std::optional<Device> m_last_used_device{};
};
} // namespace le::input

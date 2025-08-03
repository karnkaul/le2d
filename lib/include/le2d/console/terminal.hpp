#pragma once
#include <klib/concepts.hpp>
#include <le2d/console/command.hpp>
#include <le2d/console/terminal_create_info.hpp>
#include <le2d/drawable/drawable.hpp>
#include <le2d/event.hpp>
#include <le2d/resource/font.hpp>
#include <cstdint>
#include <gsl/pointers>
#include <memory>
#include <string_view>

namespace le {
class Font;
} // namespace le

namespace le::console {
enum class StateChange : std::int8_t { None, Activated, Deactivated };

class Terminal : public IDrawable, public IPrinter {
  public:
	using CreateInfo = TerminalCreateInfo;

	explicit Terminal(gsl::not_null<IFont*> font, glm::vec2 framebuffer_size, CreateInfo const& info = {});

	[[nodiscard]] auto is_active() const -> bool;

	void add_command(std::string_view name, std::unique_ptr<ICommand> command);
	void remove_command(std::string_view name);

	void println(std::string_view text) final;
	void printerr(std::string_view text) final;

	[[nodiscard]] auto get_background() const -> kvf::Color;
	void set_background(kvf::Color color);

	auto handle_events(glm::vec2 framebuffer_size, std::span<Event const> events) -> StateChange;

	void tick(kvf::Seconds dt);
	void draw(Renderer& renderer) const final;

  private:
	struct Impl;
	struct Deleter {
		void operator()(Impl* ptr) const noexcept;
	};
	std::unique_ptr<Impl, Deleter> m_impl{};

	friend class Stream;
};
} // namespace le::console

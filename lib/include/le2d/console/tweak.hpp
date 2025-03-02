#pragma once
#include <klib/concepts.hpp>
#include <le2d/console/command.hpp>
#include <charconv>
#include <format>

namespace le::console {
class ITweak : public ICommand {
  public:
	virtual void write_to(std::string& output) = 0;
	virtual auto deserialize(std::string_view input) -> bool = 0;

	void run(IPrinter& printer, std::string_view input) final;
};

template <typename Type>
class Tweak;

template <klib::StringyT Type>
class Tweak<Type> : public ITweak {
  protected:
	[[nodiscard]] virtual auto get() const -> std::string_view = 0;
	virtual void set(Type value) = 0;

  private:
	void write_to(std::string& output) final { output = get(); }

	auto deserialize(std::string_view const value) -> bool final {
		set(Type{value});
		return true;
	}
};

template <klib::NumberT Type>
class Tweak<Type> : public ITweak {
  protected:
	[[nodiscard]] virtual auto get() const -> Type = 0;
	virtual void set(Type value) = 0;

  private:
	void write_to(std::string& output) final { std::format_to(std::back_inserter(output), "{}", get()); }

	auto deserialize(std::string_view const value) -> bool final {
		auto const* end = value.data() + value.size();
		auto t = Type{};
		auto const [ptr, ec] = std::from_chars(value.data(), end, t);
		if (ec != std::errc{} || ptr != end) { return false; }
		set(t);
		return true;
	}
};
} // namespace le::console

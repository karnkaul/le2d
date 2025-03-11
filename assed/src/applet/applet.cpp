#include <applet/applet.hpp>
#include <le2d/context.hpp>
#include <le2d/input/dispatch.hpp>

namespace le::assed {
Applet::Applet(gsl::not_null<ServiceLocator const*> services) : m_services(services) { services->get<input::Dispatch>().attach(this); }

auto Applet::get_framebuffer_size() const -> glm::vec2 { return get_services().get<Context>().framebuffer_size(); }
} // namespace le::assed

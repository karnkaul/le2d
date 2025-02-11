#include <klib/visitor.hpp>
#include <le2d/context.hpp>
#include <le2d/event/dispatch.hpp>
#include <le2d/service_locator.hpp>
#include <scene/scene.hpp>

namespace hog::scene {
Scene::Scene(gsl::not_null<le::ServiceLocator const*> services) : m_services(services) { m_services->get<le::event::Dispatch>().attach(this); }

auto Scene::consume_event(le::Event const& event) -> bool {
	auto const visitor = klib::Visitor{
		[this](le::event::WindowClose const close) { on_event(close); },
		[this](le::event::WindowFocus const focus) { on_event(focus); },
		[this](le::event::CursorFocus const focus) { on_event(focus); },
		[this](le::event::FramebufferResize const resize) { on_event(resize); },
		[this](le::event::WindowResize const resize) { on_event(resize); },
		[this](le::event::WindowPos const pos) { on_event(pos); },
		[this](le::event::CursorPos const& pos) { on_event(pos); },
		[this](le::event::Codepoint const codepoint) { on_event(codepoint); },
		[this](le::event::Key const& key) { on_event(key); },
		[this](le::event::MouseButton const mb) { on_event(mb); },
		[this](le::event::Scroll const scroll) { on_event(scroll); },
		[this](le::event::Drop const& drop) { on_event(drop); },
	};
	std::visit(visitor, event);
	return false;
}

auto Scene::get_unprojector(le::Transform const& render_view) const -> le::Unprojector {
	auto const framebuffer_size = m_services->get<le::Context>().framebuffer_size();
	return le::Unprojector{render_view, framebuffer_size};
}
} // namespace hog::scene

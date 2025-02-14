#pragma once
#include <klib/polymorphic.hpp>
#include <le2d/event.hpp>

namespace hog {
class IEventHandler : public klib::Polymorphic {
  public:
	virtual void on_event(le::event::WindowClose close) = 0;
	virtual void on_event(le::event::WindowFocus focus) = 0;
	virtual void on_event(le::event::CursorFocus focus) = 0;
	virtual void on_event(le::event::FramebufferResize resize) = 0;
	virtual void on_event(le::event::WindowResize resize) = 0;
	virtual void on_event(le::event::WindowPos pos) = 0;
	virtual void on_event(le::event::CursorPos pos) = 0;
	virtual void on_event(le::event::Codepoint codepoint) = 0;
	virtual void on_event(le::event::Key key) = 0;
	virtual void on_event(le::event::MouseButton button) = 0;
	virtual void on_event(le::event::Scroll scroll) = 0;
	virtual void on_event(le::event::Drop drop) = 0;
};

class SubEventHandler : public IEventHandler {
  public:
	void on_event(le::event::WindowClose /*close*/) override {}
	void on_event(le::event::WindowFocus /*focus*/) override {}
	void on_event(le::event::CursorFocus /*focus*/) override {}
	void on_event(le::event::FramebufferResize /*resize*/) override {}
	void on_event(le::event::WindowResize /*resize*/) override {}
	void on_event(le::event::WindowPos /*pos*/) override {}
	void on_event(le::event::CursorPos /*pos*/) override {}
	void on_event(le::event::Codepoint /*codepoint*/) override {}
	void on_event(le::event::Key /*key*/) override {}
	void on_event(le::event::MouseButton /*button*/) override {}
	void on_event(le::event::Scroll /*scroll*/) override {}
	void on_event(le::event::Drop /*drop*/) override {}
};
} // namespace hog

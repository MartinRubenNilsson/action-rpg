#include "ui.h"
#include <RmlUi/Core.h>
#include "RmlUi_Renderer_GL2_SFML.h"
#include "ui_bindings.h"
#include "ui_menus.h"
#include "ui_hud.h"
#include "ui_textbox.h"
#include "console.h"
#include "audio.h"
#include "window.h"

namespace ui
{
	struct SystemInterface_SFML : Rml::SystemInterface
	{
		sf::Clock timer;

		double GetElapsedTime() override {
			return (double)timer.getElapsedTime().asMicroseconds() / 1'000'000.0;
		}

		void SetMouseCursor(const Rml::String& cursor_name) override
		{
			if (cursor_name.empty() || cursor_name == "arrow")
				window::set_cursor(sf::Cursor::Arrow);
			else if (cursor_name == "move")
				window::set_cursor(sf::Cursor::SizeAll);
			else if (cursor_name == "pointer")
				window::set_cursor(sf::Cursor::Hand);
			else if (cursor_name == "resize")
				window::set_cursor(sf::Cursor::SizeTopLeftBottomRight);
			else if (cursor_name == "cross")
				window::set_cursor(sf::Cursor::Cross);
			else if (cursor_name == "text")
				window::set_cursor(sf::Cursor::Text);
			else if (cursor_name == "unavailable")
				window::set_cursor(sf::Cursor::NotAllowed);
			else if (cursor_name.starts_with("rmlui-scroll"))
				window::set_cursor(sf::Cursor::SizeAll);
		}

		void SetClipboardText(const Rml::String& text_utf8) override {
			sf::Clipboard::setString(text_utf8);
		}

		void GetClipboardText(Rml::String& text) override {
			text = sf::Clipboard::getString();
		}
	};

	struct EventListener : Rml::EventListener
	{
		void ProcessEvent(Rml::Event& event) override
		{
			if (!event.GetTargetElement()->IsClassSet("menu-button"))
				return;
			switch (event.GetId()) {
			case Rml::EventId::Mouseover:
				audio::play("event:/ui/snd_button_hover");
				break;
			case Rml::EventId::Click:
				audio::play("event:/ui/snd_button_click");
				break;
			}
		}
	};

	SystemInterface_SFML _system_interface;
	RenderInterface_GL2_SFML _render_interface;
	Rml::Context* _context = nullptr;
	EventListener _event_listener;
	Event _event;

	void _on_window_resized(const Rml::Vector2i& new_size)
	{
		_render_interface.SetViewport(new_size.x, new_size.y);
		_context->SetDimensions(new_size);
		float dp_ratio_x = (float)new_size.x / (float)window::VIEW_SIZE.x;
		float dp_ratio_y = (float)new_size.y / (float)window::VIEW_SIZE.y;
		float dp_ratio = std::min(dp_ratio_x, dp_ratio_y);
		_context->SetDensityIndependentPixelRatio(dp_ratio);
	}

	void _on_escape_key_pressed()
	{
		MenuType current_menu = get_current_menu();
		if (current_menu == MenuType::Count) // no menus are open
			push_menu(MenuType::Pause);
		else if (current_menu != MenuType::Main) // don't pop main menu
			pop_menu();
	}

	void _on_click_play()
	{
		pop_all_menus();
		set_hud_visible(true);
		_event = Event::PlayGame;
	}

	void _on_click_settings() {
		push_menu(MenuType::Settings);
	}

	void _on_click_credits() {
		push_menu(MenuType::Credits);
	}

	void _on_click_quit() {
		_event = Event::QuitApp;
	}

	void _on_click_back() {
		pop_menu();
	}

	void _on_click_resume() {
		pop_menu();
	}

	void _on_click_main_menu()
	{
		set_hud_visible(false);
		pop_all_menus();
		push_menu(MenuType::Main);
		_event = Event::GoBackToMainMenu;
	}

	void initialize(sf::RenderWindow& window)
	{
		Rml::SetSystemInterface(&_system_interface);
		Rml::SetRenderInterface(&_render_interface);
		Rml::Initialise();
		Rml::Vector2i window_size(window.getSize().x, window.getSize().y);
		_context = Rml::CreateContext("main", window_size);
		_on_window_resized(window_size);
		create_bindings();
		create_textbox_presets();
	}

	void shutdown()
	{
		Rml::RemoveContext(_context->GetName());
		_context = nullptr;
		Rml::Shutdown();
	}

	void load_ttf_fonts(const std::filesystem::path& dir)
	{
		for (const std::filesystem::directory_entry& entry :
			std::filesystem::directory_iterator(dir)) {
			if (entry.path().extension() != ".ttf") continue;
			Rml::LoadFontFace(entry.path().string());
		}
	}

	void load_rml_documents(const std::filesystem::path& dir)
	{
		for (const std::filesystem::directory_entry& entry :
			std::filesystem::directory_iterator(dir)) {
			if (entry.path().extension() != ".rml") continue;
			Rml::ElementDocument* doc = _context->LoadDocument(entry.path().string());
			if (!doc) continue;
			doc->SetId(entry.path().stem().string());
			doc->AddEventListener(Rml::EventId::Mouseover, &_event_listener);
			doc->AddEventListener(Rml::EventId::Click, &_event_listener);
		}
	}

	void reload_styles()
	{
		for (int i = 0; i < _context->GetNumDocuments(); ++i)
			_context->GetDocument(i)->ReloadStyleSheet();
	}

	Rml::Input::KeyIdentifier _sfml_key_to_rml_key(sf::Keyboard::Key key)
	{
		using namespace Rml::Input;
		switch (key) {
		case sf::Keyboard::A:         return KI_A;
		case sf::Keyboard::B:         return KI_B;
		case sf::Keyboard::C:         return KI_C;
		case sf::Keyboard::D:         return KI_D;
		case sf::Keyboard::E:         return KI_E;
		case sf::Keyboard::F:         return KI_F;
		case sf::Keyboard::G:         return KI_G;
		case sf::Keyboard::H:         return KI_H;
		case sf::Keyboard::I:         return KI_I;
		case sf::Keyboard::J:         return KI_J;
		case sf::Keyboard::K:         return KI_K;
		case sf::Keyboard::L:         return KI_L;
		case sf::Keyboard::M:         return KI_M;
		case sf::Keyboard::N:         return KI_N;
		case sf::Keyboard::O:         return KI_O;
		case sf::Keyboard::P:         return KI_P;
		case sf::Keyboard::Q:         return KI_Q;
		case sf::Keyboard::R:         return KI_R;
		case sf::Keyboard::S:         return KI_S;
		case sf::Keyboard::T:         return KI_T;
		case sf::Keyboard::U:         return KI_U;
		case sf::Keyboard::V:         return KI_V;
		case sf::Keyboard::W:         return KI_W;
		case sf::Keyboard::X:         return KI_X;
		case sf::Keyboard::Y:         return KI_Y;
		case sf::Keyboard::Z:         return KI_Z;
		case sf::Keyboard::Num0:      return KI_0;
		case sf::Keyboard::Num1:      return KI_1;
		case sf::Keyboard::Num2:      return KI_2;
		case sf::Keyboard::Num3:      return KI_3;
		case sf::Keyboard::Num4:      return KI_4;
		case sf::Keyboard::Num5:      return KI_5;
		case sf::Keyboard::Num6:      return KI_6;
		case sf::Keyboard::Num7:      return KI_7;
		case sf::Keyboard::Num8:      return KI_8;
		case sf::Keyboard::Num9:      return KI_9;
		case sf::Keyboard::Numpad0:   return KI_NUMPAD0;
		case sf::Keyboard::Numpad1:   return KI_NUMPAD1;
		case sf::Keyboard::Numpad2:   return KI_NUMPAD2;
		case sf::Keyboard::Numpad3:   return KI_NUMPAD3;
		case sf::Keyboard::Numpad4:   return KI_NUMPAD4;
		case sf::Keyboard::Numpad5:   return KI_NUMPAD5;
		case sf::Keyboard::Numpad6:   return KI_NUMPAD6;
		case sf::Keyboard::Numpad7:   return KI_NUMPAD7;
		case sf::Keyboard::Numpad8:   return KI_NUMPAD8;
		case sf::Keyboard::Numpad9:   return KI_NUMPAD9;
		case sf::Keyboard::Left:      return KI_LEFT;
		case sf::Keyboard::Right:     return KI_RIGHT;
		case sf::Keyboard::Up:        return KI_UP;
		case sf::Keyboard::Down:      return KI_DOWN;
		case sf::Keyboard::Add:       return KI_ADD;
		case sf::Keyboard::BackSpace: return KI_BACK;
		case sf::Keyboard::Delete:    return KI_DELETE;
		case sf::Keyboard::Divide:    return KI_DIVIDE;
		case sf::Keyboard::End:       return KI_END;
		case sf::Keyboard::Escape:    return KI_ESCAPE;
		case sf::Keyboard::F1:        return KI_F1;
		case sf::Keyboard::F2:        return KI_F2;
		case sf::Keyboard::F3:        return KI_F3;
		case sf::Keyboard::F4:        return KI_F4;
		case sf::Keyboard::F5:        return KI_F5;
		case sf::Keyboard::F6:        return KI_F6;
		case sf::Keyboard::F7:        return KI_F7;
		case sf::Keyboard::F8:        return KI_F8;
		case sf::Keyboard::F9:        return KI_F9;
		case sf::Keyboard::F10:       return KI_F10;
		case sf::Keyboard::F11:       return KI_F11;
		case sf::Keyboard::F12:       return KI_F12;
		case sf::Keyboard::F13:       return KI_F13;
		case sf::Keyboard::F14:       return KI_F14;
		case sf::Keyboard::F15:       return KI_F15;
		case sf::Keyboard::Home:      return KI_HOME;
		case sf::Keyboard::Insert:    return KI_INSERT;
		case sf::Keyboard::LControl:  return KI_LCONTROL;
		case sf::Keyboard::LShift:    return KI_LSHIFT;
		case sf::Keyboard::Multiply:  return KI_MULTIPLY;
		case sf::Keyboard::Pause:     return KI_PAUSE;
		case sf::Keyboard::RControl:  return KI_RCONTROL;
		case sf::Keyboard::Return:    return KI_RETURN;
		case sf::Keyboard::RShift:    return KI_RSHIFT;
		case sf::Keyboard::Space:     return KI_SPACE;
		case sf::Keyboard::Subtract:  return KI_SUBTRACT;
		case sf::Keyboard::Tab:       return KI_TAB;
		default:                      return KI_UNKNOWN;
		}
	}

	bool process_event(const sf::Event& ev)
	{
		int key_modifier_state = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
			sf::Keyboard::isKeyPressed(sf::Keyboard::RShift))
			key_modifier_state |= Rml::Input::KM_SHIFT;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ||
			sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))
			key_modifier_state |= Rml::Input::KM_CTRL;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) ||
			sf::Keyboard::isKeyPressed(sf::Keyboard::RAlt))
			key_modifier_state |= Rml::Input::KM_ALT;

		switch (ev.type) {
		case sf::Event::Resized:
			_on_window_resized(Rml::Vector2i(ev.size.width, ev.size.height));
			return false;
		case sf::Event::MouseMoved:
			return _context->ProcessMouseMove(ev.mouseMove.x, ev.mouseMove.y, key_modifier_state);
		case sf::Event::MouseButtonPressed:
			return _context->ProcessMouseButtonDown(ev.mouseButton.button, key_modifier_state);
		case sf::Event::MouseButtonReleased:
			return _context->ProcessMouseButtonUp(ev.mouseButton.button, key_modifier_state);
		case sf::Event::MouseWheelMoved:
			return _context->ProcessMouseWheel(float(-ev.mouseWheel.delta), key_modifier_state);
		case sf::Event::MouseLeft:
			return _context->ProcessMouseLeave();
		case sf::Event::TextEntered: {
			Rml::Character c = Rml::Character(ev.text.unicode);
			if (c == Rml::Character('\r'))
				c = Rml::Character('\n');
			if (ev.text.unicode >= 32 || c == Rml::Character('\n'))
				return _context->ProcessTextInput(c);
			else
				return true;
		}
		case sf::Event::KeyPressed:
			if (ev.key.code == sf::Keyboard::Escape)
				_on_escape_key_pressed();
			return _context->ProcessKeyDown(_sfml_key_to_rml_key(ev.key.code), key_modifier_state);
		case sf::Event::KeyReleased:
			return _context->ProcessKeyUp(_sfml_key_to_rml_key(ev.key.code), key_modifier_state);
		default:
			return true;
		}
	}

	void update(float dt)
	{
		update_textbox(dt);
		dirty_all_variables();
		_context->Update();
	}

	void render()
	{
		_render_interface.BeginFrame();
		_context->Render();
		_render_interface.EndFrame();
	}

	bool should_pause_game() {
		return (get_current_menu() != MenuType::Count) || is_textbox_visible();
	}

	Event poll_event()
	{
		Event ev = _event;
		_event = Event::None;
		return ev;
	}
}
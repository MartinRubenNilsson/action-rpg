#include "ui.h"
#include <RmlUi/Debugger.h>
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

	// All documents share this event listener.
	struct CommonEventListener : Rml::EventListener
	{
		void ProcessEvent(Rml::Event& ev) override
		{
			switch (ev.GetId()) {
			case Rml::EventId::Mouseover: {
				if (ev.GetTargetElement()->IsClassSet("menu-button"))
					audio::play("event:/ui/snd_button_hover");
			} break;
			case Rml::EventId::Click: {
				if (ev.GetTargetElement()->IsClassSet("menu-button"))
					audio::play("event:/ui/snd_button_click");
			} break;
			}
		}
	};

	bool debug = false;
	SystemInterface_SFML _system_interface;
	RenderInterface_GL2_SFML _render_interface;
	Rml::Context* _context = nullptr;
	Rml::Context* _debugger_context = nullptr; // The debugger needs its own context to render at the right size.
	CommonEventListener _common_event_listener;
	Event _event;

	void _on_escape_key_pressed()
	{
		MenuType current_menu = get_top_menu();
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

	void _on_click_restart()
	{
		pop_menu();
		_event = Event::RestartMap;
	}

	void _on_click_main_menu()
	{
		set_hud_visible(false);
		pop_all_menus();
		push_menu(MenuType::Main);
		_event = Event::GoToMainMenu;
	}

	void initialize()
	{
		Rml::SetSystemInterface(&_system_interface);
		Rml::SetRenderInterface(&_render_interface);
		Rml::Initialise();
		_context = Rml::CreateContext("main", Rml::Vector2i());
		_debugger_context = Rml::CreateContext("debugger", Rml::Vector2i());
		Rml::Debugger::Initialise(_debugger_context);
		Rml::Debugger::SetContext(_context);
		create_bindings();
		initialize_textbox_presets();
	}

	void shutdown()
	{
		Rml::Shutdown();
		_context = nullptr;
		_debugger_context = nullptr;
	}

	void load_ttf_fonts(const std::filesystem::path& dir)
	{
		for (const std::filesystem::directory_entry& entry :
			std::filesystem::directory_iterator(dir)) {
			if (!entry.is_regular_file()) continue;
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
		}
	}

	void add_event_listeners()
	{
		// Add common event listener to all documents.

		for (int i = 0; i < _context->GetNumDocuments(); ++i) {
			Rml::ElementDocument* doc = _context->GetDocument(i);
			doc->AddEventListener(Rml::EventId::Mouseover, &_common_event_listener);
			doc->AddEventListener(Rml::EventId::Click, &_common_event_listener);
		}

		add_menu_event_listeners();
	}

	Rml::Input::KeyIdentifier _sfml_key_to_rml_key(sf::Keyboard::Key key)
	{
		switch (key) {
		case sf::Keyboard::A:         return Rml::Input::KI_A;
		case sf::Keyboard::B:         return Rml::Input::KI_B;
		case sf::Keyboard::C:         return Rml::Input::KI_C;
		case sf::Keyboard::D:         return Rml::Input::KI_D;
		case sf::Keyboard::E:         return Rml::Input::KI_E;
		case sf::Keyboard::F:         return Rml::Input::KI_F;
		case sf::Keyboard::G:         return Rml::Input::KI_G;
		case sf::Keyboard::H:         return Rml::Input::KI_H;
		case sf::Keyboard::I:         return Rml::Input::KI_I;
		case sf::Keyboard::J:         return Rml::Input::KI_J;
		case sf::Keyboard::K:         return Rml::Input::KI_K;
		case sf::Keyboard::L:         return Rml::Input::KI_L;
		case sf::Keyboard::M:         return Rml::Input::KI_M;
		case sf::Keyboard::N:         return Rml::Input::KI_N;
		case sf::Keyboard::O:         return Rml::Input::KI_O;
		case sf::Keyboard::P:         return Rml::Input::KI_P;
		case sf::Keyboard::Q:         return Rml::Input::KI_Q;
		case sf::Keyboard::R:         return Rml::Input::KI_R;
		case sf::Keyboard::S:         return Rml::Input::KI_S;
		case sf::Keyboard::T:         return Rml::Input::KI_T;
		case sf::Keyboard::U:         return Rml::Input::KI_U;
		case sf::Keyboard::V:         return Rml::Input::KI_V;
		case sf::Keyboard::W:         return Rml::Input::KI_W;
		case sf::Keyboard::X:         return Rml::Input::KI_X;
		case sf::Keyboard::Y:         return Rml::Input::KI_Y;
		case sf::Keyboard::Z:         return Rml::Input::KI_Z;
		case sf::Keyboard::Num0:      return Rml::Input::KI_0;
		case sf::Keyboard::Num1:      return Rml::Input::KI_1;
		case sf::Keyboard::Num2:      return Rml::Input::KI_2;
		case sf::Keyboard::Num3:      return Rml::Input::KI_3;
		case sf::Keyboard::Num4:      return Rml::Input::KI_4;
		case sf::Keyboard::Num5:      return Rml::Input::KI_5;
		case sf::Keyboard::Num6:      return Rml::Input::KI_6;
		case sf::Keyboard::Num7:      return Rml::Input::KI_7;
		case sf::Keyboard::Num8:      return Rml::Input::KI_8;
		case sf::Keyboard::Num9:      return Rml::Input::KI_9;
		case sf::Keyboard::Numpad0:   return Rml::Input::KI_NUMPAD0;
		case sf::Keyboard::Numpad1:   return Rml::Input::KI_NUMPAD1;
		case sf::Keyboard::Numpad2:   return Rml::Input::KI_NUMPAD2;
		case sf::Keyboard::Numpad3:   return Rml::Input::KI_NUMPAD3;
		case sf::Keyboard::Numpad4:   return Rml::Input::KI_NUMPAD4;
		case sf::Keyboard::Numpad5:   return Rml::Input::KI_NUMPAD5;
		case sf::Keyboard::Numpad6:   return Rml::Input::KI_NUMPAD6;
		case sf::Keyboard::Numpad7:   return Rml::Input::KI_NUMPAD7;
		case sf::Keyboard::Numpad8:   return Rml::Input::KI_NUMPAD8;
		case sf::Keyboard::Numpad9:   return Rml::Input::KI_NUMPAD9;
		case sf::Keyboard::Left:      return Rml::Input::KI_LEFT;
		case sf::Keyboard::Right:     return Rml::Input::KI_RIGHT;
		case sf::Keyboard::Up:        return Rml::Input::KI_UP;
		case sf::Keyboard::Down:      return Rml::Input::KI_DOWN;
		case sf::Keyboard::Add:       return Rml::Input::KI_ADD;
		case sf::Keyboard::BackSpace: return Rml::Input::KI_BACK;
		case sf::Keyboard::Delete:    return Rml::Input::KI_DELETE;
		case sf::Keyboard::Divide:    return Rml::Input::KI_DIVIDE;
		case sf::Keyboard::End:       return Rml::Input::KI_END;
		case sf::Keyboard::Escape:    return Rml::Input::KI_ESCAPE;
		case sf::Keyboard::F1:        return Rml::Input::KI_F1;
		case sf::Keyboard::F2:        return Rml::Input::KI_F2;
		case sf::Keyboard::F3:        return Rml::Input::KI_F3;
		case sf::Keyboard::F4:        return Rml::Input::KI_F4;
		case sf::Keyboard::F5:        return Rml::Input::KI_F5;
		case sf::Keyboard::F6:        return Rml::Input::KI_F6;
		case sf::Keyboard::F7:        return Rml::Input::KI_F7;
		case sf::Keyboard::F8:        return Rml::Input::KI_F8;
		case sf::Keyboard::F9:        return Rml::Input::KI_F9;
		case sf::Keyboard::F10:       return Rml::Input::KI_F10;
		case sf::Keyboard::F11:       return Rml::Input::KI_F11;
		case sf::Keyboard::F12:       return Rml::Input::KI_F12;
		case sf::Keyboard::F13:       return Rml::Input::KI_F13;
		case sf::Keyboard::F14:       return Rml::Input::KI_F14;
		case sf::Keyboard::F15:       return Rml::Input::KI_F15;
		case sf::Keyboard::Home:      return Rml::Input::KI_HOME;
		case sf::Keyboard::Insert:    return Rml::Input::KI_INSERT;
		case sf::Keyboard::LControl:  return Rml::Input::KI_LCONTROL;
		case sf::Keyboard::LShift:    return Rml::Input::KI_LSHIFT;
		case sf::Keyboard::Multiply:  return Rml::Input::KI_MULTIPLY;
		case sf::Keyboard::Pause:     return Rml::Input::KI_PAUSE;
		case sf::Keyboard::RControl:  return Rml::Input::KI_RCONTROL;
		case sf::Keyboard::Return:    return Rml::Input::KI_RETURN;
		case sf::Keyboard::RShift:    return Rml::Input::KI_RSHIFT;
		case sf::Keyboard::Space:     return Rml::Input::KI_SPACE;
		case sf::Keyboard::Subtract:  return Rml::Input::KI_SUBTRACT;
		case sf::Keyboard::Tab:       return Rml::Input::KI_TAB;
		default:                      return Rml::Input::KI_UNKNOWN;
		}
	}

	void process_event(const sf::Event& ev)
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
		case sf::Event::Resized: {
			_render_interface.SetViewport(ev.size.width, ev.size.height);
			_context->SetDimensions(Rml::Vector2i(ev.size.width, ev.size.height));
			_debugger_context->SetDimensions(Rml::Vector2i(ev.size.width, ev.size.height));
			float dp_ratio_x = (float)ev.size.width / (float)window::VIEW_SIZE.x;
			float dp_ratio_y = (float)ev.size.height / (float)window::VIEW_SIZE.y;
			float dp_ratio = std::min(dp_ratio_x, dp_ratio_y);
			_context->SetDensityIndependentPixelRatio(dp_ratio);
			// Don't set density independent pixel ratio for the debugger context!
			// It should always be 1.0, so that it remains the same size.
		} break;
		case sf::Event::MouseMoved:
			_context->ProcessMouseMove(ev.mouseMove.x, ev.mouseMove.y, key_modifier_state);
			_debugger_context->ProcessMouseMove(ev.mouseMove.x, ev.mouseMove.y, key_modifier_state);
			break;
		case sf::Event::MouseButtonPressed:
			_context->ProcessMouseButtonDown(ev.mouseButton.button, key_modifier_state);
			_debugger_context->ProcessMouseButtonDown(ev.mouseButton.button, key_modifier_state);
			break;
		case sf::Event::MouseButtonReleased:
			_context->ProcessMouseButtonUp(ev.mouseButton.button, key_modifier_state);
			_debugger_context->ProcessMouseButtonUp(ev.mouseButton.button, key_modifier_state);
			break;
		case sf::Event::MouseWheelMoved:
			_context->ProcessMouseWheel(float(-ev.mouseWheel.delta), key_modifier_state);
			_debugger_context->ProcessMouseWheel(float(-ev.mouseWheel.delta), key_modifier_state);
			break;
		case sf::Event::MouseLeft:
			_context->ProcessMouseLeave();
			_debugger_context->ProcessMouseLeave();
			break;
		case sf::Event::TextEntered: {
			Rml::Character c = Rml::Character(ev.text.unicode);
			if (c == Rml::Character('\r'))
				c = Rml::Character('\n');
			if (ev.text.unicode >= 32 || c == Rml::Character('\n')) {
				_context->ProcessTextInput(c);
				_debugger_context->ProcessTextInput(c);
			}
		} break;
		case sf::Event::KeyPressed:
			if (ev.key.code == sf::Keyboard::Escape)
				_on_escape_key_pressed();
			_context->ProcessKeyDown(_sfml_key_to_rml_key(ev.key.code), key_modifier_state);
			_debugger_context->ProcessKeyDown(_sfml_key_to_rml_key(ev.key.code), key_modifier_state);
			break;
		case sf::Event::KeyReleased:
			_context->ProcessKeyUp(_sfml_key_to_rml_key(ev.key.code), key_modifier_state);
			_debugger_context->ProcessKeyUp(_sfml_key_to_rml_key(ev.key.code), key_modifier_state);
			break;
		}
	}

	void update(float dt)
	{
		update_textbox(dt);
		dirty_all_variables();
		if (debug != Rml::Debugger::IsVisible())
			Rml::Debugger::SetVisible(debug);
		_context->Update();
		_debugger_context->Update();
	}

	void render(sf::RenderTarget& target)
	{
		_render_interface.BeginFrame();
		_context->Render();
		_debugger_context->Render();
		_render_interface.EndFrame();
		target.resetGLStates();
	}

	void reload_styles()
	{
		for (int i = 0; i < _context->GetNumDocuments(); ++i)
			_context->GetDocument(i)->ReloadStyleSheet();
	}

	void show_document(const std::string& name)
	{
		if (Rml::ElementDocument* doc = _context->GetDocument(name))
			doc->Show();
	}

	Event poll_event()
	{
		Event ev = _event;
		_event = Event::None;
		return ev;
	}

	bool should_pause_game() {
		return (get_top_menu() != MenuType::Count) || is_textbox_visible();
	}
}
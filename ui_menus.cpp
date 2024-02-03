#include "stdafx.h"
#include "ui_menus.h"
#include <RmlUi/Core.h>
#include "settings.h"

namespace ui
{
	const float _VOLUME_RANGE_MAX_VALUE = 100.f;

	Rml::ElementDocument* _get_menu_document(MenuType type); // Forward declaration

	struct SettingsMenuEventListener : public Rml::EventListener
	{
		void ProcessEvent(Rml::Event& ev) override
		{
			switch (ev.GetId())
			{
			case Rml::EventId::Show: {
				Settings settings{};
				settings.get();
				if (Rml::ElementDocument* doc = _get_menu_document(MenuType::Settings)) {
					if (Rml::Element* el = doc->GetElementById("checkbox-fullscreen"))
						settings.fullscreen ? el->SetAttribute("checked", "true") : el->RemoveAttribute("checked");
					if (Rml::Element* el = doc->GetElementById("range-volume-master"))
						el->SetAttribute("value", std::to_string(settings.volume_master * _VOLUME_RANGE_MAX_VALUE));
					if (Rml::Element* el = doc->GetElementById("range-volume-sound"))
						el->SetAttribute("value", std::to_string(settings.volume_sound * _VOLUME_RANGE_MAX_VALUE));
					if (Rml::Element* el = doc->GetElementById("range-volume-music"))
						el->SetAttribute("value", std::to_string(settings.volume_music * _VOLUME_RANGE_MAX_VALUE));
				}
			} break;
			case Rml::EventId::Submit: {
				std::string type = ev.GetType();
				Settings settings{};
				settings.get();
				settings.fullscreen = (ev.GetParameter("fullscreen", Rml::String("off")) == "on");
				settings.volume_master = ev.GetParameter("volume-master", _VOLUME_RANGE_MAX_VALUE) / _VOLUME_RANGE_MAX_VALUE;
				settings.volume_sound = ev.GetParameter("volume-sound", _VOLUME_RANGE_MAX_VALUE) / _VOLUME_RANGE_MAX_VALUE;
				settings.volume_music = ev.GetParameter("volume-music", _VOLUME_RANGE_MAX_VALUE) / _VOLUME_RANGE_MAX_VALUE;
				settings.set();
			} break;
		}
		}
	};

	extern Rml::Context* _context;
	SettingsMenuEventListener _settings_menu_event_listener;
	std::vector<MenuType> _menu_stack;

	Rml::ElementDocument* _get_menu_document(MenuType type)
	{
		switch (type) {
		case MenuType::Main:
			return _context->GetDocument("main_menu");
		case MenuType::Pause:
			return _context->GetDocument("pause_menu");
		case MenuType::Settings:
			return _context->GetDocument("settings_menu");
		case MenuType::Credits:
			return _context->GetDocument("credits_menu");
		default:
			return nullptr;
		}
	}

	void add_menu_event_listeners()
	{
		if (Rml::ElementDocument* doc = _get_menu_document(MenuType::Settings)) {
			doc->AddEventListener(Rml::EventId::Show, &_settings_menu_event_listener);
			doc->AddEventListener(Rml::EventId::Submit, &_settings_menu_event_listener);
		}
	}

	MenuType get_top_menu() {
		return _menu_stack.empty() ? MenuType::Count : _menu_stack.back();
	}

	void _set_menu_visible(MenuType type, bool visible)
	{
		if (Rml::ElementDocument* doc = _get_menu_document(type))
			visible ? doc->Show() : doc->Hide();
	}

	void push_menu(MenuType type)
	{
		if (!_menu_stack.empty())
			_set_menu_visible(_menu_stack.back(), false);
		_menu_stack.push_back(type);
		_set_menu_visible(type, true);
	}

	void pop_menu()
	{
		if (_menu_stack.empty()) return;
		_set_menu_visible(_menu_stack.back(), false);
		_menu_stack.pop_back();
		if (_menu_stack.empty()) return;
		_set_menu_visible(_menu_stack.back(), true);
	}

	void pop_all_menus()
	{
		_menu_stack.clear();
		for (int i = 0; i < (int)MenuType::Count; ++i)
			_set_menu_visible((MenuType)i, false);
	}
}
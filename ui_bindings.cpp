#include "stdafx.h"
#include "ui_bindings.h"
#include "ui_hud.h"
#include "ui_textbox.h"
#include "console.h"

namespace ui
{
	// CALLBACKS

	void _console_log(const std::string& message) { console::log(message); }
	extern void _on_click_play();
	extern void _on_click_settings();
	extern void _on_click_credits();
	extern void _on_click_quit();
	extern void _on_click_back();
	extern void _on_click_resume();
	extern void _on_click_restart();
	extern void _on_click_main_menu();

	extern Rml::Context* _context;
	Rml::DataModelHandle _data_model_handle;

	bool is_variable_dirty(const std::string& name) {
		return _data_model_handle.IsVariableDirty(name);
	}

	void dirty_all_variables() {
		_data_model_handle.DirtyAllVariables();
	}

	template <void (*Func)()>
	Rml::DataEventFunc _wrap() {
		return [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { if (Func) Func(); };
	}

	void create_bindings()
	{
		Rml::DataModelConstructor data_model = _context->CreateDataModel("data_model");
		if (!data_model) return;
		_data_model_handle = data_model.GetModelHandle();

		// REGISTER TYPES

		data_model.RegisterArray<std::vector<std::string>>();

		// BIND VARIABLES

		data_model.Bind("hud_player_health", &bindings::hud_player_health);
		data_model.Bind("textbox_text", &bindings::textbox_text);
		data_model.Bind("textbox_has_sprite", &bindings::textbox_has_sprite);
		data_model.Bind("textbox_sprite", &bindings::textbox_sprite);
		data_model.Bind("textbox_has_options", &bindings::textbox_has_options);
		data_model.Bind("textbox_options", &bindings::textbox_options);

		// BIND FUNCTIONS

		data_model.BindEventCallback("console_log",
			[](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args) {
				if (args.size() != 1) return;
				_console_log(args[0].Get<std::string>());
			});
		data_model.BindEventCallback("on_click_play", _wrap<_on_click_play>());
		data_model.BindEventCallback("on_click_settings", _wrap<_on_click_settings>());
		data_model.BindEventCallback("on_click_credits", _wrap<_on_click_credits>());
		data_model.BindEventCallback("on_click_quit", _wrap<_on_click_quit>());
		data_model.BindEventCallback("on_click_back", _wrap<_on_click_back>());
		data_model.BindEventCallback("on_click_resume", _wrap<_on_click_resume>());
		data_model.BindEventCallback("on_click_restart", _wrap<_on_click_restart>());
		data_model.BindEventCallback("on_click_main_menu", _wrap<_on_click_main_menu>());
	}
}

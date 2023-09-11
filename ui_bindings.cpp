#include "ui_bindings.h"
#include <RmlUi/Core.h>
#include "console.h"

namespace ui
{
	// DATA VARIABLES

	extern std::string _textbox_text;
	extern std::string _textbox_sprite;
	extern bool _textbox_sprite_is_set;

	// EVENT CALLBACKS

	void _console_log(const std::string& message) {
		console::log(message);
	}

	extern void _on_textbox_keydown(int key);

	// FUNCTIONS

	extern Rml::Context* _context;
	Rml::DataModelHandle _data_model_handle;

	bool is_variable_dirty(const std::string& name) {
		return _data_model_handle.IsVariableDirty(name);
	}

	void dirty_all_variables() {
		_data_model_handle.DirtyAllVariables();
	}

	void create_bindings()
	{
		auto data_model = _context->CreateDataModel("data_model");
		if (!data_model) return;
		_data_model_handle = data_model.GetModelHandle();

		// DATA VARIABLES

		data_model.Bind("textbox_text", &_textbox_text);
		data_model.Bind("textbox_sprite", &_textbox_sprite);
		data_model.Bind("textbox_sprite_is_set", &_textbox_sprite_is_set);

		// EVENT CALLBACKS

		data_model.BindEventCallback("console_log",
			[](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args) {
				if (args.size() != 1) return;
				_console_log(args[0].Get<std::string>());
			});
		data_model.BindEventCallback("on_textbox_keydown",
			[](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList&) {
				if (event != Rml::EventId::Keydown) return;
				_on_textbox_keydown(event.GetParameter<int>("key_identifier", 0));
			});
	}
}

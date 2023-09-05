#include "ui_bindings.h"
#include <RmlUi/Core.h>

namespace ui
{
	extern Rml::Context* _context;
	Rml::DataModelHandle _data_model_handle;

	// DATA VARIABLES

	std::string textbox_sprite = "iron-helmet";
	std::string textbox_text;

	// EVENT CALLBACKS

	void (*console_log)(const std::string& message) = nullptr;

	// INTERFACE  - DON'T TOUCH

	void create_bindings()
	{
		auto data_model = _context->CreateDataModel("data_model");
		if (!data_model) return;
		_data_model_handle = data_model.GetModelHandle();

		// DATA VARIABLES

		data_model.Bind("textbox_sprite", &textbox_sprite);
		data_model.Bind("textbox_text", &textbox_text);

		// EVENT CALLBACKS

		data_model.BindEventCallback("console_log",
			[](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args) {
				if (!console_log) return;
				if (args.size() != 1) return;
				console_log(args[0].Get<std::string>());
			});
	}

	bool is_variable_dirty(const std::string& name) {
		return _data_model_handle.IsVariableDirty(name);
	}

	void dirty_all_variables() {
		_data_model_handle.DirtyAllVariables();
	}
}

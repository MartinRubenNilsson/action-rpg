#include "ui_bindings.h"
#include <RmlUi/Core.h>

namespace ui
{
	extern Rml::Context* _context;
	Rml::DataModelHandle _data_model_handle;

	// DATA VARIABLES

	std::string test_string = "Bruh";

	// EVENT CALLBACKS

	void (*console_log)(const std::string& message) = nullptr;

	void create_bindings()
	{
		auto data_model = _context->CreateDataModel("data-model");
		if (!data_model) return;
		_data_model_handle = data_model.GetModelHandle();

		// DATA VARIABLES

		data_model.Bind("test_string", &test_string);

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

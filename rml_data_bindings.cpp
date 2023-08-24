#include "rml_data_bindings.h"
#include <RmlUi/Core.h>

namespace rml
{
	extern Rml::Context* _context;
	Rml::DataModelHandle data_model_handle;

	bool is_variable_dirty(const std::string& name) {
		return data_model_handle.IsVariableDirty(name);
	}

	void dirty_variable(const std::string& name) {
		data_model_handle.DirtyVariable(name);
	}

	// DATA VARIABLES

	std::string test_string;

	// EVENT CALLBACKS

	void _play_game(Rml::DataModelHandle, Rml::Event& ev, const Rml::VariantList&)
	{
		if (auto doc = _context->GetDocument("main_menu.rml"))
			doc->Hide();
	}

	void _quit_game(Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
	{
		//close_window();
	}

	// CREATE BINDINGS

	void _create_data_bindings()
	{
		auto data_model = _context->CreateDataModel("data_model");
		if (!data_model)
			return;
		data_model_handle = data_model.GetModelHandle();

		// DATA VARIABLES

		data_model.Bind("test_string", &test_string);

		// EVENT CALLBACKS

		data_model.BindEventCallback("play_game", _play_game);
		data_model.BindEventCallback("quit_game", _quit_game);
	}
}

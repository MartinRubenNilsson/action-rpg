#include "ui_bindings.h"
#include <RmlUi/Core.h>

namespace ui
{
	extern Rml::Context* _context;
	Rml::DataModelHandle _data_model_handle;

	bool is_variable_dirty(const std::string& name) {
		return _data_model_handle.IsVariableDirty(name);
	}

	void dirty_variable(const std::string& name) {
		_data_model_handle.DirtyVariable(name);
	}

	void dirty_all_variables() {
		_data_model_handle.DirtyAllVariables();
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

	void _create_bindings()
	{
		auto data_model = _context->CreateDataModel("data_model");
		if (!data_model) return;
		_data_model_handle = data_model.GetModelHandle();

		data_model.Bind("test_string", &test_string);
		data_model.BindEventCallback("play_game", _play_game);
		data_model.BindEventCallback("quit_game", _quit_game);
	}
}

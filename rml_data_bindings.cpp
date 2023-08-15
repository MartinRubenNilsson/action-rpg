#include "rml_data_bindings.h"
#include <RmlUi/Core.h>

namespace rml
{
	Rml::DataModelHandle data_model_handle;

	bool is_variable_dirty(const std::string& name)
	{
		return data_model_handle.IsVariableDirty(name);
	}

	void dirty_variable(const std::string& name)
	{
		data_model_handle.DirtyVariable(name);
	}

	// DATA VARIABLES

	std::string test_string;

	// EVENT CALLBACKS

	void _on_click(Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
	{
		std::cout << "You pressed play!" << std::endl;
	}

	// CREATE BINDINGS

	void _create_data_bindings(Rml::Context* context)
	{
		auto data_model = context->CreateDataModel("data_model");
		if (!data_model)
			return;
		data_model_handle = data_model.GetModelHandle();

		// DATA VARIABLES

		data_model.Bind("test_string", &test_string);

		// EVENT CALLBACKS

		data_model.BindEventCallback("test_func", _on_click);
	}
}

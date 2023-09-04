#include "ui_textbox.h"
#include <RmlUi/Core.h>
#include "console.h"

namespace ui
{
	extern Rml::Context* _context;

	// TODO: make event listener

	struct EventListener : Rml::EventListener
	{
		void ProcessEvent(Rml::Event& event) override
		{
			if (event == "keydown")
			{
				console::log_error("Shit");
			}
		}
	};

	void initialize_textbox()
	{

	}

	void set_textbox_text(const std::string& rml)
	{
		if (auto document = _context->GetDocument("textbox"))
			if (auto element = document->GetElementById("textbox-text"))
				element->SetInnerRML(rml.c_str());
	}

	//void update_textbox()
	//{
	//	// TODO
	//}
}
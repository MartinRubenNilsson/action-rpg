#include "ui_textbox.h"
#include <RmlUi/Core.h>

namespace ui
{
	extern Rml::Context* _context;

	// TODO: make event listener

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
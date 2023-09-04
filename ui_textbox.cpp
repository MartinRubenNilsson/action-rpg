#include "ui_textbox.h"
#include <RmlUi/Core.h>
#include "console.h"

namespace ui
{
	extern Rml::Context* _context;

	struct TextboxEventListener : Rml::EventListener
	{
		void ProcessEvent(Rml::Event& event) override
		{
			if (event == Rml::EventId::Keydown)
			{

			}
		}
	} _textbox_event_listener;

	void add_textbox_event_listeners()
	{
		if (auto document = _context->GetDocument("textbox"))
			document->AddEventListener(Rml::EventId::Keydown, &_textbox_event_listener);
	}

	void set_textbox_text(const std::string& rml)
	{
		if (auto document = _context->GetDocument("textbox"))
			if (auto element = document->GetElementById("textbox-text"))
				element->SetInnerRML(rml.c_str());
	}
}
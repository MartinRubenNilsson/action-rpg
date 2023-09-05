#include "ui_textbox.h"
#include <RmlUi/Core.h>
#include "ui_bindings.h"
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
				// PITFALL: Attempting to get a parameter of type
				// Rml::Input::KeyIdentifier will crash the program.
				int key = event.GetParameter<int>("key_identifier", 0);
				//set_textbox_text("Key pressed: " + std::to_string(key));
			}
		}
	} _textbox_event_listener;

	void update_textbox()
	{
		if (auto document = _context->GetDocument("textbox"))
		{
			// NOTE: AddEventListener() is safe to call multiple times.
			document->AddEventListener(Rml::EventId::Keydown, &_textbox_event_listener);
		}
	}

	void set_textbox_text(const std::string& text) {
		textbox_text = text;
	}

	void set_textbox_sprite(const std::string& sprite) {
		textbox_sprite = sprite;
	}
}
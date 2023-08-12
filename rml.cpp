#include "rml.h"
#include "RmlUi_Backend.h"
#include <RmlUi/Core.h>

namespace rml
{
	Rml::Context* _context = nullptr;

	bool _show_text = true;
	std::string _animal = "cat";

	void startup_rml(sf::RenderWindow* window)
	{
		Backend::Initialize(window);
		Rml::SetSystemInterface(Backend::GetSystemInterface());
		Rml::SetRenderInterface(Backend::GetRenderInterface());
		Rml::Initialise();
		_context = Rml::CreateContext("main", Rml::Vector2i(window->getSize().x, window->getSize().y));

		// Tell RmlUi to load the given fonts.
		Rml::LoadFontFace("assets/fonts/LatoLatin-Regular.ttf");
		// Fonts can be registered as fallback fonts, as in this case to display emojis.
		Rml::LoadFontFace("assets/fonts/NotoEmoji-Regular.ttf", true);

		// Set up data bindings to synchronize application data.
		if (Rml::DataModelConstructor constructor = _context->CreateDataModel("animals"))
		{
			constructor.Bind("show_text", &_show_text);
			constructor.Bind("animal", &_animal);
		}
	}

	void cleanup_rml()
	{
		Rml::RemoveContext(_context->GetName());
		Rml::Shutdown();
		Backend::Shutdown();
	}

	void update_rml()
	{
		Backend::ProcessEvents(_context);
		_context->Update();
	}

	void render_rml()
	{
		Backend::BeginFrame();
		_context->Render();
		Backend::PresentFrame();
	}

	bool load_rml(const std::string& filename)
	{
		Rml::ElementDocument* document = _context->LoadDocument(filename);
		if (!document)
			return false;

		document->Show();

		// Replace and style some text in the loaded document.
		Rml::Element* element = document->GetElementById("world");
		element->SetInnerRML(reinterpret_cast<const char*>(u8"🌍"));
		element->SetProperty("font-size", "1.5em");

		return true;
	}
}
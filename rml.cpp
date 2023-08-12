#include "rml.h"
#include "RmlUi_Backend.h"
#include <RmlUi/Core.h>

namespace rml
{
	Rml::Context* _context = nullptr;

	void startup(sf::RenderWindow* window)
	{
		Backend::Initialize(window);
		Rml::SetSystemInterface(Backend::GetSystemInterface());
		Rml::SetRenderInterface(Backend::GetRenderInterface());
		Rml::Initialise();
		_context = Rml::CreateContext("main", Rml::Vector2i(window->getSize().x, window->getSize().y));

		// Load all fonts in the fonts folder.
		for (const auto& entry : std::filesystem::directory_iterator("assets/fonts"))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".ttf")
				Rml::LoadFontFace(entry.path().string());
		}
	}

	void cleanup()
	{
		Rml::RemoveContext(_context->GetName());
		Rml::Shutdown();
		Backend::Shutdown();
	}

	void process_event(const sf::Event& ev)
	{
		Backend::ProcessEvent(_context, ev);
	}

	void update()
	{
		_context->Update();
	}

	void render()
	{
		Backend::BeginFrame();
		_context->Render();
		Backend::PresentFrame();
	}

	bool load_document(const std::string& filename)
	{
		Rml::ElementDocument* document = _context->LoadDocument(filename);
		if (!document)
			return false;
		document->Show();
		return true;
	}
}
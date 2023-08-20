#include "rml.h"
#include "RmlUi_Backend.h"
#include <RmlUi/Core.h>

namespace rml
{
	Rml::Context* _context = nullptr;

	extern void _create_data_bindings();

	void initialize(sf::RenderWindow* window)
	{
		Backend::Initialize(window);
		Rml::SetSystemInterface(Backend::GetSystemInterface());
		Rml::SetRenderInterface(Backend::GetRenderInterface());
		Rml::Initialise();

		Rml::Vector2i window_size(window->getSize().x, window->getSize().y);
		_context = Rml::CreateContext("main", window_size);

		_create_data_bindings();
	}

	void shutdown()
	{
		Rml::RemoveContext(_context->GetName());
		Rml::Shutdown();
		Backend::Shutdown();
	}

	void process_event(const sf::Event& event)
	{
		Backend::ProcessEvent(_context, event);
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

	void load_fonts_and_documents()
	{
		// Load all TTF fonts.
		for (const auto& entry : std::filesystem::directory_iterator("assets/fonts"))
		{
			if (entry.path().extension() == ".ttf")
				Rml::LoadFontFace(entry.path().string());
		}

		// Load all RML documents and set their IDs to their names.
		for (const auto& entry : std::filesystem::directory_iterator("assets/rml"))
		{
			if (entry.path().extension() != ".rml")
				continue;
			auto document = _context->LoadDocument(entry.path().string());
			if (!document)
				continue;
			document->SetId(entry.path().stem().string());
		}
	}

	std::vector<std::string> get_list()
	{
		std::vector<std::string> names;
		for (int i = 0; i < _context->GetNumDocuments(); ++i)
			names.push_back(_context->GetDocument(i)->GetId());
		return names;
	}

	void show(const std::string& name)
	{
		if (auto document = _context->GetDocument(name))
			document->Show();
	}

	void hide(const std::string& name)
	{
		if (auto document = _context->GetDocument(name))
			document->Hide();
	}
}
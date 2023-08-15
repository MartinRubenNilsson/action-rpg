#include "rml.h"
#include "asset_directories.h"
#include "RmlUi_Backend.h"
#include <RmlUi/Core.h>

namespace rml
{
	Rml::Context* _context = nullptr;

	extern void _create_data_bindings(Rml::Context* context);

	void startup(sf::RenderWindow* window)
	{
		Backend::Initialize(window);
		Rml::SetSystemInterface(Backend::GetSystemInterface());
		Rml::SetRenderInterface(Backend::GetRenderInterface());
		Rml::Initialise();

		Rml::Vector2i initial_window_size(window->getSize().x, window->getSize().y);
		_context = Rml::CreateContext("main", initial_window_size);

		_create_data_bindings(_context);
	}

	void cleanup()
	{
		Rml::RemoveContext(_context->GetName());
		Rml::Shutdown();
		Backend::Shutdown();
	}

	void load_assets()
	{
		// Load all TTF fonts.
		for (const auto& entry : std::filesystem::directory_iterator(ASSET_DIR_FONTS))
		{
			if (entry.path().extension() == ".ttf")
				Rml::LoadFontFace(entry.path().string());
		}

		// Load all RML documents and set their IDs to their filenames.
		for (const auto& entry : std::filesystem::directory_iterator(ASSET_DIR_RML))
		{
			if (entry.path().extension() != ".rml")
				continue;
			auto document = _context->LoadDocument(entry.path().string());
			if (!document)
				continue;
			document->SetId(entry.path().filename().string());
		}
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

	void show_document(const std::string& filename)
	{
		if (auto document = _context->GetDocument(filename))
			document->Show();
	}

	void hide_document(const std::string& filename)
	{
		if (auto document = _context->GetDocument(filename))
			document->Hide();
	}
}
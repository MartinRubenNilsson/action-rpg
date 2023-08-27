#include "ui.h"
#include <RmlUi/Core.h>
#include "RmlUi_Platform_SFML.h"
#include "RmlUi_Renderer_GL2_SFML.h"

namespace ui
{
	SystemInterface_SFML* _system_interface = nullptr;
	RenderInterface_GL2_SFML* _render_interface = nullptr;
	Rml::Context* _context = nullptr;

	void _on_window_resize(const Rml::Vector2i& size)
	{
		_render_interface->SetViewport(size.x, size.y);
		_context->SetDimensions(size);
	}

	extern void _create_bindings();

	void initialize(sf::RenderWindow& window)
	{
		_system_interface = new SystemInterface_SFML();
		_system_interface->SetWindow(&window); // So that the system interface can set the mouse cursor.
		_render_interface = new RenderInterface_GL2_SFML();

		Rml::SetSystemInterface(_system_interface);
		Rml::SetRenderInterface(_render_interface);
		Rml::Initialise();

		Rml::Vector2i window_size(window.getSize().x, window.getSize().y);
		_context = Rml::CreateContext("main", window_size);
		_on_window_resize(window_size);

		_create_bindings();
	}

	void shutdown()
	{
		Rml::RemoveContext(_context->GetName());
		Rml::Shutdown();
		delete _system_interface; _system_interface = nullptr;
		delete _render_interface; _render_interface = nullptr;
	}

	void process_event(const sf::Event& event)
	{
		if (event.type == sf::Event::Resized)
		{
			Rml::Vector2i size(event.size.height, event.size.height);
			_on_window_resize(size);
		}
		else
		{
			RmlSFML::InputHandler(_context, event);
		}
	}

	void update() {
		_context->Update();
	}

	void render()
	{
		_render_interface->BeginFrame();
		_context->Render();
		_render_interface->EndFrame();
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
		for (const auto& entry : std::filesystem::directory_iterator("assets/ui"))
		{
			if (entry.path().extension() != ".rml")
				continue;
			auto document = _context->LoadDocument(entry.path().string());
			if (!document)
				continue;
			document->SetId(entry.path().stem().string());
		}
	}

	std::vector<std::string> get_loaded_documents()
	{
		std::vector<std::string> names;
		for (int i = 0; i < _context->GetNumDocuments(); ++i)
			names.push_back(_context->GetDocument(i)->GetId());
		return names;
	}

	void show(const std::string& document_name)
	{
		if (auto document = _context->GetDocument(document_name))
			document->Show();
	}

	void hide(const std::string& document_name)
	{
		if (auto document = _context->GetDocument(document_name))
			document->Hide();
	}
}
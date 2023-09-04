#include "ui.h"
#include <RmlUi/Core.h>
#include "RmlUi_Platform_SFML.h"
#include "RmlUi_Renderer_GL2_SFML.h"
#include "ui_bindings.h"
#include "defines.h"
#include "ui_textbox.h"

namespace ui
{
	SystemInterface_SFML _system_interface;
	RenderInterface_GL2_SFML _render_interface;
	Rml::Context* _context = nullptr;

	void _on_window_resized(const Rml::Vector2i& new_size)
	{
		_render_interface.SetViewport(new_size.x, new_size.y);
		_context->SetDimensions(new_size);
		float dp_ratio_x = (float)new_size.x / (float)WINDOW_VIEW_WIDTH;
		float dp_ratio_y = (float)new_size.y / (float)WINDOW_VIEW_HEIGHT;
		float dp_ratio = std::min(dp_ratio_x, dp_ratio_y);
		_context->SetDensityIndependentPixelRatio(dp_ratio);
	}

	void initialize(sf::RenderWindow& window)
	{
		_system_interface.SetWindow(&window); // So that the system interface can set the mouse cursor.

		Rml::SetSystemInterface(&_system_interface);
		Rml::SetRenderInterface(&_render_interface);
		Rml::Initialise();

		Rml::Vector2i size(window.getSize().x, window.getSize().y);
		_context = Rml::CreateContext("main", size);
		_on_window_resized(size);

		create_bindings();
	}

	void shutdown()
	{
		Rml::RemoveContext(_context->GetName()); _context = nullptr;
		Rml::Shutdown();
	}

	void process_event(const sf::Event& event)
	{
		if (event.type == sf::Event::Resized)
			_on_window_resized(Rml::Vector2i(event.size.width, event.size.height));
		RmlSFML::InputHandler(_context, event);
	}

	void update()
	{
		dirty_all_variables();
		_context->Update();
	}

	void render()
	{
		_render_interface.BeginFrame();
		_context->Render();
		_render_interface.EndFrame();
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

		add_textbox_event_listeners();
	}

	void reload_style_sheets()
	{
		for (int i = 0; i < _context->GetNumDocuments(); ++i)
			_context->GetDocument(i)->ReloadStyleSheet();
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
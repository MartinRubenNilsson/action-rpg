#include "ui.h"
#include <RmlUi/Core.h>
#include "RmlUi_Platform_SFML.h"
#include "RmlUi_Renderer_GL2_SFML.h"
#include "defines.h"
#include "ui_bindings.h"
#include "ui_main_menu.h"
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
		float dp_ratio_x = (float)new_size.x / (float)VIEW_PIXEL_WIDTH;
		float dp_ratio_y = (float)new_size.y / (float)VIEW_PIXEL_HEIGHT;
		float dp_ratio = std::min(dp_ratio_x, dp_ratio_y);
		_context->SetDensityIndependentPixelRatio(dp_ratio);
	}

	void initialize(sf::RenderWindow& window)
	{
		_system_interface.SetWindow(&window); // So that the system interface can set the mouse cursor.
		Rml::SetSystemInterface(&_system_interface);
		Rml::SetRenderInterface(&_render_interface);
		Rml::Initialise();
		Rml::Vector2i window_size(window.getSize().x, window.getSize().y);
		_context = Rml::CreateContext("main", window_size);
		_on_window_resized(window_size);
		create_bindings();
		create_textbox_presets();
	}

	void shutdown()
	{
		Rml::RemoveContext(_context->GetName());
		_context = nullptr;
		Rml::Shutdown();
	}

	void load_ttf_fonts(const std::filesystem::path& dir)
	{
		for (const std::filesystem::directory_entry& entry :
			std::filesystem::directory_iterator(dir)) {
			if (entry.path().extension() != ".ttf") continue;
			Rml::LoadFontFace(entry.path().string());
		}
	}

	void load_rml_documents(const std::filesystem::path& dir)
	{
		for (const std::filesystem::directory_entry& entry :
			std::filesystem::directory_iterator(dir)) {
			if (entry.path().extension() != ".rml") continue;
			Rml::ElementDocument* doc = _context->LoadDocument(entry.path().string());
			if (!doc) continue;
			doc->SetId(entry.path().stem().string());
		}
	}

	void reload_styles()
	{
		for (int i = 0; i < _context->GetNumDocuments(); ++i)
			_context->GetDocument(i)->ReloadStyleSheet();
	}

	void process_event(const sf::Event& event)
	{
		if (event.type == sf::Event::Resized)
			_on_window_resized(Rml::Vector2i(event.size.width, event.size.height));
		RmlSFML::InputHandler(_context, event);
	}

	void update(float dt)
	{
		update_textbox(dt);
		dirty_all_variables();
		_context->Update();
	}

	void render()
	{
		_render_interface.BeginFrame();
		_context->Render();
		_render_interface.EndFrame();
	}

	bool should_pause_game() {
		return is_main_menu_visible() || is_textbox_open();
	}

	std::vector<std::string> get_document_names()
	{
		std::vector<std::string> names;
		for (int i = 0; i < _context->GetNumDocuments(); ++i)
			names.push_back(_context->GetDocument(i)->GetId());
		return names;
	}

	void show_document(const std::string& name)
	{
		if (Rml::ElementDocument* doc = _context->GetDocument(name))
			doc->Show();
	}

	void hide_document(const std::string& name)
	{
		if (Rml::ElementDocument* doc = _context->GetDocument(name))
			doc->Hide();
	}
}
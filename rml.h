#pragma once

namespace rml
{
	void startup(sf::RenderWindow* window);
	void cleanup();
	void load_assets();

	void process_event(const sf::Event& ev);
	void update();
	void render(); // Uses OpenGL, so push and pop the OpenGL states before and after.

	// In the below functions, document_filename is not a path,
	// just the filename including the .rml extension.

	void show_document(const std::string& filename);
	void hide_document(const std::string& filename);
}

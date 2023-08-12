#pragma once

namespace rml
{
	void startup(sf::RenderWindow* window);
	void cleanup();

	void process_event(const sf::Event& ev);
	void update();
	void render(); // Uses OpenGL, so push and pop the OpenGL states before and after calling this function.

	bool load_document(const std::string& filename);
}

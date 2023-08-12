#pragma once

namespace rml
{
	void startup_rml(sf::RenderWindow* window);
	void cleanup_rml();
	void update_rml();
	void render_rml();
	bool load_rml(const std::string& filename);
}

#include "html.h"
#include "RmlUi_Backend.h"
#include <RmlUi/Core.h>

namespace html
{
	void startup_html(sf::RenderWindow* window)
	{
		Backend::Initialize(window);
		Rml::SetSystemInterface(Backend::GetSystemInterface());
		Rml::SetRenderInterface(Backend::GetRenderInterface());
		Rml::Initialise();
	}

	void cleanup_html()
	{
		Rml::Shutdown();
		Backend::Shutdown();
	}

	void render_html()
	{
		Backend::BeginFrame();
		Backend::PresentFrame();
	}
}
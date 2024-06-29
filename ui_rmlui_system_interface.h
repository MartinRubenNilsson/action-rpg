#pragma once
#include <RmlUi/Core/SystemInterface.h>

namespace ui
{
	struct RmlUiSystemInterface : Rml::SystemInterface
	{
		double GetElapsedTime() override;
		void SetMouseCursor(const Rml::String& cursor_name) override;
		void SetClipboardText(const Rml::String& text) override;
		void GetClipboardText(Rml::String& text) override;
	};
}
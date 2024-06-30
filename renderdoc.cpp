#include "stdafx.h"
#ifdef DEBUG_RENDERDOC
#include "renderdoc.h"
#include <Windows.h>
#include <shellapi.h> // ShellExecuteW
#include <renderdoc/renderdoc_app.h>
#include "console.h"

namespace renderdoc
{
	RENDERDOC_API_1_6_0* _rdoc_api = nullptr;

	void initialize()
	{
		HMODULE mod = LoadLibraryA("renderdoc.dll");
		if (!mod) {
			console::log_error("Failed to load renderdoc.dll");
			return;
		}
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0, (void**)&_rdoc_api);
		if (ret != 1) {
			console::log_error("Failed to get renderdoc API");
			return;
		}
		RENDERDOC_InputButton capture_key = eRENDERDOC_Key_PrtScrn;
		_rdoc_api->SetCaptureKeys(&capture_key, 1);
		_rdoc_api->MaskOverlayBits(eRENDERDOC_Overlay_None, eRENDERDOC_Overlay_None);
	}

	void open_capture_folder_if_capturing()
	{
		if (!_rdoc_api) return;
		if (!_rdoc_api->IsFrameCapturing()) return;
		const char* path_cstr = _rdoc_api->GetCaptureFilePathTemplate();
		if (!path_cstr) return;
		std::filesystem::path path(path_cstr);
		path = path.parent_path();
		ShellExecuteW(0, L"open", path.c_str(), 0, 0, SW_SHOWNORMAL);
	}
}

#endif // DEBUG_RENDERDOC

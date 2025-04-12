#pragma once
#ifdef _DEBUG_RENDERDOC

namespace renderdoc {
	void initialize();
	void open_capture_folder_if_capturing();
}

#endif // DEBUG_RENDERDOC
#include "audio.h"
#include "fmod_studio.hpp"

#ifdef _DEBUG
#pragma comment(lib, "fmodL_vc.lib")
#pragma comment(lib, "fmodstudioL_vc.lib")
#else
#pragma comment(lib, "fmod_vc.lib")
#pragma comment(lib, "fmodstudio_vc.lib")
#endif

#define AUDIO_MAX_CHANNELS 512

namespace audio
{
	FMOD::Studio::System* _system = nullptr;

	void initialize()
	{
		FMOD::Studio::System::create(&_system);
		_system->initialize(
			AUDIO_MAX_CHANNELS,
			FMOD_STUDIO_INIT_NORMAL,
			FMOD_INIT_NORMAL,
			nullptr);
	}

	void shutdown() {
		_system->release();
	}

	void update() {
		_system->update();
	}
}


#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "AnimRetargeting.h" 

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	//_CrtSetBreakAlloc(8423);
#endif

	AnimRetargeting app = {};

	int32_t err = app.Init();
	if (0 != err)
		return err;

	return app.Run();
}




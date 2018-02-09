#include "AnimRetargeting.h"


int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	AnimRetargeting app = {};

	int32_t err = app.Init();
	if (0 != err)
		return err;

	return app.Run();
}




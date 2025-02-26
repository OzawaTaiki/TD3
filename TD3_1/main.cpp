#include <Essential/SampleFramework.h>

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    Framework* game = new SampleFramework;

    game->Run();

    delete game;

	return 0;
}


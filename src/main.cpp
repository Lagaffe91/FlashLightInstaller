#include <Windows.h>

#include "FlashLightInstaller.hpp"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	FlashlightInstaller installer;

	installer.InitWindow(hInstance, nCmdShow);
	installer.RunInstallation();

	return 0;
}
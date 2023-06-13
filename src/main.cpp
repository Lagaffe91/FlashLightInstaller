#include <Windows.h>

#include <thread>

#include "FlashLightInstaller.hpp"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	FlashlightInstaller installer;

	installer.InitWindow(hInstance, nCmdShow);

	std::thread installThread (&FlashlightInstaller::RunInstallation, &installer);

	installer.RunMessageLoop();

	installThread.join();

	return 0;
}
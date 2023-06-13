#pragma once

#include <Windows.h>

#include <string>

class FlashlightInstaller
{
public:
	FlashlightInstaller() = default;
	~FlashlightInstaller() = default;

private:

	//TODO : 
	//	- Find something more elegant than hardcoded parameters
	typedef struct FlashlightWindow_
	{
		//-------- Main window --------
		HWND hwnd				= NULL;

		LPCWSTR title = L"FlashlightInstaller";

		const int width		= 1000;
		const int height	= 100;

		//-------- Progress bar -------- 
		HWND hwndPB			= NULL;

	}FlashlightWindow;

	typedef struct NorthStarArchive_
	{
		std::string URL		= "";
		std::string path	= "";
	}NorthStarArchive;

	FlashlightWindow mWindow;
	NorthStarArchive mArchive;

public:
	///Optional function : Init installer GUI.
	void InitWindow(HINSTANCE hInstance, int nCmdShow);

	///Run the entire installation process : Parse the url to the latest release, then download and extract the archive.
	void RunInstallation();

	///Run windows message loop for the main window
	void RunMessageLoop();

private:

	/// Use GitHub api to retrive the direct link to the latest release of NorthStar.
	void GetDownloadURL		(NorthStarArchive *pArchive)	const;

	/// Download and write the archive from the url onto the disk.
	void DownloadArchive	(NorthStarArchive *pArchive)	const;

	///Extract the archive to the disk
	void ExtractArchive		(NorthStarArchive *pArchive)	const;
};
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Windows.h>
#include <urlmon.h>
#include <shlwapi.h>

#include <commctrl.h>

#include <string>
#include <thread>
#include <vector>

#include "../resource.h"

#include "miniz/miniz.h"

#include "FlashLightInstaller.hpp"

//TODO : WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
// - Stop the installation process when exiting the app early
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//TODO :
// - Find a more elegant way to init the two windows
void FlashlightInstaller::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	const wchar_t CLASS_NAME[] = L"FlashlightInstaller";

	WNDCLASS wc = { };

	HICON appIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hIcon = appIcon;

	RECT rcClient;

	const int scrollHeight = GetSystemMetrics(SM_CYVSCROLL);

	const int scWidth	= GetSystemMetrics(SM_CXSCREEN);
	const int scHeight	= GetSystemMetrics(SM_CYSCREEN);
	
	RegisterClass(&wc);

	InitCommonControls();

	mWindow.hwnd	= CreateWindowEx(0, CLASS_NAME, mWindow.title, WS_OVERLAPPED | WS_SYSMENU, scWidth/2 - mWindow.width/2, scHeight / 2 - mWindow.height / 2, mWindow.width, mWindow.height, NULL, NULL, hInstance, NULL);

	GetClientRect(mWindow.hwnd, &rcClient);

	mWindow.hwndPB	= CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE, rcClient.left + (rcClient.right / 20), rcClient.top + (rcClient.bottom / 4), rcClient.right - (rcClient.right / 20) * 2, rcClient.bottom/4, mWindow.hwnd, NULL, hInstance, NULL);

	ShowWindow(mWindow.hwnd, nCmdShow);

	SendMessage(mWindow.hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, 4));
	SendMessage(mWindow.hwndPB, PBM_SETSTEP, (WPARAM)1, 0);
}

///Run the entire installation process : Parse the url to the latest release, then download and extract the archive.
void FlashlightInstaller::RunInstallation()
{

	this->GetDownloadURL(&mArchive);
	this->DownloadArchive(&mArchive);
	this->ExtractArchive(&mArchive);
}

void FlashlightInstaller::RunMessageLoop()
{
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

/// Use GitHub api to retrive the direct link to the latest release of NorthStar.
void FlashlightInstaller::GetDownloadURL(NorthStarArchive* pArchive)	const
{
	SetWindowText(mWindow.hwnd, L"FlashlightInstaller : Parsing URL");
	SendMessage(mWindow.hwndPB, PBM_STEPIT, 0, 0);

	IStream* fileStream;

	DWORD byteCount;
	DWORD stringSize;

	STATSTG streamStats;

	std::string apiStr;

	memset(&streamStats, 0, sizeof(STATSTG));

	URLOpenBlockingStream(NULL, L"https://api.github.com/repos/R2Northstar/northstar/releases/latest", &fileStream, 0, NULL);

	fileStream->Stat(&streamStats, STATFLAG_DEFAULT);

	stringSize = streamStats.cbSize.LowPart;

	apiStr.reserve(stringSize);
	apiStr.resize(stringSize);

	fileStream->Read(&apiStr[0], stringSize, &byteCount);
	fileStream->Release();

	///Here we are looking for the "browser_download_url" entry to parse the direct url to the archive.
	///It should look something like : 
	/// -	"browser_download_url": "https://github.com/R2Northstar/Northstar/releases/download/v1.14.2/Northstar.release.v1.14.2.zip"

	constexpr int searchStrSize = 23;

	size_t urlOffset = apiStr.find("\"browser_download_url\":") + searchStrSize + 1;	//+ 1 to remove the extra " character
	size_t urlStrSize = (apiStr.find(".zip\"", urlOffset) - urlOffset) + 4;				// 4 -> ".zip"

	pArchive->URL = apiStr.substr(urlOffset, urlStrSize);
}

/// Download and write the zip file from pFileURL url onto the disk.
void FlashlightInstaller::DownloadArchive(NorthStarArchive* pArchive)	const
{
	SetWindowText(mWindow.hwnd, L"FlashlightInstaller : Downloading Latest Northstar Release");
	SendMessage(mWindow.hwndPB, PBM_STEPIT, 0, 0);

	std::wstring wURL;

	IStream* fileStream;
	HRESULT downloadResult;
	STATSTG streamStats;

	DWORD byteCount;
	DWORD fileSize;

	HANDLE zipHandle;

	std::vector<char> fileBuffer;

	memset(&streamStats, 0, sizeof(STATSTG));

	//Download file

	wURL = std::wstring(pArchive->URL.begin(), pArchive->URL.end());

	downloadResult = URLOpenBlockingStream(NULL, wURL.c_str(), &fileStream, 0, NULL);

	if (!SUCCEEDED(downloadResult))
		return;

	PathStripPath(&wURL[0]);
	pArchive->path = std::string(wURL.begin(), wURL.end());

	{
		fileStream->Stat(&streamStats, STATFLAG_DEFAULT);

		fileSize = streamStats.cbSize.LowPart;

		fileBuffer.reserve(fileSize);

		fileStream->Read(fileBuffer.data(), fileSize, &byteCount);
		fileStream->Release();
	}

	zipHandle = CreateFileA(&pArchive->path[0], GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	WriteFile(zipHandle, fileBuffer.data(), fileSize, &byteCount, NULL);

	CloseHandle(zipHandle);
}

///Extract the archive to the disk
void FlashlightInstaller::ExtractArchive(NorthStarArchive* pArchive)	const
{
	SetWindowText(mWindow.hwnd, L"FlashlightInstaller : Extracting archive");
	SendMessage(mWindow.hwndPB, PBM_STEPIT, 0, 0);

	mz_zip_archive archive;

	memset(&archive, 0, sizeof(mz_zip_archive));

	mz_zip_reader_init_file(&archive, pArchive->path.c_str(), 0);

	const int fileCount = mz_zip_reader_get_num_files(&archive);

	if (fileCount <= 0)
		return;

	SendMessage(mWindow.hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, fileCount));

	mz_zip_archive_file_stat stats;

	for (int i = 0; i < fileCount; i++)
	{
		SendMessage(mWindow.hwndPB, PBM_STEPIT, 0, 0);

		memset(&stats, 0, sizeof(mz_zip_archive_file_stat));
		mz_zip_reader_file_stat(&archive, i, &stats);

		const bool isDirectory = mz_zip_reader_is_file_a_directory(&archive, i);

		if (isDirectory)
			CreateDirectoryA(stats.m_filename, NULL);
		else
			mz_zip_reader_extract_to_file(&archive, i, stats.m_filename, 0);
	}

	mz_zip_reader_end(&archive);

	SendMessage(mWindow.hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, 1));
	SendMessage(mWindow.hwndPB, PBM_SETSTEP, 1, 0);
	SetWindowText(mWindow.hwnd, L"FlashlightInstaller : Installation Completed");
}
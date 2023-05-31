#include <iostream>

#include <Windows.h>

#include <urlmon.h>
#include <sstream>

#include <vector>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	IStream* fileStream;
	HRESULT downloadResult = URLOpenBlockingStream(NULL, L"https://github.com/R2Northstar/Northstar/releases/latest/download/Northstar.release.v1.14.2.zip", &fileStream, 0, NULL);

	if (!SUCCEEDED(downloadResult))
		return 1;


	STATSTG streamStats{};

	fileStream->Stat(&streamStats, STATFLAG_DEFAULT);

	DWORD byteCount;								//Out variable for windows, will not be used
	DWORD fileSize = streamStats.cbSize.LowPart;	//Will never have to use the high part.

	std::vector<char> fileBuffer;
	fileBuffer.reserve(fileSize);

	fileStream->Read(fileBuffer.data(), fileSize, &byteCount);

	fileStream->Release();

	HANDLE zipHandle = CreateFileA("./NorthStar.zip", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	WriteFile(zipHandle, fileBuffer.data(), fileSize, &byteCount, NULL);

	CloseHandle(zipHandle);

	return 0;
}

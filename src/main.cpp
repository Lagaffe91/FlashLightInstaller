#include <Windows.h>
#include <shlwapi.h>
#include <urlmon.h>

#include <sstream>
#include <vector>

/// <summary>
/// Download and parse github html to retrive the filename of the latest NorthStar release.
/// </summary>
/// <returns></returns>
LPCWSTR GetDownloadURL()
{
	IStream* fileStream;
	HRESULT downloadResult = URLOpenBlockingStream(NULL, L"https://github.com/R2Northstar/Northstar/releases/latest/", &fileStream, 0, NULL);

	STATSTG streamStats{};

	fileStream->Stat(&streamStats, STATFLAG_DEFAULT);

	DWORD stringSize = streamStats.cbSize.LowPart;

	std::string htmlStr;
	htmlStr.reserve(stringSize);

	DWORD byteCount;

	fileStream->Read(&htmlStr[0], stringSize, &byteCount);
	fileStream->Release();

	constexpr int searchStringSize = 41; //size of -> "/R2Northstar/Northstar/releases/download/"
	const int linkOffset = htmlStr.find("/R2Northstar/Northstar/releases/download/", 0);

	//htmlStr.s

	return L"";
}

/// <summary>
/// Download and write the file at url pFileURL onto the disk.
/// </summary>
/// <param name="pFileURL"> : Url of the desired file.</param>
void DownloadFile(LPCWSTR pFileURL)
{
	IStream* fileStream;
	HRESULT downloadResult = URLOpenBlockingStream(NULL, pFileURL, &fileStream, 0, NULL);

	if (!SUCCEEDED(downloadResult))
		return;

	STATSTG streamStats{};

	fileStream->Stat(&streamStats, STATFLAG_DEFAULT);

	DWORD byteCount;								//Out variable for windows, will not be used
	DWORD fileSize = streamStats.cbSize.LowPart;	//Will never have to use the high part.

	std::vector<char> fileBuffer;
	fileBuffer.reserve(fileSize);

	fileStream->Read(fileBuffer.data(), fileSize, &byteCount);

	fileStream->Release();

	//Write file
	std::wstring fileName(pFileURL);
	PathStripPath(&fileName[0]);

	HANDLE zipHandle = CreateFileA("./Northstar.release.v1.14.2.zip", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	WriteFile(zipHandle, fileBuffer.data(), fileSize, &byteCount, NULL);

	CloseHandle(zipHandle);

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	DownloadFile(L"https://github.com/R2Northstar/Northstar/releases/latest/download/Northstar.release.v1.14.2.zip");

	return 0;
}

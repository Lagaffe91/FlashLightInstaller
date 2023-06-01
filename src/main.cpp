#include <Windows.h>
#include <shlwapi.h>
#include <urlmon.h>

#include <sstream>
#include <vector>

/// <summary>
/// Download and parse github html to retrive the filename of the latest NorthStar release.
/// </summary>
/// <returns>LPCWSTR : Direct link to the latest release of NorthStar.</returns>
std::string GetDownloadURL()
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

	//We are looking for this kind of string "R2Northstar/Northstar/releases/download/v1.14.2/Northstar.release.v1.14.2.zip" to retrive current the file name

	int fileExtentionOffset = htmlStr.find(".zip"); //There is only one occurence of ".zip" in the html

	std::string fileName = "";

	return "https://github.com/R2Northstar/Northstar/releases/latest/download/" + fileName;
}

/// <summary>
/// Download and write the file at url pFileURL onto the disk.
/// </summary>
/// <param name="pFileURL"> : Url of the desired file.</param>
void DownloadFile(const std::string& pFileURL)
{
	std::wstring wURL(pFileURL.begin(), pFileURL.end());

	IStream* fileStream;
	HRESULT downloadResult = URLOpenBlockingStream(NULL, wURL.c_str(), &fileStream, 0, NULL);

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

	PathStripPath(&wURL[0]);

	HANDLE zipHandle = CreateFileA("./Northstar.release.v1.14.2.zip", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	WriteFile(zipHandle, fileBuffer.data(), fileSize, &byteCount, NULL);

	CloseHandle(zipHandle);

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	std::string downloadURL = GetDownloadURL();
	
	DownloadFile(downloadURL);

	return 0;
}

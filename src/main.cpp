#include <Windows.h>
#include <urlmon.h>

#include <sstream>
#include <vector>

/// <summary>
/// Use GitHub api to retrive the direct link to the latest release of NorthStar.
/// </summary>
/// <returns>LPCWSTR : Direct link to the latest release of NorthStar.</returns>
std::string GetDownloadURL()
{
	IStream* fileStream;
	HRESULT downloadResult = URLOpenBlockingStream(NULL, L"https://api.github.com/repos/R2Northstar/northstar/releases/latest", &fileStream, 0, NULL);

	STATSTG streamStats{};

	fileStream->Stat(&streamStats, STATFLAG_DEFAULT);

	DWORD stringSize = streamStats.cbSize.LowPart;

	std::string resultStr;

	resultStr.reserve(stringSize);
	resultStr.resize(stringSize);

	DWORD byteCount;

	fileStream->Read(&resultStr[0], stringSize, &byteCount);
	fileStream->Release();

	constexpr int searchStrSize = 23;
	int urlOffset	= resultStr.find("\"browser_download_url\":") + searchStrSize + 1; //+ 1 to remove the extra " character
	int urlStrSize  = (resultStr.find(".zip\"", urlOffset) - urlOffset) + 4 ; // 4 -> ".zip"
	
	return resultStr.substr(urlOffset, urlStrSize);
}

/// <summary>
/// Download and write the file from pFileURL url onto the disk.
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

	DWORD byteCount;
	DWORD fileSize = streamStats.cbSize.LowPart;

	std::vector<char> fileBuffer;
	fileBuffer.reserve(fileSize);

	fileStream->Read(fileBuffer.data(), fileSize, &byteCount);

	fileStream->Release();
	
	//TODO :
	//Get filename with :
	//	PathStripPath(&wURL[0]);

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

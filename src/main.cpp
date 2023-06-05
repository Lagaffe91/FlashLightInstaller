#include <Windows.h>
#include <urlmon.h>
#include <shlwapi.h>

#include <string>

#include <vector>

#include "miniz/miniz.h"

/// Use GitHub api to retrive the direct link to the latest release of NorthStar.
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
	size_t urlOffset = resultStr.find("\"browser_download_url\":") + searchStrSize + 1; //+ 1 to remove the extra " character
	size_t urlStrSize = (resultStr.find(".zip\"", urlOffset) - urlOffset) + 4; // 4 -> ".zip"

	return resultStr.substr(urlOffset, urlStrSize);
}

///Decompress a zip archive to the working directory
//TODO : 
// - Better memory management
void DecompressZIPFolder(mz_zip_archive* pArchive)
{
	if (!pArchive) return;

	const int fileCount = mz_zip_reader_get_num_files(pArchive);
	
	if (fileCount <= 0) return;

	for (int i = 0; i < fileCount; i++)
	{
		mz_zip_archive_file_stat stats{};
		mz_zip_reader_file_stat(pArchive, i, &stats);

		const bool isDirectory = mz_zip_reader_is_file_a_directory(pArchive, i);

		if (isDirectory)
			CreateDirectoryA(stats.m_filename, NULL);
		else
			mz_zip_reader_extract_to_file(pArchive, i, stats.m_filename, 0);
	}
}

/// Download and write the zip file from pFileURL url onto the disk.
void DownloadAndExtractZip(const std::string& pFileURL)
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

	PathStripPath(&wURL[0]);

	std::string fileName(wURL.begin(), wURL.end());

	HANDLE zipHandle = CreateFileA(&fileName[0], GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	WriteFile(zipHandle, fileBuffer.data(), fileSize, &byteCount, NULL);

	CloseHandle(zipHandle);

	mz_zip_archive archive{};

	mz_zip_reader_init_file(&archive, fileName.c_str(), 0);

	DecompressZIPFolder(&archive);

	mz_zip_reader_end(&archive);

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	std::string downloadURL = GetDownloadURL();

	DownloadAndExtractZip(downloadURL);

	return 0;
}

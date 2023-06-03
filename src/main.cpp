#include <Windows.h>
#include <urlmon.h>
#include <shlwapi.h>

#include <sstream>
#include <vector>

#include "miniz/miniz.h"

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

//TODO : 
// - Better memory managements
// - Proprely extract the folders inside the archive
void DecompressZIPFolder(mz_zip_archive* pArchive, int pFileCount)
{
	if (pFileCount <= 0 || !pArchive) return;

	for (int i = 0; i < pFileCount; i++)
	{

		//if (mz_zip_reader_is_file_a_directory(pArchive, i)) 
		//{
			//Folder

			//mz_extract
		//}
		//else
		{
			//File

			mz_zip_archive_file_stat stats{};

			mz_zip_reader_file_stat(pArchive, i, &stats);

			void* fileData = mz_zip_reader_extract_file_to_heap(pArchive, stats.m_filename, &stats.m_uncomp_size, 0);


			HANDLE fileHandle = CreateFileA(&stats.m_filename[0], GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			DWORD byteCount;

			WriteFile(fileHandle, fileData, stats.m_uncomp_size, &byteCount, NULL);

			CloseHandle(fileHandle);

			delete fileData;
		}
	}
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
	
	PathStripPath(&wURL[0]);

	std::string fileName(wURL.begin(), wURL.end());

	HANDLE zipHandle = CreateFileA(&fileName[0], GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	WriteFile(zipHandle, fileBuffer.data(), fileSize, &byteCount, NULL);

	CloseHandle(zipHandle);

	mz_zip_archive archive{};

	mz_zip_reader_init_file(&archive, fileName.c_str(), 0);

	const int fileCount = mz_zip_reader_get_num_files(&archive);

	DecompressZIPFolder(&archive, fileCount);

	mz_zip_reader_end(&archive);

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	std::string downloadURL = GetDownloadURL();
	
	DownloadFile(downloadURL);

	return 0;
}

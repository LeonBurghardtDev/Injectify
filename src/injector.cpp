/**
 * @file injector.cpp
 * @brief This file contains the code for injecting a DLL into a selected process.
 */

#include <windows.h>
#include <string>
#include <libloaderapi.h> // LoadLibrary
#include <vector>
#include <shlwapi.h> // PathFileExists
#include <psapi.h> // GetModuleFileNameEx
#include <tlhelp32.h> // CreateToolhelp32Snapshot
#include <filesystem>

#include "globals.h"

using namespace std;

/**
 * @brief Displays an error message and sets the global variables accordingly.
 * @param message The error message to be displayed.
 */
void error(const char* message) {
	globals::error_msg = message;
	globals::isDllInjected = false;
}

/**
 * @brief Checks whether a file exists in the file system.
 * @param file The file path to be checked.
 * @return A boolean value indicating whether the file exists.
 */
bool file_exists(string file) {
	struct stat buffer;
	return (stat(file.c_str(), &buffer) == 0);
}

/**
 * @brief Injects a DLL into the selected process.
 */
void inject_dll() {

	/* Manage process */

	DWORD proc_id = globals::selectedProcessID;

	if (proc_id == NULL) {
		error("Could not get process id");
	}

	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, NULL, proc_id);
	if (!process) {
		error("Could not open process");
	}


	/* Manage DLLs */

	for (int i = 0; i < globals::dll_paths.size() - 1; i++) {

		char dll_path[MAX_PATH];
		strcpy_s(dll_path, globals::dll_paths[i].c_str());

		LPVOID allocated_memory = VirtualAllocEx(process, nullptr, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!allocated_memory) {
			error("Could not allocate memory");
			return;
		}

		if (!WriteProcessMemory(process, allocated_memory, dll_path, MAX_PATH, nullptr)) {
			error("Could not write process memory");
			return;
		}

		LPVOID loadLibraryAddr = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
		if (!loadLibraryAddr) {
			error("Could not get address of LoadLibraryA");
			return;
		}

		HANDLE thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, allocated_memory, 0, NULL);

		if (!thread) {
			error("Could not create remote thread");
			return;
		}
		WaitForSingleObject(thread, INFINITE);

		CloseHandle(thread);
		VirtualFreeEx(process, allocated_memory, NULL, MEM_RELEASE);
		CloseHandle(process);

		globals::lastInjected = std::filesystem::path(globals::dll_paths[i]).filename().string();
		globals::isDllInjected = true;
	}


}

/**
 * @brief Gets the process IDs of all running processes.
 * @return A vector of process IDs.
 */
std::vector<DWORD> GetAllProcessIds() {
	std::vector<DWORD> processIds;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		return processIds;
	}
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	if (Process32First(hSnapshot, &pe32)) {
		do {
			processIds.push_back(pe32.th32ProcessID);
		} while (Process32Next(hSnapshot, &pe32));
	}
	CloseHandle(hSnapshot);
	return processIds;
}

/**
 * @brief Gets the name of a process.
 * @param processId The process ID of the process.
 * @return The name of the process.
 */
std::string GetProcessName(DWORD processId) {
	std::string processName;
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
	if (hProcess != NULL) {
		HMODULE hModule;
		DWORD cbNeeded;
		if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded)) {
			TCHAR szProcessName[MAX_PATH];
			if (GetModuleFileNameEx(hProcess, hModule, szProcessName, sizeof(szProcessName) / sizeof(TCHAR))) {
				TCHAR* szFileName = PathFindFileNameA(szProcessName);
				processName = szFileName;
			}
			else {
				DWORD lastError = GetLastError();
				LPVOID lpMsgBuf;
				DWORD bufLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
				if (bufLen > 0) {
					processName = "Error retrieving process name: " + std::string((LPCTSTR)lpMsgBuf);
					LocalFree(lpMsgBuf);
				}
				else {
					processName = "Error retrieving process name. Error code: " + std::to_string(lastError);
				}
			}
		}
		else {
			DWORD lastError = GetLastError();
			processName = "Error enumerating process modules. Error code: " + std::to_string(lastError);
		}
		CloseHandle(hProcess);
	}
	else {
		DWORD lastError = GetLastError();
		processName = "Error opening process. Error code: " + std::to_string(lastError);
	}
	return processName;
}


/**
 * Get the process ID for a given process name.
 * @param processName The name of the process to search for.
 * @return The process ID if found, 0 otherwise.
 */
DWORD GetProcessIdByName(const std::string& processName)
{
	PROCESSENTRY32 pe32;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		return 0; // Unable to create snapshot of running processes.
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &pe32))
	{
		do
		{
			std::string currProcessName = pe32.szExeFile;
			if (currProcessName == processName)
			{
				CloseHandle(hSnapshot);
				return pe32.th32ProcessID; // Found the process, return its ID.
			}
		} while (Process32Next(hSnapshot, &pe32));
	}

	CloseHandle(hSnapshot);
	return 0;  // Process not found.
}


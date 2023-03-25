/**

@file injector.h
@brief Header file for injecting a DLL into a process.
*/
#ifndef INJECTOR_H
#define INJECTOR_H

#include <vector>
#include <string>
#include <shlwapi.h>

/**

@brief Injects a DLL into a target process.
This function injects the specified DLL into a target process by creating a remote thread
and calling LoadLibraryA in the target process.
@throw std::runtime_error if the injection fails.
*/
void inject_dll();
/**

@brief Gets the name of the process with the specified process ID.
This function retrieves the name of the process with the specified process ID using
the Windows API functions OpenProcess, EnumProcessModules, and GetModuleFileNameEx.
@param processId The ID of the target process.
@return The name of the process with the specified process ID.
@throw std::runtime_error if the process name cannot be retrieved.
*/
std::string GetProcessName(DWORD processId);
/**

@brief Gets the IDs of all currently running processes.
This function retrieves the IDs of all currently running processes using the
Windows API function CreateToolhelp32Snapshot and Process32Next.
@return A vector containing the IDs of all currently running processes.
@throw std::runtime_error if the process IDs cannot be retrieved.
*/
std::vector<DWORD> GetAllProcessIds();
/**

@brief Gets the ID of the process with the specified name.
This function retrieves the ID of the process with the specified name by iterating
through all currently running processes and comparing their names to the specified name.
@param processName The name of the target process.
@return The ID of the process with the specified name, or 0 if no matching process is found.
@throw std::runtime_error if the process IDs cannot be retrieved.
*/
DWORD GetProcessIdByName(const std::string& processName);
#endif // INJECTOR_H
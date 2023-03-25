/**

@file globals.h
@brief Defines global variables used throughout the application.
*/

#pragma once
#include <vector>
#include <string>

namespace globals {
	/**
	* @brief Index of the currently selected process in the process list.
	*/
	inline int selectedProcessIndex = -1;
	/**
 * @brief Process ID of the currently selected process in the process list.
 */
	inline int selectedProcessID = 0;

	/**
	 * @brief Name of the currently selected process in the process list.
	 */
	inline std::string selected_process_name;

	/**
	 * @brief Whether a DLL file has been selected for injection.
	 */
	inline bool isFileSelected = false;

	/**
	 * @brief Paths of the DLL files to be injected.
	 */
	inline std::vector<std::string> dll_paths;

	/**
	 * @brief Name of the last injected DLL file.
	 */
	inline std::string lastInjected;

	/**
	 * @brief Whether a DLL file has been successfully injected.
	 */
	inline bool isDllInjected = false;

	/**
	 * @brief Error message generated during DLL injection process.
	 */
	inline std::string error_msg;
}
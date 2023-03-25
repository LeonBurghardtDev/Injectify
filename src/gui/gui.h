/**

@file gui.h
@brief Defines functions and variables used for the GUI.
*/

#pragma once
#include <d3d9.h>

namespace gui
{
	// constant window size
	constexpr int WIDTH = 350;
	constexpr int HEIGHT = 370;

	// variable to keep track of whether the GUI is running
	inline bool isRunning = true;

	// variable to keep track of whether the GUI menu should be displayed
	inline bool showMenu = true;

	// WinAPI window variables
	inline HWND window = nullptr; // handle to the window
	inline WNDCLASSEX windowClass = { }; // structure for registering the window class

	// points for window movement
	inline POINTS position = { }; // structure that stores two short values

	// Direct3D state variables
	inline PDIRECT3D9 d3d = nullptr; // pointer to Direct3D9 object
	inline LPDIRECT3DDEVICE9 device = nullptr; // pointer to Direct3D9 device object
	inline D3DPRESENT_PARAMETERS presentParameters = { }; // structure that contains the presentation parameters for Direct3D9 device

	// function to create the WinAPI window
	void CreateHWindow(const char* windowName) noexcept;

	// function to destroy the WinAPI window
	void DestroyHWindow() noexcept;

	// function to create the Direct3D9 device
	bool CreateDevice() noexcept;

	// function to reset the Direct3D9 device
	void ResetDevice() noexcept;

	// function to destroy the Direct3D9 device
	void DestroyDevice() noexcept;

	// function to create ImGui context
	void CreateImGui() noexcept;

	// function to destroy ImGui context
	void DestroyImGui() noexcept;

	// function to start rendering
	void BeginRender() noexcept;

	// function to finish rendering
	void EndRender() noexcept;

	// function to render ImGui content
	void Render() noexcept;
}

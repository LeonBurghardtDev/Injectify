/**
 * @file gui.cpp
 * @brief Renders the GUI using ImGui
 */

#include "gui.h"
#include "../globals.h"
#include "../injector.h"
#include "../../resource.h"

#include <algorithm>

#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_dx9.h"
#include "../../imgui/imgui_impl_win32.h"
#include <string>
#include <Windows.h>
#include <dwmapi.h> 
#include <filesystem>


/**
* @brief The main window procedure that handles messages for the window.
* @param window The handle to the window.
* @param message The message code.
* @param wideParameter The WPARAM value of the message.
* @param longParameter The LPARAM value of the message.
* @return The result of the message processing. If the message was handled, it returns true, otherwise it returns false.
*/

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
);

/**
* @brief The main window procedure that handles messages for the window.
* @param window The handle to the window.
* @param message The message code.
* @param wideParameter The WPARAM value of the message.
* @param longParameter The LPARAM value of the message.
* @return The result of the message processing. If the message was handled, it returns true, otherwise it returns false.
 */

LRESULT CALLBACK WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter)
{
	// Pass window messages to ImGui
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
	case WM_SIZE: {
		// Reset device parameters when window is resized
		if (gui::device && wideParameter != SIZE_MINIMIZED)
		{
			gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
			gui::presentParameters.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	} return 0;

	case WM_SYSCOMMAND: {
		// Disable ALT application menu
		if ((wideParameter & 0xfff0) == SC_KEYMENU)
			return 0;
	} break;

	case WM_DESTROY: {
		// Post quit message on window destroy
		PostQuitMessage(0);
	} return 0;

	case WM_LBUTTONDOWN: {
		// Set click points for window movement
		gui::position = MAKEPOINTS(longParameter);
	} return 0;

	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{ };

			// Get window rectangle
			GetWindowRect(gui::window, &rect);

			// Move window
			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			// Set new window position if within bounds
			if (gui::position.x >= 0 &&
				gui::position.x <= gui::WIDTH &&
				gui::position.y >= 0 && gui::position.y <= 19)
				SetWindowPos(
					gui::window,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}
	} return 0;
	}

	// Default message handling
	return DefWindowProc(window, message, wideParameter, longParameter);
}


 /**
*
* @brief Create the main window for the GUI
* @param windowName The name of the window to be created
* @remarks This function is noexcept and is safe to call from any thread
* @return void
*/
void gui::CreateHWindow(const char* windowName) noexcept
{
	if (!windowName)
		return;
	
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(0);
	windowClass.hIcon = LoadIcon(windowClass.hInstance, MAKEINTRESOURCE(IDI_ICON1)); // load icon
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = "class001";
	windowClass.hIconSm = 0;

	RegisterClassEx(&windowClass);

	window = CreateWindowEx(
		0,
		"class001",
		windowName,
		WS_POPUP,
		100,
		100,
		WIDTH,
		HEIGHT,
		0,
		0,
		windowClass.hInstance,
		0
	);
	if (window)
	{
		// Set the window to be always on top
		ShowWindow(window, SW_SHOWDEFAULT);
		UpdateWindow(window);
	}

	
}
/** 
* @brief Destroys the window and unregisters the window class
* @remarks This function is noexcept and is safe to call from any thread
*/
void gui::DestroyHWindow() noexcept
{
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

/**
* @brief Creates the Direct3D device for rendering.
* @return True if device was successfully created, false otherwise.
* @remarks This function is noexcept and is safe to call from any thread
*/
bool gui::CreateDevice() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;
}
/**
* @brief Reset the Direct3D device and recreate ImGui device objects.
* @return void
* @remarks This function is noexcept and is safe to call from any thread
*/
void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

/**
* @brief Destroys the Direct3D device and releases the COM objects.
* @remark Call this function when you are finished with the device to clean up resources.
*/
void gui::DestroyDevice() noexcept
{
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}
/**
* @brief Initializes ImGui context and sets default red style.
* @remarks This function initializes ImGui context, sets the default style to red,
*  and initializes Win32 and DirectX 9 implementations for ImGui.
* @note This function should be called after creating the main window and DirectX 9 device.
*/
void gui::CreateImGui() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsRed();



	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

/**
* @brief Shuts down ImGui and Win32/DirectX 9 implementations for ImGui.
* @remarks This function should be called before shutting down the application to release
*  resources used by ImGui and its implementations.
*/
void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

/**
* @brief Begins the Dear ImGui frame and processes window messages.
* @remarks This function should be called once per frame before rendering ImGui windows.
*  It processes window messages and sets up the ImGui frame for rendering.
* @note This function should be called before rendering any ImGui windows.
*/
void gui::BeginRender() noexcept
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			isRunning = !isRunning;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}
/**
* @brief End the rendering frame and present the final image on the screen
* @remark Call this function when you are finished with the device to clean up resources.
*/
void gui::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}

/**
* @brief A function that compares two strings in a case-insensitive manner.
* @param str1 The first string to be compared.
* @param str2 The second string to be compared.
* @return A boolean value indicating whether the first string is less than the second string in lexicographical order.
*/
bool compareStringsIgnoreCase(const std::string& str1, const std::string& str2) {
	std::string str1Lower = str1;
	std::transform(str1Lower.begin(), str1Lower.end(), str1Lower.begin(), ::tolower);
	std::string str2Lower = str2;
	std::transform(str2Lower.begin(), str2Lower.end(), str2Lower.begin(), ::tolower);
	return str1Lower < str2Lower;
}

/**
* @brief Opens a file dialog and allows the user to select a DLL file.
* @param filePath The selected file's path will be stored in this variable.
* @remarks Uses the Windows API function GetOpenFileNameA to display the dialog.
*/
void SelectFile(std::string& filePath) {
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "DLL Files (*.dll)\0*.dll\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn) == TRUE) {
		filePath = ofn.lpstrFile;
	}
}

/**
* @brief Renders the ImGui Window and Widgets
* @remarks This function is noexcept and is safe to call from any thread
*/
void gui::Render() noexcept
{
	// Initialize ImGui Window

	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
	ImGui::Begin(
		"Injectify - Simple DLL Injector by tr3x",
		&isRunning,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoMove 
	);
	
	
	ImGui::Spacing();
	

	/* Select Process */

	std::vector<DWORD> processIds = GetAllProcessIds();
	std::vector<std::string> processNames;
	for (DWORD processId : processIds) {
		std::string processName = GetProcessName(processId);
		if (processName.find("Error") == 0) {
			continue;
		}
		processNames.push_back(processName);
	}

	// sort
	std::sort(processNames.begin(), processNames.end(), compareStringsIgnoreCase);

	// get all processes in dropdown
	if (ImGui::BeginCombo("", globals::selectedProcessIndex >= 0 ? processNames[globals::selectedProcessIndex].c_str() : "Select a process")) {
		for (int i = 0; i < processNames.size(); i++) {
			bool isSelected = globals::selectedProcessIndex == i;
			if (ImGui::Selectable(processNames[i].c_str(), isSelected)) {
				globals::selectedProcessIndex = i;
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}	

	if (globals::selectedProcessIndex >= 0 && globals::selectedProcessIndex < processNames.size()) {
		globals::selected_process_name = processNames[globals::selectedProcessIndex];
		globals::selectedProcessID = GetProcessIdByName(globals::selected_process_name);
	}
	
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Change the separator color to gray
	ImGui::Separator();

	/* Select DLLs*/

	std::string filePath;

	if (ImGui::Button("Add DLL")){
		SelectFile(filePath);
		globals::isFileSelected = true;
	}
	if (globals::dll_paths.size() > 0) {
		if (ImGui::Button("Clear DLLs")) {
			for (int i = 0; i < globals::dll_paths.size(); i++)
			{
				globals::dll_paths[i].erase();	
			}
		}
	}
	
	if (globals::isFileSelected) {
		
		bool isFileAlreadySelected = false;
		for (int i = 0; i < globals::dll_paths.size(); i++) {
			if (globals::dll_paths[i] == filePath) {
				isFileAlreadySelected = true;
				break;
			}
		}
		
		if (!isFileAlreadySelected) {
			globals::dll_paths.push_back(filePath);
		}
	}

	
	/* Display selected files */
	ImGui::Text("Selected Files:");
	ImGui::BeginChild("Selected Files Child", ImVec2(0, 100), true);
	for (int i = 0; i < globals::dll_paths.size(); i++) {
		std::string fileName = std::filesystem::path(globals::dll_paths[i]).filename().string();
		ImGui::Text(fileName.c_str());
	}
	ImGui::EndChild();


	ImGui::Separator();

	/* Inject */

	if (globals::selectedProcessIndex >= 0 && globals::isFileSelected) {
		if (ImGui::Button("Inject")) {
			inject_dll();
		}
	}

	/* messages */
	
	if (globals::isDllInjected) {
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "DLL (%s) injected \nsuccessfully to '%s' ",globals::lastInjected.c_str(), processNames[globals::selectedProcessIndex].c_str());
	}
	else if (!globals::error_msg.empty()) {
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "DLL injection failed:\n%s", globals::error_msg.c_str());
	}


	ImGui::End();
}


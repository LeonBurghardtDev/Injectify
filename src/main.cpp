/**
 * @file main.cpp
 * @brief A DLL injector GUI application that creates an interface using ImGui to inject a DLL into a target process.
 */

#include "gui/gui.h" // for GUI functions
#include <thread>
#include <string>
#include "globals.h" // for isRunning

 /**
  * @brief The entry point of the application.
  *
  * @param hInstance A handle to the current instance of the application.
  * @param hPrevInstance A handle to the previous instance of the application. Unused in modern Windows.
  * @param lpCmdLine The command line parameters passed to the application.
  * @param nCmdShow The window display state.
  * @return The exit code of the application.
  */
INT APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    // Initialize GUI components
    gui::CreateHWindow("Injectify");
    gui::CreateDevice();
    gui::CreateImGui();

    // Main loop
    while (gui::isRunning)
    {
        gui::BeginRender();
        gui::Render();
        gui::EndRender();

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    // Clean up GUI components
    gui::DestroyImGui();
    gui::DestroyDevice();
    gui::DestroyHWindow();

    return EXIT_SUCCESS;
}

#include "main.hpp"

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

	auto resolution = ui::SDLWindow::getCoolResolution(3, 4);
	ui::main_window = new ui::SDLWindow("sequrunner 0.0.1", resolution.first, resolution.second);

	ImGui::CreateContext();

	ui::init();

	ImGui_ImplSDL2_InitForSDLRenderer(*ui::main_window, *ui::main_window);
	ImGui_ImplSDLRenderer_Init(*ui::main_window);

	bool done = false;
	while (!done)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
			done |= ui::event(event);

		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();

		ImGui::NewFrame();

		ui::loop();

		ImGui::Render();

		ImVec2 scale = ImGui::GetIO().DisplayFramebufferScale;
		SDL_RenderSetScale(*ui::main_window, scale.x, scale.y);

		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(*ui::main_window);
	}

	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();

	ImGui::DestroyContext();

	delete ui::main_window;
	SDL_Quit();

	return 0;
}
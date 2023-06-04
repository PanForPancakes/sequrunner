#pragma once

#include <functional>
#include <typeinfo>
#include <format>

#include <SDL.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer.h>
#include <imgui-knobs.h>

#include "engine/utility.hpp"
#include "engine/playback.hpp"
#include "engine/media.hpp"

CMRC_DECLARE(assets);

namespace ImGui
{
	using namespace ImGui;
	using namespace ImGuiKnobs;
}

namespace ui
{
	using namespace engine::playback;
	using namespace engine::utility;

	class SDLWindow
	{
		SDL_Window* window;
		SDL_Renderer* renderer;

		std::map<std::string, SDL_Texture*> textures;

		static constexpr int32_t SWPU = SDL_WINDOWPOS_UNDEFINED;
		static constexpr int32_t DSWF = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
		static constexpr int32_t DSRF = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

	public:
		static std::pair<uint32_t, uint32_t> getCoolResolution(uint32_t numerator, uint32_t denumerator, uint32_t display = 0)
		{
			SDL_Rect display_bounds;
			SDL_GetDisplayUsableBounds(display, &display_bounds);

			return std::make_pair(display_bounds.w * numerator / denumerator, display_bounds.h * numerator / denumerator);
		}

		SDLWindow(std::string title, int32_t w, int32_t h, uint32_t window_flags = DSWF) : SDLWindow(title, w, h, window_flags, SWPU, SWPU) {}

		SDLWindow(std::string title, int32_t w, int32_t h, uint32_t window_flags, int32_t x, int32_t y, uint32_t renderer_flags = DSRF, int32_t renderer_driver = -1)
		{
			window = SDL_CreateWindow(title.c_str(), x, y, w, h, window_flags);
			renderer = SDL_CreateRenderer(window, renderer_driver, renderer_flags);
		}

		~SDLWindow()
		{
			for (auto& element : textures)
				SDL_DestroyTexture(element.second);

			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
		}

		operator SDL_Window*()
		{
			return window;
		}

		operator SDL_Renderer*()
		{
			return renderer;
		}

		SDL_Window* getWindow()
		{
			return window;
		}

		SDL_Renderer* getRenderer()
		{
			return renderer;
		}

		std::set<std::string> getAllTextureIDs()
		{
			std::set<std::string> buffer;

			for (auto& element : textures)
				buffer.insert(element.first);

			return buffer;
		}

		bool doesTextureExist(std::string id)
		{
			return textures.contains(id);
		}

		bool createTexture(std::string id, SDL_Surface* surface)
		{
			if (doesTextureExist(id))
				return false;

			textures[id] = SDL_CreateTextureFromSurface(renderer, surface);

			return true;
		}

		SDL_Texture* getTexture(std::string id)
		{
			return textures[id];
		}

		SDL_Texture* operator[](const std::string& id)
		{
			return textures[id];
		}
	};

	struct UIDataCueProvider : AbstractCueVisitor
	{
		std::string visible_name;
		std::string texture_key;

		std::function<SharedCue()> generator;

		void accept(MemoCue& cue)
		{
			visible_name = "Memo Cue";
			texture_key = "memocue";
		}

		void accept(DisarmCue& cue)
		{
			visible_name = "Disarm Cue";
			texture_key = "disarmcue";
		}
	};

	extern CMRCFilesystem assets;

	extern SDLWindow* main_window;

	void loop();
	bool event(SDL_Event event);
	void init();
}
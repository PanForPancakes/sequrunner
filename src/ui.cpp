#include "ui.hpp"

namespace ui
{
	CMRCFilesystem assets(cmrc::assets::get_filesystem());

	SDLWindow* main_window;

	auto project = Project();

	struct
	{
		bool show_demo_window = true;
		bool show_go_button_window = true;
		bool show_cues_window = true;
		bool show_cue_list_toolbox = true;
		bool show_cue_properties_window = true;
		bool show_audio_mixer_window = true;
	} workbench;

	void mainMenuBar()
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("Create New Project", "Ctrl+N", false, false);
			ImGui::Separator();
			ImGui::MenuItem("Open Existing Project", "Ctrl+O", false, false);

			if (ImGui::BeginMenu("Open Recent", false))
				ImGui::EndMenu();

			ImGui::Separator();
			ImGui::MenuItem("Save", "Ctrl+S", false, false);
			ImGui::MenuItem("Save As...", "Ctrl+Shift+S", false, false);
			ImGui::Separator();
			ImGui::MenuItem("Preferences", "Alt+P", false, false);
			ImGui::Separator();
			ImGui::MenuItem("Exit", "Alt+F4", false, false);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::MenuItem("Undo", "Ctrl+Z", false, false);
			ImGui::MenuItem("Redo", "Ctrl+Y", false, false);
			ImGui::Separator();
			ImGui::MenuItem("Cut", "Ctrl+X", false, false);
			ImGui::MenuItem("Copy", "Ctrl+C", false, false);
			ImGui::MenuItem("Paste", "Ctrl+V", false, false);
			ImGui::MenuItem("Duplicate", "Ctrl+D", false, false);
			ImGui::MenuItem("Delete", "Delete", false, false);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Selection"))
		{


			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Playback"))
		{


			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			ImGui::MenuItem("Show ImGui Demo Window", "", &workbench.show_demo_window);
			ImGui::Separator();
			ImGui::MenuItem("Show Go Button Window", "", &workbench.show_go_button_window);
			ImGui::MenuItem("Show Cues Window", "", &workbench.show_cues_window);
			ImGui::MenuItem("Show Cue List Toolbox", "", &workbench.show_cue_list_toolbox, workbench.show_cues_window);
			ImGui::MenuItem("Show Cue Properties Window", "", &workbench.show_cue_properties_window);
			ImGui::Separator();

			if (ImGui::MenuItem("Toggle Fullscreen", "F11", SDL_GetWindowFlags(*main_window) & SDL_WINDOW_FULLSCREEN))
				SDL_SetWindowFullscreen(*main_window, SDL_GetWindowFlags(*main_window) & SDL_WINDOW_FULLSCREEN ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Devices"))
		{


			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			ImGui::MenuItem("Documentation", "", false, false);
			ImGui::MenuItem("About", "", false, false);

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	void goButtonWindow()
	{
		ImVec2 button_size = ImGui::GetContentRegionAvail();
		float_t new_font_scale = button_size.y / ImGui::GetTextLineHeightWithSpacing();

		// something is wrong
		if (new_font_scale <= 0)
			return;

		float_t old_font_scale = ImGui::GetFont()->Scale;
		ImGui::GetFont()->Scale *= new_font_scale;
		ImGui::PushFont(ImGui::GetFont());

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.9f, 0.0f, sin(ImGui::GetTime() * 3.14 / 2) / 5 + 0.8f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.9f, 0.0f, sin(ImGui::GetTime() * 3.14) / 5 + 0.8f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.9f, 0.0f, sin(ImGui::GetTime() * 3.14 * 4) / 5 + 0.8f));

		ImGui::BeginDisabled(false);

		if (ImGui::Button("GO", button_size));

		ImGui::EndDisabled();

		ImGui::PopStyleColor(3);

		ImGui::GetFont()->Scale = old_font_scale;
		ImGui::PopFont();
	}

	void cueListToolbox()
	{
		ImVec2 button_size = ImVec2(48, 48);
		float window_visible = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

		auto container = main_window->getAllTextureIDs();
		auto textures = std::vector(container.begin(), container.end());

		for (uint8_t i = 0; i < textures.size(); i++)
		{
			ImGui::ImageButton(textures[i].c_str(), main_window->getTexture(textures[i]), button_size);
			float_t last_button = ImGui::GetItemRectMax().x;
			float_t next_button = last_button + ImGui::GetStyle().ItemSpacing.x + button_size.x;

			if (i + 1 < textures.size() && next_button < window_visible)
				ImGui::SameLine();
		}

		ImGui::BeginDisabled(project.getSelectedCuesCount() == 0);
		if (ImGui::Button("Delete selected cues"))
			project.deleteSelectedCues();

		ImGui::SameLine();

		if (ImGui::Button("Put playhead at first selected cue"))
			project.setPlayheadCue(*project.getSelectedCues().begin());
		ImGui::EndDisabled();

		ImGui::Separator();
	}

	void cueRowBuilder(SharedCue cue, CueIndex row)
	{
		ImGui::TableNextColumn();

		// this was a thing at some point, but later i removed it because
		// i thought that i just added a lot of useless functions without
		// proper implementation of other more important stuff
		
		//if (cue->use_custom_color)
		//{
		//	ImVec4 color = { 0.0f, 0.0f, 0.0f, 0.25f };
		//	ImGui::ColorConvertHSVtoRGB(cue->custom_color_hue, 1.0, 1.0, color.x, color.y, color.z);
		//	ImU32 cell_bg_color = ImGui::GetColorU32(color);
		//	ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, cell_bg_color);
		//}

		auto selected = project.isCueSelected(cue);
		if (ImGui::Selectable(std::format("##sel{}", row).c_str(), selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
		{
			auto ctrl = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
			auto shift = ImGui::IsKeyDown(ImGuiKey_LeftShift);

			if (ctrl && !shift && !selected)
				project.addCueToSelection(cue);
			else if (!shift && selected)
				project.removeCueFromSelection(cue);
			else if (!ctrl && !shift)
			{
				project.resetCueSelection();
				project.addCueToSelection(cue);
			}
			else if (shift && !ctrl)
			{
				if (!project.isCueSelected(cue))
					project.addCueToSelection(cue);

				//auto begin_pos = std::min(getCuePos(*selected_cues.begin()), index);
				//auto end_pos = std::max(getCuePos(*selected_cues.begin()), index);
				//
				//auto important_one = *selected_cues.begin();
				//
				//resetCueSelection();
				//selectCue(important_one);
				//
				//for (int i = begin_pos; i <= end_pos; i++)
				//{
				//	if (!isCueSelected(i))
				//		selectCue(i);
				//}

				// is this proxied (whatever) function:

				//project.batchSelectCues(cue);
			}
		}

		ImGui::SameLine(1, 0);

		// technical debt be like:

		auto size = ImVec2(ImGui::GetItemRectSize().y - ImGui::GetStyle().ItemSpacing.y, ImGui::GetItemRectSize().y - ImGui::GetStyle().ItemSpacing.y);

		if (project.getPlayheadPosition() == row)
			ImGui::Image(main_window->getTexture("playhead"), size);

		ImGui::SameLine(size.x + ImGui::GetStyle().ItemSpacing.y);

		auto status = cue->getPlaybackStatus();
		if (status != PlaybackStatus::UNLOADED && status != PlaybackStatus::LOADED)
			ImGui::Image(status == PlaybackStatus::ACTIVE ? main_window->getTexture("playing") : (status == PlaybackStatus::INVALID ? main_window->getTexture("error") : main_window->getTexture("paused")), size);

		ImGui::SameLine(size.x + ImGui::GetStyle().ItemSpacing.y);

		// i believe many technical debts in this project could be
		// solved by visitor pattern, but currently im not sure how
		// to use and implement it correctly

		SDL_Texture* which = main_window->getTexture("sequrunner");

		ImGui::SameLine((size.x + ImGui::GetStyle().ItemSpacing.y) * 2);
		ImGui::Image(which, size);

		ImGui::TableNextColumn();
		ImGui::Text(cue->sequence_tag.c_str());

		ImGui::TableNextColumn();
		ImGui::Text(cue->getShownShortDescription().c_str());

		ImGui::TableNextColumn();

		if (auto continuouscue = SharedDynamicCast<ContinuousCue>(cue))
		{
			ImGui::TableNextColumn();
			ImGui::ProgressBar((std::sin(ImGui::GetTime() * 3.14 * 0.5 + row * 0.25) / 2 + 0.5f) * 3, ImVec2(ImGui::GetContentRegionAvail().x - (ImGui::GetStyle().CellPadding.x), 0), "");
			ImGui::SameLine(1, (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("03:20.44").x) / 2);
			ImGui::Text("03:20.44");

			ImGui::TableNextColumn();
			ImGui::ProgressBar((std::sin(ImGui::GetTime() * 3.14 * 0.5 + row * 0.25) / 2 + 0.5f) * 3 - 1.0f, ImVec2(ImGui::GetContentRegionAvail().x - (ImGui::GetStyle().CellPadding.x), 0), "");
			ImGui::SameLine(1, (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("02:23.84").x) / 2);
			ImGui::Text("02:23.84");

			ImGui::TableNextColumn();
			ImGui::ProgressBar((std::sin(ImGui::GetTime() * 3.14 * 0.5 + row * 0.25) / 2 + 0.5f) * 3 - 2.0f, ImVec2(ImGui::GetContentRegionAvail().x - (ImGui::GetStyle().CellPadding.x), 0), "");
			ImGui::SameLine(1, (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("02:23.84").x) / 2);
			ImGui::Text("02:23.84");
		}
		else
		{
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
		}

		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - (ImGui::GetStyle().CellPadding.x));
		const char* modes[] = { "Off", "Continue", "Follow" };

		int buf = 0;
		ImGui::BeginDisabled();
		//ImGui::SliderInt(std::format("##slid{}", row).c_str(), (int*)&project.getCueAtIndex(row)->follow, 0, 2, modes[(int)project.getCueAtIndex(row)->follow]);
		ImGui::SliderInt(std::format("##slid{}", row).c_str(), &buf, 0, 2, modes[buf]);
		ImGui::EndDisabled();
	}

	void cuesWindow()
	{
		if (workbench.show_cue_list_toolbox)
			cueListToolbox();

		if (ImGui::BeginChild("Cue list") && ImGui::BeginTable("##cue_list", 8, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Reorderable))
		{
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_WidthFixed, 80.0f);
			ImGui::TableSetupColumn("Number");
			ImGui::TableSetupColumn("Cue description");
			ImGui::TableSetupColumn("Target");
			ImGui::TableSetupColumn("Pre-wait");
			ImGui::TableSetupColumn("Action");
			ImGui::TableSetupColumn("Post-wait");
			ImGui::TableSetupColumn("Auto");

			ImGui::TableSetupScrollFreeze(1, 1);
			ImGui::TableHeadersRow();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			for (int row = 0; row < project.getProjectCuesCount(); row++)
			{
				ImGui::TableNextRow();
				cueRowBuilder(project.getCueFromPosition(row), row);
			}
			ImGui::PopStyleVar();

			ImGui::EndTable();
		}
		ImGui::EndChild();
	}

	void ezTextField(std::string text, std::string* dst, bool multiline = false)
	{
		text += ":";
		ImGui::Text(text.c_str());
		ImGui::SetNextItemWidth(-FLT_MIN);

		text = "##" + text;
		if (multiline)
			ImGui::InputTextMultiline(text.c_str(), dst);
		else
			ImGui::InputText(text.c_str(), dst);
	}

	void cuePropertiesWindow()
	{
		if (project.getSelectedCues().size() == 0)
		{
			ImGui::Text("Select cue to view and edit its properties.");
			return;
		}
		else if (project.getSelectedCues().size() > 1)
		{
			ImGui::Text("Batch-editing is not supported at the moment.");
			return;
		}

		SharedCue cue = *project.getSelectedCues().begin();

		if (auto basecue = cue)
		{
			ImGui::SeparatorText("Basics");

			ezTextField("Sequence Tag", &cue->sequence_tag);
			ezTextField("Short Description", &cue->short_description);
			ezTextField("Personal Notes", &cue->user_notes, true);
		}

		if (auto filetargetable = SharedDynamicCast<FileTargetable>(cue))
		{
			ImGui::SeparatorText("File Target");
			ezTextField("Filename", &filetargetable->target);
		}

		if (auto cuetargetable = SharedDynamicCast<CueTargetable<BaseCue>>(cue))
		{
			ImGui::SeparatorText("Cue Target");

			static std::string cuenum;

			if (cuetargetable->target.expired())
			{
				ezTextField("Cue Sequence Tag", &cuenum);
				if (ImGui::Button("Try Lock"))
				{
					auto k = std::find_if(project.project_cues.begin(), project.project_cues.end(), [](std::shared_ptr<BaseCue>& element) { return element->sequence_tag == cuenum; });
					if (k != project.project_cues.end())
						cuetargetable->target = *k;
				}
			}
			else
			{
				ImGui::Text(cuetargetable->target.lock()->sequence_tag.c_str());
				if (ImGui::Button("Unlock"))
					cuetargetable->target.reset();
			}
		}
	}

	void loop()
	{
		ImGui::DockSpaceOverViewport();

		if (ImGui::BeginMainMenuBar())
			mainMenuBar();

		if (workbench.show_demo_window)
			ImGui::ShowDemoWindow(&workbench.show_demo_window);

		if (workbench.show_go_button_window)
		{
			if (ImGui::Begin("GO Button", &workbench.show_go_button_window))
				goButtonWindow(); ImGui::End();
		}

		if (workbench.show_cues_window)
		{
			if (ImGui::Begin("Cues", &workbench.show_cues_window))
				cuesWindow(); ImGui::End();
		}

		if (workbench.show_cue_properties_window)
		{
			if (ImGui::Begin("Cue Properties", &workbench.show_cue_properties_window))
				cuePropertiesWindow(); ImGui::End();
		}

		if (workbench.show_audio_mixer_window)
		{
			//if (ImGui::Begin("Audio Mixer", &workbench.show_audio_mixer_window))
			//	audioMixerWindow(); ImGui::End();
		}

		ImGui::Begin("asdf");

		for (auto& asdf : main_window->getAllTextureIDs())
		{
			ImGui::Image(main_window->getTexture(asdf), ImVec2(64, 64));
			ImGui::SameLine();
			ImGui::ImageButton(asdf.c_str(), main_window->getTexture(asdf), ImVec2(64, 64));
		}

		ImGui::End();
	}

	bool event(SDL_Event event)
	{
		bool done = false;

		if (event.type == SDL_QUIT)
			done = true;

		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(*main_window))
			done = true;

		if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F11)
		{
			if (event.window.windowID == SDL_GetWindowID(*main_window))
				SDL_SetWindowFullscreen(*main_window, SDL_GetWindowFlags(*main_window) & SDL_WINDOW_FULLSCREEN ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
		}

		if (event.window.windowID == SDL_GetWindowID(*main_window))
			ImGui_ImplSDL2_ProcessEvent(&event);

		return done;
	}

	void init()
	{
		for (auto& filename : assets.getFilenames("textures"))
		{
			auto file_content = assets.loadFile("textures/" + filename);

			auto sdl_data = SDL_RWFromMem(file_content.data(), file_content.size());
			auto sdl_surface = SDL_LoadBMP_RW(sdl_data, true);

			// .bmp
			main_window->createTexture(filename.substr(0, filename.size() - 4), sdl_surface);

			SDL_FreeSurface(sdl_surface);
		}

		// yes, file is being read twice
		auto file_content = assets.loadFile("textures/sequrunner.bmp");

		auto sdl_data = SDL_RWFromMem(file_content.data(), file_content.size());
		auto sdl_surface = SDL_LoadBMP_RW(sdl_data, true);

		SDL_SetWindowIcon(*main_window, sdl_surface);
		SDL_FreeSurface(sdl_surface);

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		auto display_bounds = SDLWindow::getCoolResolution(1, 1);
		float font_size = std::roundf(std::max(display_bounds.first, display_bounds.second) / 10.0f) / 10.0f;

		auto cool_font = assets.loadFile("misc/Roboto-Regular.ttf");
		char* fontdata = new char[cool_font.size()];
		// fontdata is actually supposed to get deleted after font atlas is built, but its not done yet
		std::copy(cool_font.begin(), cool_font.end(), fontdata);

		io.Fonts->AddFontFromMemoryTTF(fontdata, cool_font.size(), font_size);
		io.IniFilename = "sequrunner-gui.ini";

		ImGui::StyleColorsLight();

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding = ImVec2(10, 10);
		style.FramePadding = ImVec2(6, 3);
		style.CellPadding = ImVec2(3, 3);
		style.ItemSpacing = ImVec2(8, 4);
		style.ItemInnerSpacing = ImVec2(4, 4);
		style.TouchExtraPadding = ImVec2(0, 0);
		style.IndentSpacing = 21;
		style.ScrollbarSize = 17;
		style.GrabMinSize = 12;
		style.WindowBorderSize = 1;
		style.FrameBorderSize = 0;
		style.ChildBorderSize = 1;
		style.PopupBorderSize = 1;
		style.TabBorderSize = 0;
		style.WindowRounding = 6;
		style.ChildRounding = 0;
		style.FrameRounding = 6;
		style.PopupRounding = 12;
		style.ScrollbarRounding = 2;
		style.GrabRounding = 6;
		style.TabRounding = 6;
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_None;
		style.ColorButtonPosition = ImGuiDir_Left;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
		style.SeparatorTextBorderSize = 5;
		style.SeparatorTextAlign = ImVec2(0.0f, 0.5f);
		style.SeparatorTextPadding = ImVec2(20, 3);
		style.LogSliderDeadzone = 4;
		style.DisplaySafeAreaPadding = ImVec2(3, 3);
	}
}
#pragma once

#include <string>
#include <chrono>
#include <regex>
#include <list>

#include "utility.hpp"
#include "media.hpp"

namespace engine::playback
{
	using namespace engine::utility;

	/// <summary>
	/// Typedef for storing points-in-time/durations/etc.
	/// </summary>
	typedef std::chrono::duration<float> PlaybackTime;

	/// <summary>
	/// Playback statuses that cues might return
	/// </summary>
	enum class PlaybackStatus
	{
		INVALID,
		UNLOADED,
		LOADED,
		PAUSED,
		ACTIVE
	};

	/// <summary>
	/// Abstract structure that exposes basic cue functionality
	/// </summary>
	struct BaseCue
	{
		std::string short_description;
		std::string sequence_tag;
		std::string user_notes;

		virtual std::string getShownShortDescription();

		virtual PlaybackStatus getPlaybackStatus() = 0;
		virtual void validatePlayback() = 0;
		virtual void startPlayback() = 0;
	};

	/// <summary>
	/// Cue modifier structure that exposes functionality for cues that could be preloaded to decrease startup time and/or unloaded to save RAM/CPU/etc.
	/// </summary>
	struct LoadableCue : virtual BaseCue
	{
		virtual void loadPlayback() = 0;
		virtual void freePlayback() = 0;
	};

	/// <summary>
	/// Cue modifier structure that exposes functionality for cues that aren't instant and run until they are stopped manually or stopped after some period of time
	/// </summary>
	struct ContinuousCue : virtual BaseCue
	{
		virtual PlaybackTime getPlaybackDuration() = 0;
		virtual void stopPlayback() = 0;
	};

	/// <summary>
	/// Cue modifier structure that exposes functionality for cues that allow seeking through them (for example audio/video/etc.)
	/// </summary>
	struct SeekableCue : virtual ContinuousCue
	{
		virtual void seekPlayback(PlaybackTime position) = 0;
		virtual PlaybackTime tellPlayback() = 0;

		virtual void pausePlayback() = 0;
		virtual void resetPlayback() = 0;
	};

	typedef Shared<BaseCue> SharedCue;
	typedef Weak<BaseCue> WeakCue;

	typedef std::list<SharedCue> SharedCueList;
	typedef std::list<WeakCue> WeakCueList;

	typedef int32_t CueIndex;

	/// <summary>
	/// Templated cue modifier structure that stores and exposes targeting functionality
	/// </summary>
	template<class Type>
	struct Targetable
	{
		Type target;
	};

	typedef Targetable<std::string> FileTargetable;

	template<class Type = BaseCue, std::enable_if_t<std::is_base_of_v<BaseCue, Type>, bool> = false>
	using CueTargetable = Targetable<Weak<Type>>;

	/// <summary>
	/// Project struct that stores cues, provides helper methods for editing them, keeps project related info, loads/saves data from/to .srj project files and etc.
	/// </summary>
	struct Project : UUIDable
	{
		SharedCueList project_cues;
		SharedCueList selected_cues;

		SharedCue playhead_cue;

		CueIndex getProjectCuesCount();
		CueIndex getSelectedCuesCount();

		CueIndex getCuePosition(SharedCue cue);
		SharedCue getCueFromPosition(CueIndex index);

		void insertCueBefore(SharedCue cue, SharedCue before = nullptr);

		bool isCueSelected(SharedCue cue);
		void addCueToSelection(SharedCue cue, SharedCue before = nullptr);
		void removeCueFromSelection(SharedCue cue);
		void deleteSelectedCues();
		void resetCueSelection();

		void setPlayheadCue(SharedCue cue);
		SharedCue getPlayheadCue();
		void unsetPlayheadCue();
		CueIndex getPlayheadPosition();

		SharedCueList getAllCues();
		SharedCueList getSelectedCues();
	};

	struct MemoCue : BaseCue
	{
		PlaybackStatus getPlaybackStatus() override { return PlaybackStatus::LOADED; }
		void validatePlayback() override {}
		void startPlayback() override {}
	};

	struct FooCue : BaseCue
	{
		PlaybackStatus getPlaybackStatus() override { return PlaybackStatus::LOADED; }
		void validatePlayback() override {}
		void startPlayback() override {}
	};
}
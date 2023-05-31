#include "playback.hpp"

namespace engine::playback
{
	std::string BaseCue::getShownShortDescription()
	{
		if (short_description.size() > 0)
			return short_description;
		else
			return std::regex_replace(user_notes, std::regex("[\\r\\n]"), " ");
	}

	CueIndex Project::getProjectCuesCount()
	{
		return project_cues.size();
	}

	CueIndex Project::getSelectedCuesCount()
	{
		return selected_cues.size();
	}

	CueIndex Project::getCuePosition(SharedCue cue)
	{
		return std::distance(project_cues.begin(), std::find(project_cues.begin(), project_cues.end(), cue));
	}

	SharedCue Project::getCueFromPosition(CueIndex index)
	{
		auto iterator = project_cues.begin();
		std::advance(iterator, index);

		return *iterator;
	}

	void Project::insertCueBefore(SharedCue cue, SharedCue before)
	{
		if (before)
			project_cues.insert(std::find(project_cues.begin(), project_cues.end(), before), cue);
		else
			project_cues.push_back(cue);
	}

	bool Project::isCueSelected(SharedCue cue)
	{
		return std::find(selected_cues.begin(), selected_cues.end(), cue) != selected_cues.end();
	}

	void Project::addCueToSelection(SharedCue cue, SharedCue before)
	{
		if (before)
			selected_cues.insert(std::find(selected_cues.begin(), selected_cues.end(), before), cue);
		else
			selected_cues.push_back(cue);
	}

	void Project::removeCueFromSelection(SharedCue cue)
	{
		selected_cues.remove(cue);
	}

	void Project::deleteSelectedCues()
	{
		auto predicate = [&](const SharedCue& element) -> bool
		{
			return std::find(selected_cues.begin(), selected_cues.end(), element) != selected_cues.end();
		};

		project_cues.remove_if(predicate);
		selected_cues.clear();
	}

	void Project::resetCueSelection()
	{
		selected_cues.clear();
	}

	void Project::setPlayheadCue(SharedCue cue)
	{
		playhead_cue = cue;
	}

	SharedCue Project::getPlayheadCue()
	{
		return playhead_cue;
	}

	void Project::unsetPlayheadCue()
	{
		playhead_cue = nullptr;
	}

	CueIndex Project::getPlayheadPosition()
	{
		return playhead_cue ? getCuePosition(playhead_cue) : -1;
	}

	SharedCueList Project::getAllCues()
	{
		return project_cues;
	}

	SharedCueList Project::getSelectedCues()
	{
		return selected_cues;
	}
}
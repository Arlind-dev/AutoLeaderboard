#include <Geode/Geode.hpp>
using namespace geode::prelude;
#include <Geode/modify/GJGameLevel.hpp>
#include <Geode/modify/GameLevelManager.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

enum class UpdateTrigger { LevelPage, OnExit };

static UpdateTrigger getTrigger() {
	return Mod::get()->getSettingValue<std::string>("update-trigger") == "on-exit"
		? UpdateTrigger::OnExit
		: UpdateTrigger::LevelPage;
}

class $modify(ModGJGameLevel, GJGameLevel) {
	void updateLeaderboard() {
		auto gm = GameManager::sharedState();
		auto glm = GameLevelManager::sharedState();
		auto leaderboardType = gm->getIntGameVariable("0098");
		auto leaderboardMode = gm->getIntGameVariable("0164");

		// using friends cuts down the amount of data fetched compared to all-time or weekly
		// gd only updates whatever stat is fetched, so we have to call getLevelLeaderboard twice for platformer levels
		glm->getLevelLeaderboard(static_cast<GJGameLevel*>(this), LevelLeaderboardType::Friends, LevelLeaderboardMode::Time);
		if (isPlatformer()) glm->getLevelLeaderboard(static_cast<GJGameLevel*>(this), LevelLeaderboardType::Friends, LevelLeaderboardMode::Points);

		// getLevelLeaderboard automatically sets the mode and type to whatever was passed into it, so we have to reset it
		gm->setIntGameVariable("0098", leaderboardType);
		gm->setIntGameVariable("0164", leaderboardMode);
	}
};

class $modify(ModLevelInfoLayer, LevelInfoLayer) {
	struct Fields {
		bool hasChecked;
	};

	void levelDownloadFinished(GJGameLevel* p0) {
		LevelInfoLayer::levelDownloadFinished(p0);
		if (getTrigger() == UpdateTrigger::LevelPage) updateLeaderboardAuto();
	}

	void updateSideButtons() {
		LevelInfoLayer::updateSideButtons();
		if (getTrigger() == UpdateTrigger::LevelPage && !LevelInfoLayer::shouldDownloadLevel()) updateLeaderboardAuto();
	}

	$override
	void onEnter() {
		LevelInfoLayer::onEnter();
		// Reset the flag when returning to the level info page so it updates after playing
		if (getTrigger() == UpdateTrigger::LevelPage)
			m_fields->hasChecked = false;
	}

	void updateLeaderboardAuto() {
		if (m_fields->hasChecked) return;
		m_fields->hasChecked = true;
		static_cast<ModGJGameLevel*>(m_level)->updateLeaderboard();
	}
};

class $modify(ModPlayLayer, PlayLayer) {
	$override
	void levelComplete() {
		auto oldBestTime = m_level->m_bestTime;
		PlayLayer::levelComplete();

		if (m_level->m_levelType != GJLevelType::Saved) return;
		// for platformer, only update on new best time; for classic, always update since coins may have been collected
		bool isNewBestTime = m_level->m_bestTime < oldBestTime || oldBestTime == 0;
		if (m_level->isPlatformer() && !isNewBestTime) return;

		if (getTrigger() == UpdateTrigger::LevelPage)
			static_cast<ModGJGameLevel*>(m_level)->updateLeaderboard();
	}

	$override
	void onQuit() {
		// submit before calling the original so m_level is still valid
		if (getTrigger() == UpdateTrigger::OnExit && m_level->m_levelType == GJLevelType::Saved)
			static_cast<ModGJGameLevel*>(m_level)->updateLeaderboard();

		PlayLayer::onQuit();
	}
};

class $modify(GameLevelManager) {
	$override
	void onGetLevelLeaderboardCompleted(gd::string response, gd::string tag) {
		GameLevelManager::onGetLevelLeaderboardCompleted(response, tag);
		// m_storedLevels holds all cached level-related data (levels, comments, leaderboards, etc)
		// m_timerDict holds timestamps for when the data was last fetched. deleting these forces a refetch
		// prevents the leaderboard from erroneously showing "1 second ago" despite beating the level earlier
		m_storedLevels->removeObjectForKey(tag);
		m_timerDict->removeObjectForKey(tag);
	}
};

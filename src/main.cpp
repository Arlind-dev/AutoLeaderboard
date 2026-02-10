#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include <Geode/modify/GJGameLevel.hpp>
#include <Geode/modify/GameLevelManager.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

class $modify(ModGJGameLevel, GJGameLevel) {
	void updateLeaderboard() {
		auto gm = GameManager::sharedState();
		auto glm = GameLevelManager::sharedState();

		auto leaderboardType = gm->getIntGameVariable("0098");
		auto leaderboardMode = gm->getIntGameVariable("0164");

		// using weekly should cut down the amount of data fetched
		// might help mobile players on cell service but probably doesn't matter

		// gd only updates whatever stat is fetched, so we have to call getLevelLeaderboard twice for platformer levels
		glm->getLevelLeaderboard(static_cast<GJGameLevel*>(this), LevelLeaderboardType::Weekly, LevelLeaderboardMode::Time);
		if (isPlatformer()) glm->getLevelLeaderboard(static_cast<GJGameLevel*>(this), LevelLeaderboardType::Weekly, LevelLeaderboardMode::Points);

		// getLevelLeaderboard automatically sets the mode and type to whatever was passed into it, so we have to reset it
		gm->setIntGameVariable("0098", leaderboardType);
		gm->setIntGameVariable("0164", leaderboardMode);
	}
};

class $modify(ModLevelInfoLayer, LevelInfoLayer) {
	struct Fields {
		bool hasChecked; // used to check if the leaderboard has been updated already
	};

	void levelDownloadFinished(GJGameLevel* p0) {
		LevelInfoLayer::levelDownloadFinished(p0);
		updateLeaderboardAuto();
	}

	void updateSideButtons() {
		LevelInfoLayer::updateSideButtons();
		if (!LevelInfoLayer::shouldDownloadLevel()) updateLeaderboardAuto();
	}

	$override
	void onEnter() {
		LevelInfoLayer::onEnter();
		// Reset the flag when returning to the level info page so it updates after playing
		m_fields->hasChecked = false;
	}

	void updateLeaderboardAuto() {
		if (m_fields->hasChecked) return;
		m_fields->hasChecked = true;

		static_cast<ModGJGameLevel*>(m_level)->updateLeaderboard();
	}
};

class $modify(PlayLayer) {
	$override
	void levelComplete() {
		PlayLayer::levelComplete();
		static_cast<ModGJGameLevel*>(m_level)->updateLeaderboard();
	}
};

class $modify(GameLevelManager) {
	$override
	void handleIt(bool idk, gd::string fetchedData, gd::string tag, GJHttpType httpType) {
		GameLevelManager::handleIt(idk, fetchedData, tag, httpType);
		if (httpType != GJHttpType::GetLevelLeaderboard) return;

		// m_storedLevels holds all cached level-related data (levels, comments, leaderboards, etc)
		// m_timerDict holds timestamps for when the data was last fetched. deleting these forces a refetch

		// doing this prevents the leaderboard from erroneously showing "1 second ago" despite beating the level earlier than that

		m_storedLevels->removeObjectForKey(tag);
		m_timerDict->removeObjectForKey(tag);
	}
};

# Changelog

## 2.1.0

- Fixed a crash on Android caused by the `handleIt` hook interfering with the rewards system
- Added a new **Update Trigger** setting to choose between two leaderboard update behaviours:
  - **level-page** *(default)*: updates when viewing the level page
  - **on-exit**: updates when exiting a level (BetterInfo-style)
- Both triggers now share the same smart guards: platformer levels only update on a new best time, and editor/daily levels are never submitted
- Both triggers now use the Friends leaderboard type for lighter requests

## 2.0.2

- Create a new release to refresh the index description, changelog, and logo

## 2.0.1

- Add disclaimer for temporary IP bans
- Change minimum Geode version to v5.0.0

## 2.0.0

- Changed the leaderboard update trigger from death to map open
- Updated Geode target to 5.0.1
- Removed unneeded dependencies

---

## 1.2.1

- Added iOS compatibility (still experimental)
- Fixed the mod downloading Level Info & Mod Badge Info even though they aren't required

## 1.2.0

Thanks to NinKaz for all of the bugfixes/enhancements for this version!

- No more lag when opening a level page
- Always update leaderboard stats (previously didn't if it had already been updated once)
- Leaderboards stats now get updated as soon as you get a new best (previously only on the level page)
- New logo

## 1.0.1

- Ported the mod to Geode 4.0.0-beta.1 (GD 2.2074)

## 1.0.0

- Automatically update normal and platformer mode levels' leaderboard if percentage is above 0

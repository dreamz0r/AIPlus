# AI Plus

AI Plus is a DayZ Expansion AI mod that adds dynamic hostile AI activity around player behavior and configured map locations. It is built for servers that want more unpredictable PvE pressure without turning every encounter into a static mission.

The mod can spawn AI counter-raids when players attack bases, send bandit groups after random online players, and run global POI missions at configured buildings such as police stations, fire stations, hospitals, or custom locations. POI missions include map markers, global notices, locked reward chests, and cleanup handling.

## Requirements

Load AI Plus after these mods:

- DayZ Expansion Core
- DayZ Expansion AI
- DayZ Expansion BaseBuilding

## Install Notes

On first server start, the mod creates:

`$profile:AIPlus/AIPlusConfig.json`

Use [AIPlusConfig.example.json](AIPlusConfig.example.json) as a reference when editing the generated config.

## Main Features

- Counter-raid AI can spawn when players damage base-building parts.
- Territory members can be prevented from triggering counter-raids on their own territory.
- Counter-raid AI can be configured to ignore members of the raided territory while attacking outsiders.
- Player hunts can randomly target online players and send a private warning to that player.
- POI missions can spawn at police, fire station, hospital, or custom building class names.
- POI mission notices are global and use the nearest town, village, or city name.
- POI mission markers are 2D map markers only.
- POI reward chests stay locked until all mission AI are dead.
- Reward chests cannot be picked up.
- Reward chests can show a road flare effect and sound when unlocked.
- AI and rewards are cleaned up when events end.
- AI spawned in contaminated gas zones receive gas protection.
- AI spawn positions are checked to avoid water and bad terrain placement.

## Config Overview

Chance values use `0.0` to `1.0`.

Examples:

- `0.0` = disabled by chance roll
- `0.25` = 25 percent chance
- `1.0` = always passes the chance roll

### Counter-Raids

Counter-raids are controlled by:

- `EnableCounterRaids`
- `CounterRaidChance`
- `CounterRaidCooldownSeconds`
- `CounterRaidPatrolDurationSeconds`
- `CounterRaidSpawnDistanceMin`
- `CounterRaidSpawnDistanceMax`
- `CounterRaidPatrolRadius`
- `IgnoreTerritoryMemberDamage`
- `CounterRaidProtectTerritoryMembers`
- `SendCounterRaidNotice`
- `CounterRaidSpawn`

By default, counter-raid AI spawn `200-300m` from the player location and move toward the player's last known position.

Counter-raid corpse loot is controlled by `CounterRaidSpawn.LootableChance`. The default is `0.0`, meaning counter-raid AI are unlootable unless you raise that value.

### Player Hunts

Player hunts are controlled by:

- `EnablePlayerHunts`
- `PlayerHuntCheckSeconds`
- `PlayerHuntChance`
- `PlayerHuntCooldownSeconds`
- `PlayerHuntPatrolDurationSeconds`
- `PlayerHuntSpawnDistanceMin`
- `PlayerHuntSpawnDistanceMax`
- `PlayerHuntPatrolRadius`
- `PlayerHuntNoticeTitle`
- `PlayerHuntNoticeText`
- `PlayerHuntSpawn`

Default player hunt notice:

Title: `Bandits are hunting you.`

Text: `You have been spotted by bandits. Run, hide or fight!`

### POI Missions

POI missions are configured in `POIEncounters`.

Important fields per mission:

- `Enabled`
- `ID`
- `RaidType`
- `BuildingClassNames`
- `Chance`
- `CooldownSeconds`
- `PatrolDurationSeconds`
- `PatrolRadius`
- `CreateExpansionMarker`
- `MarkerName`
- `MarkerIcon`
- `SendGlobalNotice`
- `NoticeTitle`
- `NoticeText`
- `CompletionNoticeTitle`
- `CompletionNoticeText`
- `Spawn`

Default mission spawn notice:

Title: `Bandits spotted`

Text: `Bandits are looting the '%1' at '%2', take them out!`

`%1` is replaced with the mission type, such as `Police`, `Firestation`, or `Hospital`.

`%2` is replaced with the nearest map location name.

`POIMinDistanceBetweenEvents` controls how close active POI missions can be to each other. The default is `2500m`.

## Reward Chests

POI missions use `AIPlusRewardChest` by default.

Reward chest settings:

- `ChestClassName`
- `RewardChestLifetimeSeconds`
- `EnableRewardChestUnlockIndicator`
- `RewardChestUnlockIndicatorHeight`
- `ChestLootMin`
- `ChestLootMax`
- `ChestLootItemChance`
- `ChestLootItems`

The reward chest is locked and hidden from inventory until all mission AI are dead. After the mission AI are eliminated, the chest unlocks, receives loot, and can show a road flare particle and sound if enabled.

`RewardChestLifetimeSeconds` controls how long the unlocked chest remains before cleanup.

## AI Loadouts And Factions

Each spawn profile has:

- `MinAI`
- `MaxAI`
- `Loadouts`
- `Factions`
- `Units`
- `Formation`
- `Speed`
- `UnderThreatSpeed`
- `AccuracyMin`
- `AccuracyMax`
- `ThreatDistanceLimit`
- `LootableChance`
- `UnlimitedReload`

`Loadouts` must be valid DayZ Expansion AI loadout names without `.json`.

`Factions` must be valid DayZ Expansion AI faction names, such as `Raiders`, `West`, `East`, `Mercenaries`, `Guards`, `Civilian`, or `Passive`.

## Cleanup Behavior

AI Plus tracks spawned AI, reward chests, event gear, and mission markers so they can be removed when an event ends.

The cleanup system is designed to avoid deleting player base-building parts, including vanilla and Expansion base-building objects.

## Admin Tips

- Keep `DebugLogging` disabled on live servers unless troubleshooting.
- Use `1.0` chances and short cooldowns only while testing.
- Raise cooldowns and lower chance values before running on a populated server.
- Keep POI mission spacing high enough to avoid several missions stacking in the same area.
- Use Expansion AI loadouts that match your server economy and difficulty.

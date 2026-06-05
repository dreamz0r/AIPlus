class AIPlusManager
{
	protected static ref AIPlusManager s_Instance;

	protected ref map<string, int> m_Cooldowns;
	protected ref array<ref AIPlusPatrol> m_Patrols;
	protected ref map<string, ref AIPlusPOIEvent> m_ActivePOIEvents;
	protected bool m_Started;

	void AIPlusManager()
	{
		m_Cooldowns = new map<string, int>();
		m_Patrols = {};
		m_ActivePOIEvents = new map<string, ref AIPlusPOIEvent>();
	}

	static AIPlusManager Get()
	{
		if (!s_Instance)
			s_Instance = new AIPlusManager();

		return s_Instance;
	}

	void Start()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (m_Started)
			return;

		AIPlusConfigData config = AIPlusConfig.Get();
		if (!config.Enabled)
			return;

		m_Started = true;
		WarmupRandom();
		AIPlusPOIWorld.Load(config.POIEncounters);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(CheckPlayerHunts, config.PlayerHuntCheckSeconds * 1000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(CheckPOIEncounters, config.POICheckSeconds * 1000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(CleanupPatrolRefs, 60000, true);
	}

	void OnCounterRaid(PlayerBase player, Object raidTarget)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		AIPlusConfigData config = AIPlusConfig.Get();
		if (!config.Enabled || !config.EnableCounterRaids || !player || !player.GetIdentity() || !raidTarget)
			return;

		ExpansionTerritory raidedTerritory = AIPlusTerritory.GetTerritoryAt(raidTarget.GetPosition());
		if (config.IgnoreTerritoryMemberDamage && AIPlusTerritory.IsMember(raidedTerritory, player))
		{
			Log("Counter raid ignored because " + GetPlayerID(player) + " damaged their own territory.");
			return;
		}

		vector lastKnownPosition = player.GetPosition();
		string playerID = GetPlayerID(player);
		string key = "counter:" + playerID;
		if (IsOnCooldown(key))
			return;

		SetCooldown(key, config.CounterRaidCooldownSeconds);

		if (!Roll(config.CounterRaidChance))
			return;

		vector spawnPosition;
		if (!FindSpawnPosition(lastKnownPosition, config.CounterRaidSpawnDistanceMin, config.CounterRaidSpawnDistanceMax, spawnPosition))
			return;

		TStringArray protectedUIDs;
		if (config.CounterRaidProtectTerritoryMembers)
			protectedUIDs = AIPlusTerritory.GetMemberUIDs(raidedTerritory);

		CreatePatrol("CounterRaid-" + playerID, spawnPosition, lastKnownPosition, config.CounterRaidPatrolRadius, config.CounterRaidPatrolDurationSeconds, config.CounterRaidSpawn, protectedUIDs);

		if (config.SendCounterRaidNotice)
			SendPrivateNotice(config.CounterRaidNoticeTitle, config.CounterRaidNoticeText, player.GetIdentity());
	}

	void CheckPlayerHunts()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		AIPlusConfigData config = AIPlusConfig.Get();
		if (!config.Enabled || !config.EnablePlayerHunts)
			return;

		array<Man> players = {};
		GetGame().GetPlayers(players);

		foreach (Man man: players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || !player.IsAlive() || !player.GetIdentity())
				continue;

			string playerID = GetPlayerID(player);
			string key = "hunt:" + playerID;
			if (IsOnCooldown(key))
				continue;

			SetCooldown(key, config.PlayerHuntCooldownSeconds);

			if (!Roll(config.PlayerHuntChance))
				continue;

			vector lastKnownPosition = player.GetPosition();
			vector spawnPosition;
			if (!FindSpawnPosition(lastKnownPosition, config.PlayerHuntSpawnDistanceMin, config.PlayerHuntSpawnDistanceMax, spawnPosition))
				continue;

			CreatePatrol("PlayerHunt-" + playerID, spawnPosition, lastKnownPosition, config.PlayerHuntPatrolRadius, config.PlayerHuntPatrolDurationSeconds, config.PlayerHuntSpawn);
			SendPrivateNotice(config.PlayerHuntNoticeTitle, config.PlayerHuntNoticeText, player.GetIdentity());
		}
	}

	void CheckPOIEncounters()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		AIPlusConfigData config = AIPlusConfig.Get();
		if (!config.Enabled || !config.EnablePOIEncounters || !config.POIEncounters)
			return;

		array<ref AIPlusPOIEncounter> encounters = {};
		foreach (AIPlusPOIEncounter configuredEncounter: config.POIEncounters)
		{
			if (configuredEncounter)
				encounters.Insert(configuredEncounter);
		}

		while (encounters.Count() > 0)
		{
			int encounterIndex = Math.RandomInt(0, encounters.Count());
			AIPlusPOIEncounter encounter = encounters.Get(encounterIndex);
			encounters.Remove(encounterIndex);

			if (!encounter || !encounter.Enabled)
				continue;

			if (IsPOIEventActive(encounter.ID))
				continue;

			string key = "poi:" + encounter.ID;
			if (IsOnCooldown(key))
				continue;

			if (!Roll(encounter.Chance))
				continue;

			string locationName;
			AIPlusBuildingInstance building = PickAvailablePOIBuilding(encounter, locationName);
			if (!building)
			{
				Log("No available building positions found for POI event " + encounter.ID);
				continue;
			}

			vector eventPosition = ExpansionAIPatrol.GetPlacementPosition(building.Position);

			int patrolCleanupSeconds = encounter.PatrolDurationSeconds + encounter.RewardChestLifetimeSeconds + 30;
			AIPlusPatrol patrol = CreatePatrol(encounter.ID, eventPosition, eventPosition, encounter.PatrolRadius, patrolCleanupSeconds, encounter.Spawn);
			if (!patrol || !patrol.IsActive())
			{
				Log("Failed to spawn POI patrol for " + encounter.ID);
				continue;
			}

			AIPlusPOIEvent raidEvent = AIPlusPOIEvent.Create(encounter, building, patrol, eventPosition, locationName);
			m_ActivePOIEvents.Set(encounter.ID, raidEvent);
			SetCooldown(key, encounter.CooldownSeconds);

			if (encounter.SendGlobalNotice)
			{
				string text = string.Format(encounter.NoticeText, encounter.RaidType, locationName);
				SendGlobalNotice(encounter.NoticeTitle, text);
			}
		}
	}

	protected AIPlusBuildingInstance PickAvailablePOIBuilding(AIPlusPOIEncounter encounter, out string locationName)
	{
		array<ref AIPlusBuildingInstance> candidates = AIPlusPOIWorld.GetBuildings(encounter);
		Log("Picking POI building for " + encounter.ID + " from " + candidates.Count() + " candidates.");

		while (candidates.Count() > 0)
		{
			int index = Math.RandomInt(0, candidates.Count());
			AIPlusBuildingInstance building = candidates.Get(index);
			candidates.Remove(index);

			if (!building)
				continue;

			string candidateLocation = AIPlusLocationNames.GetNearestName(building.Position);
			if (IsPOILocationActive(candidateLocation))
			{
				Log("Skipping " + encounter.ID + " candidate at " + building.Position + " because " + candidateLocation + " already has an active event.");
				continue;
			}

			if (IsPOIPositionNearActiveEvent(building.Position, AIPlusConfig.Get().POIMinDistanceBetweenEvents))
			{
				Log("Skipping " + encounter.ID + " candidate at " + building.Position + " near " + candidateLocation + " because it is too close to another active POI event.");
				continue;
			}

			locationName = candidateLocation;
			Log("Selected " + encounter.ID + " POI at " + building.Position + " near " + locationName + ".");
			return building;
		}

		locationName = "";
		return null;
	}

	protected AIPlusPatrol CreatePatrol(string name, vector spawnPosition, vector lastKnownPosition, float patrolRadius, int durationSeconds, AIPlusSpawnProfile profile, TStringArray protectedUIDs = null)
	{
		string eventID = string.Format("%1:%2:%3", name, GetGame().GetTime(), Math.RandomInt(1000, 999999));
		AIPlusPatrol patrol = AIPlusPatrol.Create(name, spawnPosition, lastKnownPosition, patrolRadius, durationSeconds, profile, protectedUIDs, eventID);
		if (patrol)
			m_Patrols.Insert(patrol);

		return patrol;
	}

	protected void WarmupRandom()
	{
		Math.Randomize(-1);
		Math.RandomFloat01();
		Math.RandomFloat01();
		Math.RandomFloat01();
	}

	protected bool IsPOIEventActive(string id)
	{
		AIPlusPOIEvent raidEvent = m_ActivePOIEvents.Get(id);
		return raidEvent && raidEvent.IsActive();
	}

	protected bool IsPOILocationActive(string locationName)
	{
		if (locationName == "" || locationName == "an unmarked settlement")
			return false;

		foreach (string id, AIPlusPOIEvent raidEvent: m_ActivePOIEvents)
		{
			if (raidEvent && raidEvent.IsActive() && raidEvent.GetLocationName() == locationName)
				return true;
		}

		return false;
	}

	protected bool IsPOIPositionNearActiveEvent(vector position, float minDistance)
	{
		if (minDistance <= 0.0)
			return false;

		float minDistanceSq = minDistance * minDistance;
		foreach (string id, AIPlusPOIEvent raidEvent: m_ActivePOIEvents)
		{
			if (!raidEvent || !raidEvent.IsActive())
				continue;

			if (vector.DistanceSq(position, raidEvent.GetPosition()) <= minDistanceSq)
				return true;
		}

		return false;
	}

	protected bool FindSpawnPosition(vector targetPosition, float minDistance, float maxDistance, out vector spawnPosition)
	{
		for (int i = 0; i < 50; i++)
		{
			vector candidate = ExpansionMath.GetRandomPointInRing(targetPosition, minDistance, maxDistance);
			vector safeCandidate;

			if (!AIPlusSpawnSafety.GetSafeAIPosition(candidate, safeCandidate))
				continue;

			if (!DistanceInRange2D(targetPosition, safeCandidate, minDistance, maxDistance))
				continue;

			spawnPosition = safeCandidate;
			return true;
		}

		return false;
	}

	protected bool DistanceInRange2D(vector targetPosition, vector spawnPosition, float minDistance, float maxDistance)
	{
		float x = spawnPosition[0] - targetPosition[0];
		float z = spawnPosition[2] - targetPosition[2];
		float distanceSq = x * x + z * z;
		return distanceSq >= minDistance * minDistance && distanceSq <= maxDistance * maxDistance;
	}

	protected bool IsOnCooldown(string key)
	{
		if (!m_Cooldowns.Contains(key))
			return false;

		return m_Cooldowns.Get(key) > Now();
	}

	protected string GetPlayerID(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return "";

		string playerID = player.GetIdentity().GetPlainId();
		if (playerID == "")
			playerID = player.GetIdentity().GetId();

		return playerID;
	}

	protected void SetCooldown(string key, int seconds)
	{
		m_Cooldowns.Set(key, Now() + seconds);
	}

	protected int Now()
	{
		return GetGame().GetTime() / 1000;
	}

	protected bool Roll(float chance)
	{
		if (chance <= 0.0)
			return false;

		if (chance >= 1.0)
			return true;

		return Math.RandomFloat01() <= chance;
	}

	protected void CleanupPatrolRefs()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		for (int i = m_Patrols.Count() - 1; i >= 0; i--)
		{
			if (!m_Patrols[i] || !m_Patrols[i].IsActive())
				m_Patrols.Remove(i);
		}

		TStringArray inactiveEvents = {};
		foreach (string id, AIPlusPOIEvent raidEvent: m_ActivePOIEvents)
		{
			if (!raidEvent || !raidEvent.IsActive())
				inactiveEvents.Insert(id);
		}

		foreach (string inactiveID: inactiveEvents)
		{
			m_ActivePOIEvents.Remove(inactiveID);
		}
	}

	protected void SendPrivateNotice(string title, string text, PlayerIdentity identity)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (!identity)
			return;

		ExpansionNotification(title, text, EXPANSION_NOTIFICATION_ICON_INFO, COLOR_EXPANSION_NOTIFICATION_INFO, 10).Create(identity);
	}

	protected void SendGlobalNotice(string title, string text)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		ExpansionNotification(title, text, EXPANSION_NOTIFICATION_ICON_INFO, COLOR_EXPANSION_NOTIFICATION_INFO, 10).Create();
	}

	protected void Log(string message)
	{
		if (AIPlusConfig.Get().DebugLogging)
			Print("[AIPlusManager] " + message);
	}
}

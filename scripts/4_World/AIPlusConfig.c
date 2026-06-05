class AIPlusSpawnProfile
{
	int MinAI;
	int MaxAI;
	ref array<string> Loadouts;
	ref array<string> Factions;
	ref array<string> Units;
	string Formation;
	string Speed;
	string UnderThreatSpeed;
	float AccuracyMin;
	float AccuracyMax;
	float ThreatDistanceLimit;
	float LootableChance;
	bool UnlimitedReload;

	void AIPlusSpawnProfile()
	{
		MinAI = 3;
		MaxAI = 5;
		Loadouts = {"HumanLoadout"};
		Factions = {"Raiders"};
		Units = {};
		Formation = "Vee";
		Speed = "JOG";
		UnderThreatSpeed = "SPRINT";
		AccuracyMin = -1.0;
		AccuracyMax = -1.0;
		ThreatDistanceLimit = -1.0;
		LootableChance = 0.25;
		UnlimitedReload = false;
	}
}

class AIPlusPOIEncounter
{
	bool Enabled;
	string ID;
	string RaidType;
	ref array<string> BuildingClassNames;
	float Chance;
	int CooldownSeconds;
	int PatrolDurationSeconds;
	float PatrolRadius;
	string ChestClassName;
	int RewardChestLifetimeSeconds;
	bool EnableRewardChestUnlockIndicator;
	float RewardChestUnlockIndicatorHeight;
	int ChestLootMin;
	int ChestLootMax;
	float ChestLootItemChance;
	ref array<string> ChestLootItems;
	bool CreateExpansionMarker;
	string MarkerName;
	string MarkerIcon;
	bool SendGlobalNotice;
	string NoticeTitle;
	string NoticeText;
	string CompletionNoticeTitle;
	string CompletionNoticeText;
	ref AIPlusSpawnProfile Spawn;

	void AIPlusPOIEncounter()
	{
		Enabled = true;
		ID = "";
		RaidType = "Mission";
		BuildingClassNames = {};
		Chance = 1.0;
		CooldownSeconds = 300;
		PatrolDurationSeconds = 300;
		PatrolRadius = 50.0;
		ChestClassName = "AIPlusRewardChest";
		RewardChestLifetimeSeconds = 900;
		EnableRewardChestUnlockIndicator = true;
		RewardChestUnlockIndicatorHeight = 1.1;
		ChestLootMin = 3;
		ChestLootMax = 3;
		ChestLootItemChance = 1.0;
		ChestLootItems = {"BandageDressing", "TacticalBaconCan", "AmmoBox_556x45_20Rnd", "AmmoBox_762x39_20Rnd", "M67Grenade", "Mag_STANAG_30Rnd", "Mag_AKM_30Rnd", "Morphine"};
		CreateExpansionMarker = true;
		MarkerName = "";
		MarkerIcon = "Territory";
		SendGlobalNotice = true;
		NoticeTitle = "Bandits spotted";
		NoticeText = "Bandits are looting the '%1' at '%2', take them out!";
		CompletionNoticeTitle = "Mission complete";
		CompletionNoticeText = "%1 mission near %2 is over.";
		Spawn = new AIPlusSpawnProfile();
	}
}

class AIPlusConfigData
{
	bool Enabled;
	bool DebugLogging;

	bool EnableCounterRaids;
	float CounterRaidChance;
	int CounterRaidCooldownSeconds;
	int CounterRaidPatrolDurationSeconds;
	float CounterRaidSpawnDistanceMin;
	float CounterRaidSpawnDistanceMax;
	float CounterRaidPatrolRadius;
	bool IgnoreTerritoryMemberDamage;
	bool CounterRaidProtectTerritoryMembers;
	bool SendCounterRaidNotice;
	string CounterRaidNoticeTitle;
	string CounterRaidNoticeText;
	ref AIPlusSpawnProfile CounterRaidSpawn;

	bool EnablePlayerHunts;
	int PlayerHuntCheckSeconds;
	float PlayerHuntChance;
	int PlayerHuntCooldownSeconds;
	int PlayerHuntPatrolDurationSeconds;
	float PlayerHuntSpawnDistanceMin;
	float PlayerHuntSpawnDistanceMax;
	float PlayerHuntPatrolRadius;
	string PlayerHuntNoticeTitle;
	string PlayerHuntNoticeText;
	ref AIPlusSpawnProfile PlayerHuntSpawn;

	bool EnablePOIEncounters;
	int POICheckSeconds;
	float POIMinDistanceBetweenEvents;
	ref array<ref AIPlusPOIEncounter> POIEncounters;

	void AIPlusConfigData()
	{
		Enabled = true;
		DebugLogging = false;

		EnableCounterRaids = true;
		CounterRaidChance = 1.0;
		CounterRaidCooldownSeconds = 300;
		CounterRaidPatrolDurationSeconds = 300;
		CounterRaidSpawnDistanceMin = 200.0;
		CounterRaidSpawnDistanceMax = 300.0;
		CounterRaidPatrolRadius = 50.0;
		IgnoreTerritoryMemberDamage = true;
		CounterRaidProtectTerritoryMembers = true;
		SendCounterRaidNotice = true;
		CounterRaidNoticeTitle = "Counter-raid response";
		CounterRaidNoticeText = "Bandits have noticed your raid and are closing in on your location.";
		CounterRaidSpawn = new AIPlusSpawnProfile();
		CounterRaidSpawn.MinAI = 3;
		CounterRaidSpawn.MaxAI = 6;
		CounterRaidSpawn.LootableChance = 0.0;

		EnablePlayerHunts = true;
		PlayerHuntCheckSeconds = 60;
		PlayerHuntChance = 1.0;
		PlayerHuntCooldownSeconds = 300;
		PlayerHuntPatrolDurationSeconds = 300;
		PlayerHuntSpawnDistanceMin = 200.0;
		PlayerHuntSpawnDistanceMax = 300.0;
		PlayerHuntPatrolRadius = 50.0;
		PlayerHuntNoticeTitle = "Bandits are hunting you.";
		PlayerHuntNoticeText = "You have been spotted by bandits. Run, hide or fight!";
		PlayerHuntSpawn = new AIPlusSpawnProfile();
		PlayerHuntSpawn.MinAI = 2;
		PlayerHuntSpawn.MaxAI = 4;
		PlayerHuntSpawn.LootableChance = 0.2;

		EnablePOIEncounters = true;
		POICheckSeconds = 60;
		POIMinDistanceBetweenEvents = 2500.0;
		POIEncounters = {};

		TStringArray policeBuildings = {"Land_City_PoliceStation", "Land_City_PoliceStation_Enoch"};
		TStringArray firestationBuildings = {"Land_City_FireStation", "Land_Mil_FireStation"};
		TStringArray hospitalBuildings = {"Land_City_Hospital"};

		POIEncounters.Insert(MakePOI("police", "Police", policeBuildings, 1.0, "Police mission", "Territory"));
		POIEncounters.Insert(MakePOI("firestation", "Firestation", firestationBuildings, 1.0, "Firestation mission", "Territory"));
		POIEncounters.Insert(MakePOI("hospital", "Hospital", hospitalBuildings, 1.0, "Hospital mission", "Territory"));
	}

	protected AIPlusPOIEncounter MakePOI(string id, string raidType, TStringArray buildingClassNames, float chance, string markerName, string markerIcon)
	{
		AIPlusPOIEncounter encounter = new AIPlusPOIEncounter();
		encounter.ID = id;
		encounter.RaidType = raidType;
		encounter.Chance = chance;
		encounter.BuildingClassNames = buildingClassNames;
		encounter.MarkerName = markerName;
		encounter.MarkerIcon = markerIcon;
		return encounter;
	}
}

class AIPlusConfig
{
	protected static ref AIPlusConfigData m_Config;

	static AIPlusConfigData Get()
	{
		if (!m_Config)
			Load();

		return m_Config;
	}

	static void Load()
	{
		m_Config = new AIPlusConfigData();

		if (!GetGame().IsServer())
			return;

		EnsureFolder();

		if (FileExist(GetFile()))
			JsonFileLoader<AIPlusConfigData>.JsonLoadFile(GetFile(), m_Config);

		Validate();
		Save();
	}

	static void Save()
	{
		if (!GetGame().IsServer())
			return;

		EnsureFolder();
		JsonFileLoader<AIPlusConfigData>.JsonSaveFile(GetFile(), m_Config);
	}

	protected static void EnsureFolder()
	{
		if (!FileExist("$profile:AIPlus"))
			MakeDirectory("$profile:AIPlus");
	}

	protected static string GetFile()
	{
		return "$profile:AIPlus/AIPlusConfig.json";
	}

	protected static void Validate()
	{
		if (!m_Config.CounterRaidSpawn)
			m_Config.CounterRaidSpawn = new AIPlusSpawnProfile();

		if (!m_Config.PlayerHuntSpawn)
			m_Config.PlayerHuntSpawn = new AIPlusSpawnProfile();

		ValidateSpawn(m_Config.CounterRaidSpawn);
		ValidateSpawn(m_Config.PlayerHuntSpawn);

		m_Config.CounterRaidChance = ClampChance(m_Config.CounterRaidChance);
		m_Config.CounterRaidCooldownSeconds = Math.Max(0, m_Config.CounterRaidCooldownSeconds);
		m_Config.CounterRaidPatrolDurationSeconds = Math.Max(60, m_Config.CounterRaidPatrolDurationSeconds);
		m_Config.CounterRaidSpawnDistanceMin = Math.Max(25.0, m_Config.CounterRaidSpawnDistanceMin);
		m_Config.CounterRaidSpawnDistanceMax = Math.Max(m_Config.CounterRaidSpawnDistanceMin, m_Config.CounterRaidSpawnDistanceMax);
		m_Config.CounterRaidPatrolRadius = Math.Max(5.0, m_Config.CounterRaidPatrolRadius);
		m_Config.CounterRaidNoticeTitle = EnsureString(m_Config.CounterRaidNoticeTitle, "Counter-raid response");
		m_Config.CounterRaidNoticeText = EnsureString(m_Config.CounterRaidNoticeText, "Bandits have noticed your raid and are closing in on your location.");

		m_Config.PlayerHuntCheckSeconds = Math.Max(60, m_Config.PlayerHuntCheckSeconds);
		m_Config.PlayerHuntChance = ClampChance(m_Config.PlayerHuntChance);
		m_Config.PlayerHuntCooldownSeconds = Math.Max(0, m_Config.PlayerHuntCooldownSeconds);
		m_Config.PlayerHuntPatrolDurationSeconds = Math.Max(60, m_Config.PlayerHuntPatrolDurationSeconds);
		m_Config.PlayerHuntSpawnDistanceMin = Math.Max(25.0, m_Config.PlayerHuntSpawnDistanceMin);
		m_Config.PlayerHuntSpawnDistanceMax = Math.Max(m_Config.PlayerHuntSpawnDistanceMin, m_Config.PlayerHuntSpawnDistanceMax);
		m_Config.PlayerHuntPatrolRadius = Math.Max(5.0, m_Config.PlayerHuntPatrolRadius);
		m_Config.PlayerHuntNoticeTitle = EnsureString(m_Config.PlayerHuntNoticeTitle, "Bandits are hunting you.");
		m_Config.PlayerHuntNoticeText = EnsureString(m_Config.PlayerHuntNoticeText, "You have been spotted by bandits. Run, hide or fight!");
		MigrateOldPlayerHuntNoticeDefaults();

		m_Config.POICheckSeconds = Math.Max(60, m_Config.POICheckSeconds);
		m_Config.POIMinDistanceBetweenEvents = Math.Max(0.0, m_Config.POIMinDistanceBetweenEvents);

		if (!m_Config.POIEncounters)
			m_Config.POIEncounters = {};

		foreach (AIPlusPOIEncounter encounter: m_Config.POIEncounters)
		{
			if (!encounter)
				continue;

			encounter.ID = EnsureString(encounter.ID, encounter.RaidType);
			encounter.RaidType = EnsureString(encounter.RaidType, "Mission");
			if (IsCoreRaidPOI(encounter.ID, encounter.RaidType))
				encounter.SendGlobalNotice = true;

			encounter.Chance = ClampChance(encounter.Chance);
			encounter.CooldownSeconds = Math.Max(0, encounter.CooldownSeconds);
			encounter.PatrolDurationSeconds = Math.Max(60, encounter.PatrolDurationSeconds);
			encounter.PatrolRadius = Math.Max(5.0, encounter.PatrolRadius);
			encounter.ChestClassName = EnsureRewardChestClass(encounter.ChestClassName);
			if (encounter.RewardChestLifetimeSeconds <= 0)
				encounter.RewardChestLifetimeSeconds = 900;
			else
				encounter.RewardChestLifetimeSeconds = Math.Max(60, encounter.RewardChestLifetimeSeconds);
			encounter.RewardChestUnlockIndicatorHeight = Math.Max(0.0, encounter.RewardChestUnlockIndicatorHeight);
			encounter.ChestLootMin = Math.Max(0, encounter.ChestLootMin);
			encounter.ChestLootMax = Math.Max(encounter.ChestLootMin, encounter.ChestLootMax);
			encounter.ChestLootItemChance = ClampChance(encounter.ChestLootItemChance);
			encounter.MarkerName = EnsureString(encounter.MarkerName, encounter.RaidType + " mission");
			encounter.MarkerIcon = EnsureString(encounter.MarkerIcon, "Territory");
			encounter.NoticeTitle = EnsureString(encounter.NoticeTitle, "Bandits spotted");
			encounter.NoticeText = EnsureString(encounter.NoticeText, "Bandits are looting the '%1' at '%2', take them out!");
			encounter.CompletionNoticeTitle = EnsureString(encounter.CompletionNoticeTitle, "Mission complete");
			encounter.CompletionNoticeText = EnsureString(encounter.CompletionNoticeText, "%1 mission near %2 is over.");
			MigrateOldPOINoticeDefaults(encounter);

			if (!encounter.BuildingClassNames)
				encounter.BuildingClassNames = {};

			if (!encounter.ChestLootItems)
				encounter.ChestLootItems = {};

			if (!encounter.Spawn)
				encounter.Spawn = new AIPlusSpawnProfile();

			ValidateSpawn(encounter.Spawn);
		}
	}

	protected static void ValidateSpawn(AIPlusSpawnProfile spawn)
	{
		spawn.MinAI = Math.Max(1, spawn.MinAI);
		spawn.MaxAI = Math.Max(spawn.MinAI, spawn.MaxAI);
		spawn.Formation = EnsureString(spawn.Formation, "Vee");
		spawn.Speed = EnsureString(spawn.Speed, "JOG");
		spawn.UnderThreatSpeed = EnsureString(spawn.UnderThreatSpeed, "SPRINT");
		spawn.LootableChance = ClampChance(spawn.LootableChance);

		if (!spawn.Loadouts || spawn.Loadouts.Count() == 0)
			spawn.Loadouts = {"HumanLoadout"};

		if (!spawn.Factions || spawn.Factions.Count() == 0)
			spawn.Factions = {"Raiders"};

		if (!spawn.Units)
			spawn.Units = {};
	}

	protected static void MigrateOldPlayerHuntNoticeDefaults()
	{
		if (m_Config.PlayerHuntNoticeTitle == "Bandits nearby")
			m_Config.PlayerHuntNoticeTitle = "Bandits are hunting you.";

		if (m_Config.PlayerHuntNoticeText == "Bandits have noticed you and are closing in on your location.")
			m_Config.PlayerHuntNoticeText = "You have been spotted by bandits. Run, hide or fight!";
	}

	protected static void MigrateOldPOINoticeDefaults(AIPlusPOIEncounter encounter)
	{
		if (!encounter)
			return;

		if (encounter.NoticeTitle == "Raiders sighted")
			encounter.NoticeTitle = "Bandits spotted";

		if (encounter.NoticeText == "%1 are raiding near %2.")
			encounter.NoticeText = "Bandits are looting the '%1' at '%2', take them out!";

		if (encounter.CompletionNoticeTitle == "Raiders eliminated")
			encounter.CompletionNoticeTitle = "Mission complete";

		if (encounter.CompletionNoticeText == "%1 raid near %2 is over.")
			encounter.CompletionNoticeText = "%1 mission near %2 is over.";

		if (encounter.MarkerName == "Police raid")
			encounter.MarkerName = "Police mission";
		else if (encounter.MarkerName == "Firestation raid")
			encounter.MarkerName = "Firestation mission";
		else if (encounter.MarkerName == "Hospital raid")
			encounter.MarkerName = "Hospital mission";
		else if (encounter.MarkerName == encounter.RaidType + " raid")
			encounter.MarkerName = encounter.RaidType + " mission";
	}

	protected static bool IsCoreRaidPOI(string id, string raidType)
	{
		id.ToLower();
		raidType.ToLower();

		return id.Contains("police") || id.Contains("fire") || id.Contains("hospital") || raidType.Contains("police") || raidType.Contains("fire") || raidType.Contains("hospital");
	}

	protected static string EnsureString(string value, string defaultValue)
	{
		value.TrimInPlace();

		if (value == "")
			return defaultValue;

		return value;
	}

	protected static string EnsureRewardChestClass(string className)
	{
		className = EnsureString(className, "AIPlusRewardChest");

		string lowerClassName = className;
		lowerClassName.ToLower();

		if (lowerClassName == "seachest")
			return "AIPlusRewardChest";

		return className;
	}

	protected static float ClampChance(float chance)
	{
		if (chance < 0.0)
			return 0.0;

		if (chance > 1.0)
			return 1.0;

		return chance;
	}
}

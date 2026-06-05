class AIPlusPOIEvent : Managed
{
	protected string m_ID;
	protected string m_EventID;
	protected ref AIPlusPOIEncounter m_Encounter;
	protected ref AIPlusPatrol m_Patrol;
	protected EntityAI m_Chest;
	protected string m_ChestEventID;
	protected vector m_Position;
	protected string m_LocationName;
	protected bool m_ChestSpawned;
	protected bool m_ChestUnlocked;
	protected bool m_ChestFilled;
	protected bool m_Active;

#ifdef EXPANSIONMODNAVIGATION
	protected ExpansionMarkerModule m_MarkerModule;
	protected ExpansionMarkerData m_ServerMarker;
#endif

	static AIPlusPOIEvent Create(AIPlusPOIEncounter encounter, AIPlusBuildingInstance building, AIPlusPatrol patrol, vector eventPosition, string locationName)
	{
		AIPlusPOIEvent raidEvent = new AIPlusPOIEvent();
		raidEvent.Setup(encounter, building, patrol, eventPosition, locationName);
		return raidEvent;
	}

	protected void Setup(AIPlusPOIEncounter encounter, AIPlusBuildingInstance building, AIPlusPatrol patrol, vector eventPosition, string locationName)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		m_Encounter = encounter;
		m_Patrol = patrol;
		m_ID = encounter.ID;
		if (m_Patrol)
			m_EventID = m_Patrol.GetEventID();

		if (m_EventID == "")
			m_EventID = string.Format("poi:%1:%2", encounter.ID, GetGame().GetTime());
		m_ChestEventID = m_EventID + ":reward";
		m_Position = eventPosition;
		m_LocationName = locationName;
		m_Active = true;

		SpawnChest(building);
		CreateMarker(m_Position);

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.CheckRewardUnlock, 5000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.EndMissionTimeout, encounter.PatrolDurationSeconds * 1000, false);
	}

	bool IsActive()
	{
		return m_Active;
	}

	string GetID()
	{
		return m_ID;
	}

	string GetLocationName()
	{
		return m_LocationName;
	}

	vector GetPosition()
	{
		return m_Position;
	}

	void End()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (!m_Active)
			return;

		m_Active = false;
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.CheckRewardUnlock);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.EndMissionTimeout);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.End);
		RemoveMarker();

		if (m_Patrol)
			m_Patrol.Despawn();

		AIPlusEventRegistry.UnregisterPOIChest(m_ID, m_Chest);
		AIPlusEventRegistry.Cleanup(m_ChestEventID);
		AIPlusEventRegistry.Cleanup(m_EventID);
		m_Chest = null;
	}

	protected void EndMissionTimeout()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (!m_Active || m_ChestUnlocked)
			return;

		Log("Mission timed out for " + m_ID + ". Cleaning up locked reward chest and AI.");
		End();
	}

	protected void CheckRewardUnlock()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (!m_Active || m_ChestUnlocked)
			return;

		if (!m_Patrol || m_Patrol.GetAliveCount() > 0)
			return;

		SendCompletionNotice();
		RemoveMarker();
		UnlockChest();
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.CheckRewardUnlock);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.EndMissionTimeout);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.End, m_Encounter.RewardChestLifetimeSeconds * 1000, false);
	}

	protected void SpawnChest(AIPlusBuildingInstance building)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (m_ChestSpawned)
			return;

		m_ChestSpawned = true;

		vector chestPosition = AIPlusPOIWorld.GetRandomAccessibleChestPosition(building);
		if (chestPosition == vector.Zero)
			chestPosition = building.Position;

		AIPlusEventRegistry.DeletePOIChest(m_ID);
		m_Chest = EntityAI.Cast(GetGame().CreateObjectEx(m_Encounter.ChestClassName, chestPosition, ECE_PLACE_ON_SURFACE));
		if (!m_Chest)
			return;

		m_Chest.SetOrientation(Vector(building.Yaw + Math.RandomFloatInclusive(-25.0, 25.0), 0, 0));
		m_Chest.SetTakeable(false);
		AIPlusEventRegistry.RegisterPOIChest(m_ID, m_ChestEventID, m_Chest);
		LockChest();
		Log("Spawned locked reward chest " + m_Chest.GetType() + " for " + m_ID + " at " + chestPosition + ".");
	}

	protected void UnlockChest()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (m_ChestUnlocked)
			return;

		if (!m_Chest)
			return;

		m_ChestUnlocked = true;
		m_Chest.SetTakeable(false);
		AIPlusEventRegistry.UnlockEventChest(m_Chest);
		AIPlusRewardChest rewardChest = AIPlusRewardChest.Cast(m_Chest);
		if (rewardChest)
			rewardChest.AIPlus_SetUnlocked(true, m_Encounter.EnableRewardChestUnlockIndicator, m_Encounter.RewardChestUnlockIndicatorHeight);

		m_Chest.GetInventory().UnlockInventory(HIDE_INV_FROM_SCRIPT);
		m_Chest.GetInventory().UnlockInventory(LOCK_FROM_SCRIPT);
		FillChest();
		AIPlusEventRegistry.Register(m_ChestEventID, m_Chest);
		Log("Unlocked reward chest for " + m_ID + ".");
	}

	protected void LockChest()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (!m_Chest)
			return;

		m_Chest.SetTakeable(false);
		AIPlusRewardChest rewardChest = AIPlusRewardChest.Cast(m_Chest);
		if (rewardChest)
			rewardChest.AIPlus_SetUnlocked(false, false);

		m_Chest.GetInventory().LockInventory(HIDE_INV_FROM_SCRIPT);
		m_Chest.GetInventory().LockInventory(LOCK_FROM_SCRIPT);
	}

	protected void FillChest()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (m_ChestFilled)
			return;

		m_ChestFilled = true;

		if (!m_Chest || !m_Encounter.ChestLootItems || m_Encounter.ChestLootItems.Count() == 0)
			return;

		int targetCount = Math.RandomIntInclusive(m_Encounter.ChestLootMin, m_Encounter.ChestLootMax);
		int createdCount;
		int attempts;
		int maxAttempts = targetCount * 10;
		if (maxAttempts < 25)
			maxAttempts = 25;

		while (createdCount < targetCount && attempts < maxAttempts)
		{
			attempts++;

			if (Math.RandomFloat01() > m_Encounter.ChestLootItemChance)
				continue;

			string itemName = m_Encounter.ChestLootItems.GetRandomElement();
			if (itemName != "")
			{
				EntityAI reward = m_Chest.GetInventory().CreateInInventory(itemName);
				if (reward)
				{
					createdCount++;
				}
				else
				{
					Log("Failed to create reward item " + itemName + " in " + m_Chest.GetType() + ".");
				}
			}
		}

		Log("Filled reward chest for " + m_ID + " with " + createdCount + " / " + targetCount + " items.");
	}

	protected void SendCompletionNotice()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (!m_Encounter.SendGlobalNotice)
			return;

		string text = string.Format(m_Encounter.CompletionNoticeText, m_Encounter.RaidType, m_LocationName);
		ExpansionNotification(m_Encounter.CompletionNoticeTitle, text, EXPANSION_NOTIFICATION_ICON_INFO, COLOR_EXPANSION_NOTIFICATION_INFO, 10).Create();
	}

	protected void CreateMarker(vector position)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

#ifdef EXPANSIONMODNAVIGATION
		if (!m_Encounter.CreateExpansionMarker)
			return;

		if (CF_Modules<ExpansionMarkerModule>.Get(m_MarkerModule))
			m_ServerMarker = m_MarkerModule.CreateServerMarker(m_Encounter.MarkerName, m_Encounter.MarkerIcon, position, ARGB(255, 235, 59, 90), false);
#endif
	}

	protected void RemoveMarker()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

#ifdef EXPANSIONMODNAVIGATION
		if (!m_MarkerModule || !m_ServerMarker)
			return;

		m_MarkerModule.RemoveServerMarker(m_ServerMarker.GetUID());
		m_ServerMarker = null;
#endif
	}

	protected void Log(string message)
	{
		if (AIPlusConfig.Get().DebugLogging)
			Print("[AIPlusPOIEvent] " + message);
	}
}

class AIPlusPatrol : Managed
{
	protected vector m_SpawnPosition;
	protected vector m_LastKnownPosition;
	protected float m_PatrolRadius;
	protected int m_DurationSeconds;
	protected ref AIPlusSpawnProfile m_Profile;
	protected ref eAIGroup m_Group;
	protected ref eAIFormation m_Formation;
	protected ref TStringArray m_ProtectedUIDs;
	protected string m_Name;
	protected string m_EventID;

	static AIPlusPatrol Create(string name, vector spawnPosition, vector lastKnownPosition, float patrolRadius, int durationSeconds, AIPlusSpawnProfile profile, TStringArray protectedUIDs = null, string eventID = "")
	{
		AIPlusPatrol patrol = new AIPlusPatrol();
		patrol.Setup(name, spawnPosition, lastKnownPosition, patrolRadius, durationSeconds, profile, protectedUIDs, eventID);
		return patrol;
	}

	protected void Setup(string name, vector spawnPosition, vector lastKnownPosition, float patrolRadius, int durationSeconds, AIPlusSpawnProfile profile, TStringArray protectedUIDs = null, string eventID = "")
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		m_Name = name;
		m_EventID = eventID;
		m_SpawnPosition = ExpansionAIPatrol.GetPlacementPosition(spawnPosition);
		m_LastKnownPosition = ExpansionAIPatrol.GetPlacementPosition(lastKnownPosition);
		m_PatrolRadius = Math.Max(5.0, patrolRadius);
		m_DurationSeconds = Math.Max(60, durationSeconds);
		m_Profile = profile;
		m_ProtectedUIDs = {};

		if (protectedUIDs)
		{
			foreach (string uid: protectedUIDs)
			{
				if (uid != "" && m_ProtectedUIDs.Find(uid) == -1)
					m_ProtectedUIDs.Insert(uid);
			}
		}

		Spawn();
		if (m_ProtectedUIDs.Count() > 0)
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.ProtectTerritoryMembers, 2000, true);

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.Despawn, m_DurationSeconds * 1000, false, false);
	}

	protected void Spawn()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (!m_Profile)
			return;

		eAIBase leader = SpawnAI(m_SpawnPosition);
		if (!leader)
			return;

		m_Group = leader.GetGroup();
		if (!m_Group)
			return;

		eAIFaction faction = CreateFaction();
		if (!faction)
			faction = new eAIFactionCivilian();

		m_Group.SetFaction(faction);
		m_Formation = eAIFormation.Create(m_Profile.Formation);
		if (!m_Formation)
			m_Formation = new eAIFormationVee();

		m_Group.SetFormation(m_Formation);
		m_Group.SetWaypointBehaviour(eAIWaypointBehavior.ALTERNATE);
		m_Group.SetName(m_Name);
		m_Group.ClearWaypoints();
		m_Group.m_BackTracking = false;
		m_Group.AddWaypoint(m_LastKnownPosition);

		for (int i = 0; i < 4; i++)
		{
			vector waypoint = ExpansionMath.GetRandomPointInRing(m_LastKnownPosition, 5.0, m_PatrolRadius);
			m_Group.AddWaypoint(ExpansionAIPatrol.GetPlacementPosition(waypoint));
		}

		int count = Math.RandomIntInclusive(m_Profile.MinAI, m_Profile.MaxAI);
		int spawnedCount = 1;
		for (int member = 1; member < count; member++)
		{
			eAIBase ai = SpawnAI(m_Formation.ToWorld(m_Formation.GetPosition(member)));
			if (ai)
			{
				ai.SetGroup(m_Group);
				spawnedCount++;
			}
		}

		Log("Spawned " + spawnedCount + " / " + count + " AI at " + m_SpawnPosition + " moving to " + m_LastKnownPosition);
	}

	protected eAIBase SpawnAI(vector position)
	{
		if (!GetGame() || !GetGame().IsServer())
			return null;

		vector safePosition;
		if (!AIPlusSpawnSafety.FindSafeAIPosition(position, safePosition))
		{
			Log("Skipped AI spawn because no safe land position was found near " + position + ".");
			return null;
		}

		position = safePosition;

		eAIBase ai;
		if (!Class.CastTo(ai, GetGame().CreateObject(GetRandomUnit(), position)))
			return null;

		ai.SetPosition(position);
		ExpansionHumanLoadout.Apply(ai, PickString(m_Profile.Loadouts, "HumanLoadout"), false);
		EquipGasZoneProtection(ai, position);
		AIPlusEventRegistry.Register(m_EventID, ai);
		ai.SetMovementSpeedLimits(GetMovementSpeed(m_Profile.Speed, eAIMovementSpeed.JOG), GetMovementSpeed(m_Profile.UnderThreatSpeed, eAIMovementSpeed.SPRINT));
		ai.Expansion_SetCanBeLooted(Roll(m_Profile.LootableChance));
		ai.eAI_SetUnlimitedReload(m_Profile.UnlimitedReload);
		ai.eAI_SetAccuracy(m_Profile.AccuracyMin, m_Profile.AccuracyMax);
		ai.eAI_SetThreatDistanceLimit(m_Profile.ThreatDistanceLimit);
		ai.eAI_SetLootingBehavior(eAILootingBehavior.NONE);
		ai.GetOnDeathStart().Insert(OnAIDeathStart);

		return ai;
	}

	protected void EquipGasZoneProtection(eAIBase ai, vector position)
	{
		if (!ai || !AIPlusContaminatedZones.IsPositionInside(position))
			return;

		EntityAI existingMask = ai.FindAttachmentBySlotName("Mask");
		if (existingMask && existingMask.IsInherited(MaskBase))
		{
			EnsureGasMaskFilter(existingMask);
			return;
		}

		if (existingMask)
			GetGame().ObjectDelete(existingMask);

		EntityAI mask = ai.GetInventory().CreateAttachmentEx("AirborneMask", InventorySlots.MASK);
		if (!mask)
		{
			EntityAI headgear = ai.FindAttachmentBySlotName("Headgear");
			if (headgear)
				GetGame().ObjectDelete(headgear);

			mask = ai.GetInventory().CreateAttachmentEx("AirborneMask", InventorySlots.MASK);
		}

		if (!mask)
		{
			Log("Failed to equip gas zone protection for AI at " + position);
			return;
		}

		EnsureGasMaskFilter(mask);
		AIPlusEventRegistry.Register(m_EventID, mask);
		Log("Equipped gas zone protection for AI at " + position);
	}

	protected void EnsureGasMaskFilter(EntityAI mask)
	{
		if (!mask || mask.FindAttachmentBySlotName("GasMaskFilter"))
			return;

		EntityAI filter = mask.GetInventory().CreateAttachment("GasMask_Filter");
		if (!filter)
			filter = mask.GetInventory().CreateInInventory("GasMask_Filter");

		if (filter)
		{
			ItemBase filterItem = ItemBase.Cast(filter);
			if (filterItem)
				filterItem.SetQuantity(filterItem.GetQuantityMax());

			AIPlusEventRegistry.Register(m_EventID, filter);
		}
	}

	protected void OnAIDeathStart(DayZPlayer player)
	{
		if (!player || !GetGame().IsServer())
			return;

		EntityAI itemInHands = player.GetHumanInventory().GetEntityInHands();
		if (!itemInHands)
			return;

		AIPlusEventRegistry.Register(m_EventID, itemInHands);
		MoveDeathItemToBody(player, itemInHands);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.MoveDeathItemToBody, 250, false, player, itemInHands);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.MoveDeathItemToBody, 1000, false, player, itemInHands);
	}

	protected void MoveDeathItemToBody(DayZPlayer player, EntityAI item)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (!player || !item)
			return;

		if (item.GetHierarchyRoot() == player && player.GetHumanInventory().GetEntityInHands() != item)
			return;

		player.ServerTakeEntityToInventory(FindInventoryLocationType.ANY_CARGO | FindInventoryLocationType.ATTACHMENT, item);
	}

	protected eAIFaction CreateFaction()
	{
		if (m_ProtectedUIDs && m_ProtectedUIDs.Count() > 0)
		{
			eAIFactionAIPlusCounterRaid counterRaidFaction = new eAIFactionAIPlusCounterRaid();
			counterRaidFaction.SetProtectedUIDs(m_ProtectedUIDs);
			return counterRaidFaction;
		}

		return eAIFaction.Create(PickString(m_Profile.Factions, "Raiders"));
	}

	protected void ProtectTerritoryMembers()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (!m_Group || !m_ProtectedUIDs || m_ProtectedUIDs.Count() == 0)
		{
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.ProtectTerritoryMembers);
			return;
		}

		array<Man> players = {};
		GetGame().GetPlayers(players);

		foreach (Man man: players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!IsProtectedPlayer(player))
				continue;

			eAIPlayerTargetInformation targetInfo = player.GetTargetInformation();
			for (int i = 0; i < m_Group.Count(); i++)
			{
				eAIBase ai = eAIBase.Cast(m_Group.GetMember(i));
				if (ai)
					targetInfo.RemoveAI(ai);
			}
		}
	}

	protected bool IsProtectedPlayer(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return false;

		return m_ProtectedUIDs.Find(player.GetIdentity().GetId()) > -1 || m_ProtectedUIDs.Find(player.GetIdentity().GetPlainId()) > -1;
	}

	void Despawn(bool deferDespawnUntilLoosingAggro = false)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.ProtectTerritoryMembers);

		RegisterCurrentGroupInventory();

		if (m_Group)
		{
			m_Group.ClearAI(true, deferDespawnUntilLoosingAggro);
			m_Group = null;
		}

		AIPlusEventRegistry.Cleanup(m_EventID);
		Log("Despawned " + m_Name);
	}

	protected void RegisterCurrentGroupInventory()
	{
		if (!m_Group)
			return;

		for (int i = 0; i < m_Group.Count(); i++)
		{
			EntityAI member = EntityAI.Cast(m_Group.GetMember(i));
			if (member)
				AIPlusEventRegistry.Register(m_EventID, member);
		}
	}

	bool IsActive()
	{
		return m_Group != null;
	}

	string GetEventID()
	{
		return m_EventID;
	}

	int GetAliveCount()
	{
		if (!m_Group)
			return 0;

		int alive;
		for (int i = 0; i < m_Group.Count(); i++)
		{
			DayZPlayerImplement member = m_Group.GetMember(i);
			if (member && member.IsAlive())
				alive++;
		}

		return alive;
	}

	protected string GetRandomUnit()
	{
		if (m_Profile.Units && m_Profile.Units.Count() > 0)
			return m_Profile.Units.GetRandomElement();

		return eAISurvivor.GetQuasiRandom();
	}

	protected float GetMovementSpeed(string speed, int fallback)
	{
		int value = typename.StringToEnum(eAIMovementSpeed, speed);
		if (value == -1)
			return fallback;

		return value;
	}

	protected string PickString(array<string> values, string fallback)
	{
		if (!values || values.Count() == 0)
			return fallback;

		return values.GetRandomElement();
	}

	protected bool Roll(float chance)
	{
		if (chance <= 0.0)
			return false;

		if (chance >= 1.0)
			return true;

		return Math.RandomFloat01() <= chance;
	}

	protected void Log(string message)
	{
		if (AIPlusConfig.Get().DebugLogging)
			Print("[AIPlusPatrol] " + message);
	}
}

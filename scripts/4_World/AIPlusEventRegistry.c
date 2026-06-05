class AIPlusEventRegistry
{
	protected static ref map<string, ref array<EntityAI>> s_EventEntities;
	protected static ref map<string, EntityAI> s_POIChestsByKey;
	protected static ref array<EntityAI> s_EventChests;
	protected static ref array<EntityAI> s_UnlockedEventChests;

	protected static void Ensure()
	{
		if (!s_EventEntities)
			s_EventEntities = new map<string, ref array<EntityAI>>();

		if (!s_POIChestsByKey)
			s_POIChestsByKey = new map<string, EntityAI>();

		if (!s_EventChests)
			s_EventChests = {};

		if (!s_UnlockedEventChests)
			s_UnlockedEventChests = {};
	}

	static void Register(string eventID, EntityAI entity)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (eventID == "" || !entity)
			return;

		if (IsProtectedBaseBuildingEntity(entity))
		{
			Log("Refused to register protected base-building entity " + entity.GetType() + " for event cleanup.");
			return;
		}

		Ensure();

		array<EntityAI> entities = s_EventEntities.Get(eventID);
		if (!entities)
		{
			entities = {};
			s_EventEntities.Set(eventID, entities);
		}

		RegisterOnly(entities, entity);
		RegisterInventory(eventID, entity);
	}

	static void RegisterPOIChest(string poiKey, string eventID, EntityAI chest)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (poiKey == "" || !chest)
			return;

		Ensure();

		s_POIChestsByKey.Set(poiKey, chest);
		RegisterEventChest(chest, false);
		Register(eventID, chest);
	}

	static void DeletePOIChest(string poiKey)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (poiKey == "")
			return;

		Ensure();

		EntityAI oldChest = s_POIChestsByKey.Get(poiKey);
		if (!oldChest)
			return;

		s_POIChestsByKey.Remove(poiKey);
		UnregisterEventChest(oldChest);
		Log("Deleting stale POI reward chest " + oldChest.GetType() + " for " + poiKey + " at " + oldChest.GetPosition() + ".");
		SafeDelete(oldChest);
	}

	static void UnlockEventChest(EntityAI chest)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		RegisterEventChest(chest, true);
	}

	static bool IsEventChest(EntityAI chest)
	{
		Ensure();
		return chest && s_EventChests.Find(chest) > -1;
	}

	static bool IsEventChestUnlocked(EntityAI chest)
	{
		Ensure();
		return chest && s_UnlockedEventChests.Find(chest) > -1;
	}

	static bool IsRegisteredEntity(EntityAI entity)
	{
		if (!entity || !s_EventEntities)
			return false;

		foreach (string eventID, array<EntityAI> entities: s_EventEntities)
		{
			if (entities && entities.Find(entity) > -1)
				return true;
		}

		return false;
	}

	static void UnregisterPOIChest(string poiKey, EntityAI chest)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (poiKey == "" || !s_POIChestsByKey)
			return;

		EntityAI activeChest = s_POIChestsByKey.Get(poiKey);
		if (!activeChest || activeChest == chest)
			s_POIChestsByKey.Remove(poiKey);

		UnregisterEventChest(chest);
	}

	static void RegisterInventory(string eventID, EntityAI entity)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (eventID == "" || !entity || !entity.GetInventory())
			return;

		Ensure();

		array<EntityAI> entities = s_EventEntities.Get(eventID);
		if (!entities)
		{
			entities = {};
			s_EventEntities.Set(eventID, entities);
		}

		array<EntityAI> inventoryItems = {};
		entity.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, inventoryItems);
		foreach (EntityAI item: inventoryItems)
		{
			if (item && item != entity)
				RegisterOnly(entities, item);
		}
	}

	static void Cleanup(string eventID)
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		if (eventID == "" || !s_EventEntities)
			return;

		array<EntityAI> entities = s_EventEntities.Get(eventID);
		if (!entities)
			return;

		for (int i = entities.Count() - 1; i >= 0; i--)
		{
			EntityAI entity = entities[i];
			if (entity && !IsInsidePlayerInventory(entity))
			{
				UnregisterEventChest(entity);
				SafeDelete(entity);
			}
		}

		s_EventEntities.Remove(eventID);
	}

	protected static void RegisterOnly(array<EntityAI> entities, EntityAI entity)
	{
		if (IsProtectedBaseBuildingEntity(entity))
		{
			Log("Refused to track protected base-building entity " + entity.GetType() + " for event cleanup.");
			return;
		}

		if (entities.Find(entity) == -1)
			entities.Insert(entity);
	}

	protected static void SafeDelete(EntityAI entity)
	{
		if (!entity)
			return;

		if (IsProtectedBaseBuildingEntity(entity))
		{
			Log("Skipped cleanup delete for protected base-building entity " + entity.GetType() + " at " + entity.GetPosition() + ".");
			return;
		}

		GetGame().ObjectDelete(entity);
	}

	protected static bool IsProtectedBaseBuildingEntity(EntityAI entity)
	{
		if (!entity)
			return false;

		if (entity.IsKindOf("BaseBuildingBase") || entity.IsKindOf("ExpansionBaseBuilding") || entity.IsKindOf("ExpansionBaseBuildingBase") || entity.IsKindOf("ExpansionSafeBase"))
			return true;

		string type = entity.GetType();
		type.ToLower();

		if (type.IndexOf("expansion") == -1)
			return false;

		return type.IndexOf("basebuilding") > -1 || type.IndexOf("wall") > -1 || type.IndexOf("floor") > -1 || type.IndexOf("door") > -1 || type.IndexOf("gate") > -1 || type.IndexOf("stair") > -1 || type.IndexOf("ramp") > -1 || type.IndexOf("roof") > -1 || type.IndexOf("window") > -1 || type.IndexOf("pillar") > -1 || type.IndexOf("foundation") > -1 || type.IndexOf("fence") > -1 || type.IndexOf("barrier") > -1 || type.IndexOf("hatch") > -1 || type.IndexOf("territory") > -1;
	}

	protected static void RegisterEventChest(EntityAI chest, bool unlocked)
	{
		if (!chest)
			return;

		Ensure();

		if (s_EventChests.Find(chest) == -1)
			s_EventChests.Insert(chest);

		if (unlocked && s_UnlockedEventChests.Find(chest) == -1)
			s_UnlockedEventChests.Insert(chest);
	}

	protected static void UnregisterEventChest(EntityAI chest)
	{
		if (!chest)
			return;

		if (s_EventChests)
		{
			int chestIndex = s_EventChests.Find(chest);
			if (chestIndex > -1)
				s_EventChests.Remove(chestIndex);
		}

		if (s_UnlockedEventChests)
		{
			int unlockedIndex = s_UnlockedEventChests.Find(chest);
			if (unlockedIndex > -1)
				s_UnlockedEventChests.Remove(unlockedIndex);
		}
	}

	protected static bool IsInsidePlayerInventory(EntityAI entity)
	{
		if (!entity)
			return false;

		Object root = entity.GetHierarchyRoot();
		return PlayerBase.Cast(root) != null;
	}

	protected static void Log(string message)
	{
		if (AIPlusConfig.Get().DebugLogging)
			Print("[AIPlusEventRegistry] " + message);
	}
}

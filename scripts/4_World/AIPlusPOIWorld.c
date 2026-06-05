class AIPlusBuildingInstance
{
	string ClassName;
	vector Position;
	float Yaw;

	void AIPlusBuildingInstance(string className, vector position, float yaw)
	{
		ClassName = className;
		Position = position;
		Yaw = yaw;
	}
}

class AIPlusPOIWorld
{
	protected static ref map<string, ref array<ref AIPlusBuildingInstance>> s_BuildingsByClass;
	protected static ref map<string, ref array<vector>> s_ProxyOffsetsByClass;
	protected static bool s_Loaded;

	static void Load(array<ref AIPlusPOIEncounter> encounters)
	{
		if (s_Loaded)
			return;

		s_Loaded = true;
		s_BuildingsByClass = new map<string, ref array<ref AIPlusBuildingInstance>>();
		s_ProxyOffsetsByClass = new map<string, ref array<vector>>();

		TStringArray wanted = {};
		foreach (AIPlusPOIEncounter encounter: encounters)
		{
			if (!encounter || !encounter.BuildingClassNames)
				continue;

			foreach (string className: encounter.BuildingClassNames)
			{
				if (className != "" && wanted.Find(className) == -1)
				{
					wanted.Insert(className);
					s_BuildingsByClass.Insert(className, new array<ref AIPlusBuildingInstance>());
					s_ProxyOffsetsByClass.Insert(className, new array<vector>());
				}
			}
		}

		if (wanted.Count() == 0)
			return;

		string worldName;
		GetGame().GetWorldName(worldName);
		worldName.ToLower();

		LoadMapGroupPositions("dz/worlds/" + worldName + "/ce/mapgrouppos.xml", wanted);
		LoadMapGroupProxies("dz/worlds/" + worldName + "/ce/mapgroupproto.xml", wanted);

		foreach (string wantedClassName: wanted)
		{
			array<ref AIPlusBuildingInstance> buildings = s_BuildingsByClass.Get(wantedClassName);
			array<vector> proxyOffsets = s_ProxyOffsetsByClass.Get(wantedClassName);

			int buildingCount;
			if (buildings)
				buildingCount = buildings.Count();

			int proxyCount;
			if (proxyOffsets)
				proxyCount = proxyOffsets.Count();

			Log("Loaded " + buildingCount + " positions and " + proxyCount + " loot proxies for " + wantedClassName + ".");
		}
	}

	static AIPlusBuildingInstance PickBuilding(AIPlusPOIEncounter encounter)
	{
		array<ref AIPlusBuildingInstance> candidates = GetBuildings(encounter);

		if (candidates.Count() == 0)
			return null;

		return candidates.GetRandomElement();
	}

	static array<ref AIPlusBuildingInstance> GetBuildings(AIPlusPOIEncounter encounter)
	{
		array<ref AIPlusBuildingInstance> candidates = {};
		if (!encounter || !encounter.BuildingClassNames)
			return candidates;

		foreach (string className: encounter.BuildingClassNames)
		{
			array<ref AIPlusBuildingInstance> buildings = s_BuildingsByClass.Get(className);
			if (!buildings)
				continue;

			foreach (AIPlusBuildingInstance building: buildings)
			{
				if (building)
					candidates.Insert(building);
			}
		}

		return candidates;
	}

	static vector GetRandomBuildingLootPosition(AIPlusBuildingInstance building, float fallbackRadius = 8.0)
	{
		if (!building)
			return vector.Zero;

		array<vector> offsets = s_ProxyOffsetsByClass.Get(building.ClassName);
		if (offsets && offsets.Count() > 0)
			return RotateOffset(building.Position, offsets.GetRandomElement(), building.Yaw);

		vector fallback = ExpansionMath.GetRandomPointInRing(building.Position, 1.0, fallbackRadius);
		return ExpansionAIPatrol.GetPlacementPosition(fallback);
	}

	static vector GetRandomAccessibleChestPosition(AIPlusBuildingInstance building)
	{
		if (!building)
			return vector.Zero;

		vector position;
		if (FindAccessibleGroundPosition(building.Position, 8.0, 22.0, position))
			return position;

		if (FindAccessibleGroundPosition(building.Position, 22.0, 36.0, position))
			return position;

		if (FindAccessibleGroundPosition(building.Position, 36.0, 55.0, position))
			return position;

		return ExpansionAIPatrol.GetPlacementPosition(building.Position);
	}

	protected static bool FindAccessibleGroundPosition(vector origin, float minRadius, float maxRadius, out vector position)
	{
		for (int i = 0; i < 60; i++)
		{
			vector candidate = ExpansionMath.GetRandomPointInRing(origin, minRadius, maxRadius);
			candidate[1] = GetGame().SurfaceY(candidate[0], candidate[2]);
			candidate = ExpansionAIPatrol.GetPlacementPosition(candidate);

			if (IsAccessibleChestPosition(candidate))
			{
				position = candidate;
				return true;
			}
		}

		return false;
	}

	protected static bool IsAccessibleChestPosition(vector position)
	{
		if (position == vector.Zero)
			return false;

		if (GetGame().GetWaterDepth(position) > 0)
			return false;

		array<Object> excludedObjects = {};
		array<Object> collidedObjects = {};
		vector collisionCenter = position + Vector(0, 0.7, 0);
		vector collisionSize = Vector(1.4, 1.4, 1.4);
		if (GetGame().IsBoxCollidingGeometry(collisionCenter, Vector(0, 0, 0), collisionSize, ObjIntersectView, ObjIntersectGeom, excludedObjects, collidedObjects))
			return false;

		return true;
	}

	protected static void LoadMapGroupPositions(string path, TStringArray wanted)
	{
		FileHandle handle = OpenFile(path, FileMode.READ);
		if (!handle)
			return;

		string line;
		while (FGets(handle, line) >= 0)
		{
			if (line.IndexOf("<group") == -1)
				continue;

			string className = GetAttribute(line, "name");
			if (wanted.Find(className) == -1)
				continue;

			string posText = GetAttribute(line, "pos");
			if (posText == "")
				continue;

			float yaw = GetAttribute(line, "a").ToFloat();
			array<ref AIPlusBuildingInstance> buildings = s_BuildingsByClass.Get(className);
			if (buildings)
				buildings.Insert(new AIPlusBuildingInstance(className, posText.ToVector(), yaw));
		}

		CloseFile(handle);
	}

	protected static void LoadMapGroupProxies(string path, TStringArray wanted)
	{
		FileHandle handle = OpenFile(path, FileMode.READ);
		if (!handle)
			return;

		string activeClassName;
		string line;
		while (FGets(handle, line) >= 0)
		{
			if (line.IndexOf("<group") > -1)
			{
				string groupName = GetAttribute(line, "name");
				if (wanted.Find(groupName) > -1)
					activeClassName = groupName;
				else
					activeClassName = "";
			}
			else if (line.IndexOf("</group>") > -1)
			{
				activeClassName = "";
			}
			else if (activeClassName != "" && line.IndexOf("<proxy") > -1)
			{
				string posText = GetAttribute(line, "pos");
				if (posText == "")
					continue;

				array<vector> offsets = s_ProxyOffsetsByClass.Get(activeClassName);
				if (offsets)
					offsets.Insert(posText.ToVector());
			}
		}

		CloseFile(handle);
	}

	protected static vector RotateOffset(vector origin, vector offset, float yaw)
	{
		float angle = yaw * Math.DEG2RAD;
		float cos = Math.Cos(angle);
		float sin = Math.Sin(angle);

		float x = (offset[0] * cos) - (offset[2] * sin);
		float z = (offset[0] * sin) + (offset[2] * cos);
		return Vector(origin[0] + x, origin[1] + offset[1], origin[2] + z);
	}

	protected static string GetAttribute(string line, string attribute)
	{
		string pattern = attribute + "=\"";
		int start = line.IndexOf(pattern);
		if (start == -1)
			return "";

		start += pattern.Length();
		int end = line.IndexOfFrom(start, "\"");
		if (end == -1)
			return "";

		return line.Substring(start, end - start);
	}

	protected static void Log(string message)
	{
		if (AIPlusConfig.Get().DebugLogging)
			Print("[AIPlusPOIWorld] " + message);
	}
}

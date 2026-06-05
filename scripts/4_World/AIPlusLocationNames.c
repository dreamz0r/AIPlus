class AIPlusLocationName
{
	string Name;
	vector Position;
	float Weight;

	void AIPlusLocationName(string name, vector position, float weight)
	{
		Name = name;
		Position = position;
		Weight = weight;
	}
}

class AIPlusLocationNames
{
	protected static ref array<ref AIPlusLocationName> s_Locations;
	protected static bool s_Loaded;

	static string GetNearestName(vector position)
	{
		Load();

		AIPlusLocationName nearest;
		float bestScore = float.MAX;

		foreach (AIPlusLocationName location: s_Locations)
		{
			if (!location)
				continue;

			float dx = position[0] - location.Position[0];
			float dz = position[2] - location.Position[2];
			float distanceSq = dx * dx + dz * dz;
			float score = distanceSq;

			if (score < bestScore)
			{
				bestScore = score;
				nearest = location;
			}
		}

		if (nearest && nearest.Name != "")
			return nearest.Name;

		return "an unmarked settlement";
	}

	protected static void Load()
	{
		if (s_Loaded)
			return;

		s_Loaded = true;
		s_Locations = {};

		string worldName;
		GetGame().GetWorldName(worldName);

		string rootPath = "CfgWorlds " + worldName + " Names";
		if (!GetGame().ConfigIsExisting(rootPath))
			return;

		int count = GetGame().ConfigGetChildrenCount(rootPath);
		for (int i = 0; i < count; i++)
		{
			string childName;
			GetGame().ConfigGetChildName(rootPath, i, childName);

			string path = rootPath + " " + childName;
			string displayName;
			if (!GetGame().ConfigGetText(path + " name", displayName))
				GetGame().ConfigGetText(path + " text", displayName);

			displayName = CleanName(displayName, childName);
			if (displayName == "")
				continue;

			string typeName;
			GetGame().ConfigGetText(path + " type", typeName);

			float weight = GetTypeWeight(typeName);
			if (weight <= 0.0)
				continue;

			vector locationPosition = GetMapNamePosition(path + " position");
			if (locationPosition == vector.Zero)
				continue;

			s_Locations.Insert(new AIPlusLocationName(displayName, locationPosition, weight));
		}

		Log("Loaded " + s_Locations.Count() + " visible map names from " + rootPath + ".");
	}

	protected static vector GetMapNamePosition(string path)
	{
		TFloatArray values = new TFloatArray;
		GetGame().ConfigGetFloatArray(path, values);

		if (values.Count() >= 3)
			return Vector(values[0], values[1], values[2]);

		if (values.Count() >= 2)
			return Vector(values[0], 0, values[1]);

		return vector.Zero;
	}

	protected static string CleanName(string name, string fallbackName)
	{
		name.TrimInPlace();
		fallbackName.TrimInPlace();

		if (name.Length() > 0 && name.Substring(0, 1) == "#")
			name = Widget.TranslateString(name);

		name.TrimInPlace();
		if (name == "")
			name = CleanFallbackName(fallbackName);

		return name;
	}

	protected static string CleanFallbackName(string name)
	{
		name.Replace("Settlement_", "");
		name.Replace("Local_", "");
		name.Replace("Area_", "");
		name.Replace("_", " ");
		name.TrimInPlace();
		return name;
	}

	protected static float GetTypeWeight(string typeName)
	{
		if (typeName == "Capital" || typeName == "NameCityCapital")
			return 4.0;

		if (typeName == "City" || typeName == "NameCity")
			return 3.5;

		if (typeName == "Village" || typeName == "NameVillage")
			return 3.0;

		return 0.0;
	}

	protected static void Log(string message)
	{
		if (AIPlusConfig.Get().DebugLogging)
			Print("[AIPlusLocationNames] " + message);
	}
}

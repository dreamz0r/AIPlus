class AIPlusTerritory
{
	static ExpansionTerritory GetTerritoryAt(vector position)
	{
		set<TerritoryFlag> flags = TerritoryFlag.ExpansionGetAll();
		if (!flags)
			return null;

		float radius = GetTerritoryRadius();
		float radiusSq = radius * radius;
		float bestDistanceSq = 999999999.0;
		ExpansionTerritory bestTerritory;

		foreach (TerritoryFlag flag: flags)
		{
			if (!flag || !flag.HasExpansionTerritoryInformation())
				continue;

			ExpansionTerritory territory = flag.GetTerritory();
			if (!territory || territory.GetTerritoryID() < 0)
				continue;

			vector center = territory.GetPosition();
			if (center == vector.Zero)
				center = flag.GetPosition();

			float distanceSq = vector.DistanceSq(position, center);
			if (distanceSq > radiusSq || distanceSq >= bestDistanceSq)
				continue;

			bestDistanceSq = distanceSq;
			bestTerritory = territory;
		}

		return bestTerritory;
	}

	static bool IsMember(ExpansionTerritory territory, PlayerBase player)
	{
		if (!territory || !player || !player.GetIdentity())
			return false;

		return territory.IsMember(player.GetIdentity().GetId()) || territory.IsMember(player.GetIdentity().GetPlainId());
	}

	static TStringArray GetMemberUIDs(ExpansionTerritory territory)
	{
		TStringArray memberUIDs = {};
		if (!territory)
			return memberUIDs;

		array<ref ExpansionTerritoryMember> members = territory.GetTerritoryMembers();
		if (!members)
			return memberUIDs;

		foreach (ExpansionTerritoryMember member: members)
		{
			if (!member)
				continue;

			string uid = member.GetID();
			if (uid != "" && memberUIDs.Find(uid) == -1)
				memberUIDs.Insert(uid);
		}

		return memberUIDs;
	}

	protected static float GetTerritoryRadius()
	{
		if (GetExpansionSettings() && GetExpansionSettings().GetTerritory())
			return Math.Max(25.0, GetExpansionSettings().GetTerritory().TerritorySize);

		return 150.0;
	}
}

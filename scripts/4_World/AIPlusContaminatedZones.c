class AIPlusContaminatedZones
{
	protected static ref array<EffectArea> s_Zones;

	protected static void Ensure()
	{
		if (!s_Zones)
			s_Zones = {};
	}

	static void Register(EffectArea zone)
	{
		if (!zone)
			return;

		Ensure();

		if (s_Zones.Find(zone) == -1)
			s_Zones.Insert(zone);
	}

	static void Unregister(EffectArea zone)
	{
		if (!s_Zones || !zone)
			return;

		int index = s_Zones.Find(zone);
		if (index > -1)
			s_Zones.Remove(index);
	}

	static bool IsPositionInside(vector position)
	{
		if (!s_Zones)
			return false;

		for (int i = s_Zones.Count() - 1; i >= 0; i--)
		{
			EffectArea zone = s_Zones[i];
			if (!zone)
			{
				s_Zones.Remove(i);
				continue;
			}

			if (IsPositionInsideZone(position, zone))
				return true;
		}

		return false;
	}

	protected static bool IsPositionInsideZone(vector position, EffectArea zone)
	{
		vector center = zone.GetPosition();
		float dx = position[0] - center[0];
		float dz = position[2] - center[2];
		float distanceSq = dx * dx + dz * dz;

		if (distanceSq > zone.m_Radius * zone.m_Radius)
			return false;

		if (position[1] < center[1] - zone.m_NegativeHeight)
			return false;

		if (position[1] > center[1] + zone.m_PositiveHeight)
			return false;

		return true;
	}
}

modded class EffectArea
{
	override void InitZone()
	{
		super.InitZone();
		AIPlusContaminatedZones.Register(this);
	}

	override void EEDelete(EntityAI parent)
	{
		AIPlusContaminatedZones.Unregister(this);
		super.EEDelete(parent);
	}
}

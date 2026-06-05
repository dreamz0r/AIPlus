[eAIRegisterFaction(eAIFactionAIPlusCounterRaid)]
class eAIFactionAIPlusCounterRaid : eAIFactionRaiders
{
	protected ref TStringArray m_ProtectedUIDs;

	void eAIFactionAIPlusCounterRaid()
	{
		m_ProtectedUIDs = {};
	}

	void SetProtectedUIDs(TStringArray protectedUIDs)
	{
		m_ProtectedUIDs.Clear();

		if (!protectedUIDs)
			return;

		foreach (string uid: protectedUIDs)
		{
			if (uid != "" && m_ProtectedUIDs.Find(uid) == -1)
				m_ProtectedUIDs.Insert(uid);
		}
	}

	override bool IsFriendlyEntity(EntityAI other, DayZPlayer factionMember = null)
	{
		Man man = Man.Cast(other);
		if (!man || !man.GetIdentity())
			return false;

		return IsProtectedUID(man.GetIdentity().GetId()) || IsProtectedUID(man.GetIdentity().GetPlainId());
	}

	protected bool IsProtectedUID(string uid)
	{
		return uid != "" && m_ProtectedUIDs && m_ProtectedUIDs.Find(uid) > -1;
	}
}

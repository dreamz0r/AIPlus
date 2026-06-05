modded class ExpansionActionDamageBaseBuilding
{
	override void OnFinishProgressServer(ActionData action_data)
	{
		super.OnFinishProgressServer(action_data);

		if (!action_data || !action_data.m_Player || !action_data.m_Target)
			return;

		Object target = action_data.m_Target.GetObject();
		if (!target)
			return;

		AIPlusManager.Get().OnCounterRaid(PlayerBase.Cast(action_data.m_Player), target);
	}
}

modded class BaseBuildingBase
{
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (AIPlusRaidHooks.ShouldBlockRaidersBaseDamage(source, this))
			return;

		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		AIPlusRaidHooks.TryTriggerCounterRaid(source, this);
	}
}

modded class ItemBase
{
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!AIPlusRaidHooks.IsExpansionBaseBuildingPart(this))
		{
			super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
			return;
		}

		if (AIPlusRaidHooks.ShouldBlockRaidersBaseDamage(source, this))
			return;

		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		AIPlusRaidHooks.TryTriggerCounterRaid(source, this);
	}
}

class AIPlusRaidHooks
{
	static void TryTriggerCounterRaid(EntityAI source, Object target)
	{
		if (!GetGame().IsServer() || !source || !target)
			return;

		PlayerBase player = PlayerBase.Cast(source);
		if (!player)
			player = PlayerBase.Cast(source.GetHierarchyRootPlayer());

		if (player)
			AIPlusManager.Get().OnCounterRaid(player, target);
	}

	static bool IsExpansionBaseBuildingPart(Object target)
	{
		if (!target)
			return false;

		string type = target.GetType();
		type.ToLower();

		if (type.IndexOf("expansion") == -1)
			return false;

		if (target.IsKindOf("ExpansionBaseBuilding") || target.IsKindOf("ExpansionBaseBuildingBase") || target.IsKindOf("ExpansionSafeBase"))
			return true;

		if (target.IsKindOf("BaseBuildingBase"))
			return true;

		return type.IndexOf("basebuilding") > -1 || type.IndexOf("wall") > -1 || type.IndexOf("floor") > -1 || type.IndexOf("door") > -1 || type.IndexOf("gate") > -1 || type.IndexOf("stair") > -1 || type.IndexOf("ramp") > -1 || type.IndexOf("roof") > -1 || type.IndexOf("window") > -1 || type.IndexOf("pillar") > -1 || type.IndexOf("foundation") > -1 || type.IndexOf("fence") > -1 || type.IndexOf("barrier") > -1 || type.IndexOf("hatch") > -1 || type.IndexOf("territory") > -1;
	}

	static bool ShouldBlockRaidersBaseDamage(EntityAI source, Object target)
	{
		if (!GetGame().IsServer() || !source || !target)
			return false;

		if (!IsExpansionBaseBuildingPart(target) && !target.IsKindOf("BaseBuildingBase"))
			return false;

		EntityAI sourceRoot = EntityAI.Cast(source.GetHierarchyRoot());
		if (AIPlusEventRegistry.IsRegisteredEntity(source) || AIPlusEventRegistry.IsRegisteredEntity(sourceRoot))
		{
			if (AIPlusConfig.Get().DebugLogging)
				Print("[AIPlusRaidHooks] Blocked event AI damage to base-building entity " + target.GetType() + " at " + target.GetPosition() + ".");

			return true;
		}

		return false;
	}
}

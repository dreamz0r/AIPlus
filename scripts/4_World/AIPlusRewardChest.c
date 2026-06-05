class AIPlusRewardChest extends SeaChest
{
	protected bool m_AIPlusUnlocked;
	protected bool m_AIPlusShowUnlockEffect;
	protected float m_AIPlusUnlockEffectHeight;
	protected Particle m_AIPlusUnlockParticle;
	protected EffectSound m_AIPlusUnlockLoopSound;
	protected EffectSound m_AIPlusUnlockStartSound;

	static protected const string AI_PLUS_FLARE_LOOP_SOUND = "roadflareLoop_SoundSet";
	static protected const string AI_PLUS_FLARE_START_SOUND = "roadflareTurnOn_SoundSet";

	void AIPlusRewardChest()
	{
		RegisterNetSyncVariableBool("m_AIPlusUnlocked");
		RegisterNetSyncVariableBool("m_AIPlusShowUnlockEffect");
		RegisterNetSyncVariableFloat("m_AIPlusUnlockEffectHeight", 0, 5, 2);
	}

	void AIPlus_SetUnlocked(bool unlocked, bool showUnlockEffect, float unlockEffectHeight = 1.1)
	{
		if (!AIPlus_IsServer())
			return;

		m_AIPlusUnlocked = unlocked;
		m_AIPlusShowUnlockEffect = unlocked && showUnlockEffect;
		m_AIPlusUnlockEffectHeight = unlockEffectHeight;

		if (GetGame())
			SetSynchDirty();

		UpdateUnlockEffect();
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		UpdateUnlockEffect();
	}

	override void EEDelete(EntityAI parent)
	{
		StopUnlockEffect();
		super.EEDelete(parent);
	}

	override bool IsTakeable()
	{
		return false;
	}

	override bool CanDisplayCargo()
	{
		if (!AIPlus_IsUnlocked())
			return false;

		return super.CanDisplayCargo();
	}

	override bool CanPutInCargo(EntityAI parent)
	{
		return false;
	}

	override bool CanRemoveFromCargo(EntityAI parent)
	{
		return false;
	}

	override bool CanPutIntoHands(EntityAI parent)
	{
		return false;
	}

	override bool CanReceiveItemIntoCargo(EntityAI item)
	{
		if (!AIPlus_IsUnlocked())
			return false;

		return super.CanReceiveItemIntoCargo(item);
	}

	override bool CanLoadItemIntoCargo(EntityAI item)
	{
		if (!AIPlus_IsUnlocked())
			return false;

		return super.CanLoadItemIntoCargo(item);
	}

	override bool CanReleaseCargo(EntityAI cargo)
	{
		if (!AIPlus_IsUnlocked())
			return false;

		return super.CanReleaseCargo(cargo);
	}

	override bool IsInventoryVisible()
	{
		if (!AIPlus_IsUnlocked())
			return false;

		return super.IsInventoryVisible();
	}

	protected bool AIPlus_IsUnlocked()
	{
		if (m_AIPlusUnlocked)
			return true;

		return AIPlus_IsServer() && AIPlusEventRegistry.IsEventChestUnlocked(this);
	}

	protected void UpdateUnlockEffect()
	{
		if (m_AIPlusShowUnlockEffect)
		{
			StartUnlockEffect();
			return;
		}

		StopUnlockEffect();
	}

	protected void StartUnlockEffect()
	{
		if (!AIPlus_CanPlayLocalEffects())
			return;

		if (!m_AIPlusUnlockParticle)
			m_AIPlusUnlockParticle = ParticleManager.GetInstance().PlayOnObject(ParticleList.ROADFLARE_BURNING_MAIN, this, Vector(0, m_AIPlusUnlockEffectHeight, 0));

		if (!m_AIPlusUnlockLoopSound)
		{
			PlaySoundSetLoop(m_AIPlusUnlockLoopSound, AI_PLUS_FLARE_LOOP_SOUND, 0.5, 0);
			PlaySoundSet(m_AIPlusUnlockStartSound, AI_PLUS_FLARE_START_SOUND, 0, 0);
		}
	}

	protected void StopUnlockEffect()
	{
		if (m_AIPlusUnlockParticle)
		{
			m_AIPlusUnlockParticle.StopParticle(StopParticleFlags.IMMEDIATE);
			m_AIPlusUnlockParticle = null;
		}

		if (m_AIPlusUnlockLoopSound)
		{
			StopSoundSet(m_AIPlusUnlockLoopSound);
			m_AIPlusUnlockLoopSound = null;
		}

		if (m_AIPlusUnlockStartSound)
		{
			SEffectManager.DestroyEffect(m_AIPlusUnlockStartSound);
			m_AIPlusUnlockStartSound = null;
		}
	}

	protected bool AIPlus_IsServer()
	{
		return GetGame() && GetGame().IsServer();
	}

	protected bool AIPlus_CanPlayLocalEffects()
	{
		if (!GetGame())
			return false;

		return !GetGame().IsMultiplayer() || !GetGame().IsServer();
	}
}

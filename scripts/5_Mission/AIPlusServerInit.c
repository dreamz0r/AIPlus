class AIPlusServerInit
{
	static void Init()
	{
		if (!GetGame() || !GetGame().IsServer())
			return;

		AIPlusConfig.Get();
		AIPlusManager.Get().Start();
	}
}

modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		AIPlusServerInit.Init();
	}
}

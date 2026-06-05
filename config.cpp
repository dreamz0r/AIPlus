class CfgPatches
{
	class AIPlus
	{
		requiredAddons[] = {
			"DZ_Data",
			"DZ_Scripts",
			"DZ_Gear_Camping",
			"DayZExpansion_Core_Scripts",
			"DayZExpansion_AI_Scripts",
			"DayZExpansion_BaseBuilding_Scripts"
		};
		units[] = {"AIPlusRewardChest"};
		weapons[] = {};
		requiredVersion = 0.1;
	};
};

class CfgVehicles
{
	class SeaChest;
	class AIPlusRewardChest: SeaChest
	{
		scope = 2;
		displayName = "AI Plus Reward Chest";
		descriptionShort = "A locked reward chest used by AI Plus missions.";
		model = "\DZ\gear\camping\sea_chest.p3d";
		debug_ItemCategory = 10;
		storageCategory = 10;
		hologramMaterial = "sea_chest";
		hologramMaterialPath = "dz\gear\camping\data";
		slopeTolerance = 0.40000001;
		yawPitchRollLimit[] = {45,45,45};
		weight = 10000;
		itemBehaviour = 0;
		itemSize[] = {10,10};
		carveNavmesh = 1;
		canBeDigged = 0;
		rotationFlags = 2;
		hiddenSelections[] = {"camoGround"};
		hiddenSelectionsTextures[] = {"\dz\gear\camping\data\sea_chest_co.paa"};
		class Cargo
		{
			itemsCargoSize[] = {10,10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
	};
};

class CfgMods
{
	class AIPlus
	{
		dir = "AI-Plus";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "AI Plus";
		credits = "Griffin";
		author = "Griffin";
		authorID = "0";
		version = "1.0";
		extra = 0;
		type = "mod";
		dependencies[] = {"Game","World","Mission"};
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"AI-Plus/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"AI-Plus/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"AI-Plus/scripts/5_Mission"};
			};
		};
	};
};

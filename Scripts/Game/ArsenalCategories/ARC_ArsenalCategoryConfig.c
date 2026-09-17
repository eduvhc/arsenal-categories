//! Ordered list of categories shown in the arsenal filter list.
//! Edit Configs/ArsenalCategories/ARC_ArsenalCategories.conf in Workbench to change them; if the
//! config cannot be loaded the built-in defaults below are used and a warning is logged.
[BaseContainerProps(configRoot: true)]
class ARC_ArsenalCategoryConfig
{
	static const ResourceName CONFIG_PATH = "{73AC35200B110147}Configs/ArsenalCategories/ARC_ArsenalCategories.conf";

	[Attribute("", UIWidgets.Object, "Categories in display order; first match wins")]
	protected ref array<ref ARC_ArsenalCategory> m_aCategories;

	//------------------------------------------------------------------------------------------------
	array<ref ARC_ArsenalCategory> GetCategories()
	{
		return m_aCategories;
	}

	//------------------------------------------------------------------------------------------------
	//! Load the shipped config, or fall back to the defaults so the filter list always works.
	static ARC_ArsenalCategoryConfig Load()
	{
		Resource resource = Resource.Load(CONFIG_PATH);
		if (resource && resource.IsValid())
		{
			ARC_ArsenalCategoryConfig config = ARC_ArsenalCategoryConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(resource.GetResource().ToBaseContainer()));
			if (config && config.m_aCategories && !config.m_aCategories.IsEmpty())
				return config;
		}

		Print("[ARC] Category config unavailable; using built-in defaults", LogLevel.WARNING);
		return CreateDefault();
	}

	//------------------------------------------------------------------------------------------------
	//! Same content as the shipped .conf, kept in script so a broken config never leaves the panel unfiltered.
	static ARC_ArsenalCategoryConfig CreateDefault()
	{
		ARC_ArsenalCategoryConfig config = new ARC_ArsenalCategoryConfig();
		config.m_aCategories = {};

		SCR_EArsenalItemMode weaponModes = SCR_EArsenalItemMode.WEAPON | SCR_EArsenalItemMode.WEAPON_VARIANTS;
		SCR_EArsenalItemType throwableTypes = SCR_EArsenalItemType.LETHAL_THROWABLE | SCR_EArsenalItemType.NON_LETHAL_THROWABLE;
		SCR_EArsenalItemType clothingTypes = SCR_EArsenalItemType.HEADWEAR | SCR_EArsenalItemType.TORSO | SCR_EArsenalItemType.LEGS
			| SCR_EArsenalItemType.FOOTWEAR | SCR_EArsenalItemType.HANDWEAR;
		SCR_EArsenalItemType carryTypes = SCR_EArsenalItemType.VEST_AND_WAIST | SCR_EArsenalItemType.BACKPACK | SCR_EArsenalItemType.RADIO_BACKPACK;
		SCR_EArsenalItemType launcherTypes = SCR_EArsenalItemType.ROCKET_LAUNCHER | SCR_EArsenalItemType.MORTARS;

		// The engine has no SMG type (mods tag them RIFLE), so they are picked out by prefab name first.
		array<string> smgNames = {"smg", "_mp5", "mpx", "ump", "vityaz", "pp19", "pp2000", "ppsh", "uzi", "vector", "p90", "mp7", "kedr", "bizon"};

		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Submachine Guns", SCR_EArsenalItemType.RIFLE, weaponModes, smgNames));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Assault Rifles", SCR_EArsenalItemType.RIFLE, weaponModes));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Sniper Rifles", SCR_EArsenalItemType.SNIPER_RIFLE, weaponModes));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Machine Guns", SCR_EArsenalItemType.MACHINE_GUN, weaponModes));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Pistols", SCR_EArsenalItemType.PISTOL, weaponModes));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Launchers", launcherTypes, weaponModes));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Ammunition", 0, SCR_EArsenalItemMode.AMMUNITION));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Attachments", 0, SCR_EArsenalItemMode.ATTACHMENT));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Throwables", throwableTypes, 0));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Explosives", SCR_EArsenalItemType.EXPLOSIVES, 0));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Clothing", clothingTypes, 0));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Vests and Backpacks", carryTypes, 0));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Medical", SCR_EArsenalItemType.HEAL, 0));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Equipment", SCR_EArsenalItemType.EQUIPMENT, 0));

		return config;
	}
}

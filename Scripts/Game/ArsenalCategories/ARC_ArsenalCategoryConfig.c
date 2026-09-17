//! Ordered list of categories shown in the arsenal filter list. Sources, first one that works wins:
//!   1. JSON pushed by the server (its $profile:ArsenalCategories/categories.json) - see ARC_PlayerController
//!   2. the local $profile:ArsenalCategories/categories.json (single player, testing)
//!   3. Configs/ArsenalCategories/ARC_ArsenalCategories.conf (edit in Workbench, overridable by mods)
//!   4. the built-in defaults below
[BaseContainerProps(configRoot: true)]
class ARC_ArsenalCategoryConfig
{
	static const ResourceName CONFIG_PATH = "{73AC35200B110147}Configs/ArsenalCategories/ARC_ArsenalCategories.conf";

	// Vanilla arsenal attribute icons, so the list looks native and no textures ship with the addon.
	static const ResourceName ICON_RIFLES = "{71648F15B3984B87}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_AssaultRifles.edds";
	static const ResourceName ICON_SNIPERS = "{A2B4C0BFBECE6400}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_SniperRifles.edds";
	static const ResourceName ICON_MACHINE_GUNS = "{A02EB3B80E276400}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_MachineGuns.edds";
	static const ResourceName ICON_PISTOLS = "{2EEBBBCA36DD775F}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Pistols.edds";
	static const ResourceName ICON_LAUNCHERS = "{10840233D666C940}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Launcher.edds";
	static const ResourceName ICON_MAGAZINES = "{A3D157619860A564}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Magazines.edds";
	static const ResourceName ICON_OPTICS = "{CB055708E982C0A5}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Optics.edds";
	static const ResourceName ICON_GRENADES = "{233A8BC0520B1B8B}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Grenades.edds";
	static const ResourceName ICON_EXPLOSIVES = "{91C4A9B5AD80D443}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Explosive.edds";
	static const ResourceName ICON_JACKETS = "{92245C15E122EDB2}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Jackets.edds";
	static const ResourceName ICON_VESTS = "{2DCA69EEB8628C06}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Vests.edds";
	static const ResourceName ICON_MEDICAL = "{CDF868B16669B7EA}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Medical.edds";
	static const ResourceName ICON_ACCESSORIES = "{CDF94F179A33CFB9}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Accessories.edds";

	[Attribute("", UIWidgets.Object, "Categories in display order; first match wins")]
	protected ref array<ref ARC_ArsenalCategory> m_aCategories;

	//! Show/hide rules; JSON only. Empty = every item the arsenal offers is shown.
	protected ref array<ref ARC_VisibilityRule> m_aVisibilityRules = {};

	//! prefab -> addon IDs, filled lazily; loading a container per item is not free.
	protected ref map<ResourceName, ref array<string>> m_mPrefabAddons = new map<ResourceName, ref array<string>>();

	protected static string s_sServerJson;
	protected static ref ARC_ArsenalCategoryConfig s_Active;

	//------------------------------------------------------------------------------------------------
	//! Called when the server has pushed its categories.json (on clients), or on the server itself.
	static void SetServerJson(string json)
	{
		s_sServerJson = json;
		s_Active = null;
	}

	//------------------------------------------------------------------------------------------------
	//! The configuration currently in force, loaded once and reused until a new server JSON arrives.
	static ARC_ArsenalCategoryConfig GetActive()
	{
		if (!s_Active)
			s_Active = Load();

		return s_Active;
	}

	//------------------------------------------------------------------------------------------------
	array<ref ARC_VisibilityRule> GetVisibilityRules()
	{
		return m_aVisibilityRules;
	}

	//------------------------------------------------------------------------------------------------
	void SetVisibilityRules(notnull array<ref ARC_VisibilityRule> rules)
	{
		m_aVisibilityRules.Copy(rules);
	}

	//------------------------------------------------------------------------------------------------
	bool HasVisibilityRules()
	{
		return !m_aVisibilityRules.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! \return false when a rule hides this item; items matching no rule are shown
	bool IsVisible(notnull SCR_ArsenalItem item)
	{
		if (m_aVisibilityRules.IsEmpty())
			return true;

		ResourceName prefab = item.GetItemResourceName();
		string lowerPath = prefab;
		lowerPath.ToLower();

		array<string> addons;
		if (!m_mPrefabAddons.Find(prefab, addons))
		{
			addons = SCR_AddonTool.GetResourceAddons(prefab);
			m_mPrefabAddons.Insert(prefab, addons);
		}

		foreach (ARC_VisibilityRule rule : m_aVisibilityRules)
		{
			if (rule && rule.Matches(item, lowerPath, addons))
				return rule.IsShow();
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	array<ref ARC_ArsenalCategory> GetCategories()
	{
		return m_aCategories;
	}

	//------------------------------------------------------------------------------------------------
	//! Load the shipped config, or fall back to the defaults so the filter list always works.
	static ARC_ArsenalCategoryConfig Load()
	{
		array<ref ARC_ArsenalCategory> jsonCategories = {};
		array<ref ARC_VisibilityRule> jsonRules = {};
		if (!s_sServerJson.IsEmpty() && ARC_CategoryJson.Parse(s_sServerJson, jsonCategories, jsonRules))
		{
			PrintFormat("[ARC] Using categories pushed by the server (%1 categories, %2 visibility rules)", jsonCategories.Count(), jsonRules.Count());
			return FromCategories(jsonCategories, jsonRules);
		}

		if (ARC_CategoryJson.LoadFile(ARC_CategoryJson.FILE_PATH, jsonCategories, jsonRules))
		{
			PrintFormat("[ARC] Using categories from %1 (%2 categories, %3 visibility rules)", ARC_CategoryJson.FILE_PATH, jsonCategories.Count(), jsonRules.Count());
			return FromCategories(jsonCategories, jsonRules);
		}

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
	static ARC_ArsenalCategoryConfig FromCategories(notnull array<ref ARC_ArsenalCategory> categories, array<ref ARC_VisibilityRule> rules = null)
	{
		ARC_ArsenalCategoryConfig config = new ARC_ArsenalCategoryConfig();
		config.m_aCategories = {};
		config.m_aCategories.Copy(categories);
		if (rules)
			config.SetVisibilityRules(rules);

		return config;
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

		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Submachine Guns", ICON_RIFLES, SCR_EArsenalItemType.RIFLE, weaponModes, smgNames));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Assault Rifles", ICON_RIFLES, SCR_EArsenalItemType.RIFLE, weaponModes));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Sniper Rifles", ICON_SNIPERS, SCR_EArsenalItemType.SNIPER_RIFLE, weaponModes));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Machine Guns", ICON_MACHINE_GUNS, SCR_EArsenalItemType.MACHINE_GUN, weaponModes));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Pistols", ICON_PISTOLS, SCR_EArsenalItemType.PISTOL, weaponModes));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Launchers", ICON_LAUNCHERS, launcherTypes, weaponModes));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Ammunition", ICON_MAGAZINES, 0, SCR_EArsenalItemMode.AMMUNITION));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Attachments", ICON_OPTICS, 0, SCR_EArsenalItemMode.ATTACHMENT));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Throwables", ICON_GRENADES, throwableTypes, 0));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Explosives", ICON_EXPLOSIVES, SCR_EArsenalItemType.EXPLOSIVES, 0));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Clothing", ICON_JACKETS, clothingTypes, 0));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Vests and Backpacks", ICON_VESTS, carryTypes, 0));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Medical", ICON_MEDICAL, SCR_EArsenalItemType.HEAL, 0));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Equipment", ICON_ACCESSORIES, SCR_EArsenalItemType.EQUIPMENT, 0));

		return config;
	}
}

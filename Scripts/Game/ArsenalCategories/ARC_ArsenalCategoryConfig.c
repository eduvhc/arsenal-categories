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

	[Attribute("1", UIWidgets.CheckBox, "Wide arsenal panel: own layout for the Vicinity panel with the categories inside it", category: "Layout")]
	protected bool m_bWidePanel;

	[Attribute("8", UIWidgets.Slider, "Item grid columns of the wide panel", "4 12 1", category: "Layout")]
	protected int m_iGridColumns;

	[Attribute("8", UIWidgets.Slider, "Item grid rows of the wide panel", "4 12 1", category: "Layout")]
	protected int m_iGridRows;

	[Attribute("11", UIWidgets.Slider, "Category buttons per column", "4 30 1", category: "Layout")]
	protected int m_iCategoriesPerColumn;

	[Attribute("200", UIWidgets.Slider, "Width of a category button (px)", "120 320 1", category: "Layout")]
	protected int m_iCategoryWidth;

	//! Layout in force; built from the JSON "layout" object or from the attributes above.
	protected ref ARC_LayoutSettings m_Layout;

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
	//! Never null: falls back to the defaults of ARC_LayoutSettings.
	ARC_LayoutSettings GetLayout()
	{
		if (!m_Layout)
			m_Layout = ARC_LayoutSettings.Create(m_bWidePanel, m_iGridColumns, m_iGridRows, m_iCategoriesPerColumn, m_iCategoryWidth);

		return m_Layout;
	}

	//------------------------------------------------------------------------------------------------
	//! Layout of the configuration currently in force (static convenience for the UI).
	static ARC_LayoutSettings GetActiveLayout()
	{
		ARC_ArsenalCategoryConfig config = GetActive();
		if (!config)
			return new ARC_LayoutSettings();

		return config.GetLayout();
	}

	//------------------------------------------------------------------------------------------------
	array<ref ARC_VisibilityRule> GetVisibilityRules()
	{
		return m_aVisibilityRules;
	}

	//------------------------------------------------------------------------------------------------
	void SetVisibilityRules(notnull array<ref ARC_VisibilityRule> rules)
	{
		m_aVisibilityRules.Clear();
		foreach (ARC_VisibilityRule rule : rules)
			m_aVisibilityRules.Insert(rule);
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
		ARC_LayoutSettings jsonLayout;
		if (!s_sServerJson.IsEmpty() && ARC_CategoryJson.Parse(s_sServerJson, jsonCategories, jsonRules, jsonLayout))
		{
			PrintFormat("[ARC] Using categories pushed by the server (%1 categories, %2 visibility rules)", jsonCategories.Count(), jsonRules.Count());
			return FromCategories(jsonCategories, jsonRules, jsonLayout);
		}

		if (ARC_CategoryJson.LoadFile(ARC_CategoryJson.FILE_PATH, jsonCategories, jsonRules, jsonLayout))
		{
			PrintFormat("[ARC] Using categories from %1 (%2 categories, %3 visibility rules)", ARC_CategoryJson.FILE_PATH, jsonCategories.Count(), jsonRules.Count());
			return FromCategories(jsonCategories, jsonRules, jsonLayout);
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
	static ARC_ArsenalCategoryConfig FromCategories(notnull array<ref ARC_ArsenalCategory> categories, array<ref ARC_VisibilityRule> rules = null, ARC_LayoutSettings layout = null)
	{
		ARC_ArsenalCategoryConfig config = new ARC_ArsenalCategoryConfig();
		config.m_aCategories = {};
		config.m_Layout = layout;
		foreach (ARC_ArsenalCategory category : categories)
			config.m_aCategories.Insert(category);
		if (rules)
			config.SetVisibilityRules(rules);

		return config;
	}

	//------------------------------------------------------------------------------------------------
	//! Same content as the shipped .conf, kept in script so a broken config never leaves the panel unfiltered.
	//! Order matters: an item lands in the first category it satisfies, so name-based sub-classes
	//! (SMGs, shotguns, optics, helmets, navigation...) sit above the broad type they are carved out of.
	//! Verified against every vanilla and RHS catalog entry: nothing lands in "Other".
	static ARC_ArsenalCategoryConfig CreateDefault()
	{
		ARC_ArsenalCategoryConfig config = new ARC_ArsenalCategoryConfig();
		config.m_aCategories = {};

		// Weapons: any mode except magazines/attachments. Requiring WEAPON would lose entries that mods
		// leave on the default mode (RHS M4A1 Block 1, for example). Gear: also not deployable parts.
		SCR_EArsenalItemMode notWeaponModes = SCR_EArsenalItemMode.AMMUNITION | SCR_EArsenalItemMode.ATTACHMENT;
		SCR_EArsenalItemMode notGearModes = notWeaponModes | SCR_EArsenalItemMode.SUPPORT_STATION;

		// no SMG type in the engine (mods tag them RIFLE): picked out by name
		array<string> submachineGunsNames = {"/smg", "_smg", "/mp5", "_mp5", "_mpx", "_ump", "vityaz", "pp19", "pp2000", "ppsh", "_uzi", "kriss", "_p90", "_mp7", "kedr", "bizon", "_mp40", "sten_", "thompson", "evo3", "_mac10", "_mac11"};
		Add(config, "Submachine Guns", ICON_RIFLES, SCR_EArsenalItemType.RIFLE, 0, submachineGunsNames, null, 0, notWeaponModes);

		// same trick as SMGs; empty (hidden) until a mod adds shotguns
		array<string> shotgunsNames = {"shotgun", "/m590", "_m590", "m1014", "_ks23", "saiga12", "saiga-12", "_spas", "benelli", "mossberg", "_870", "_aa12", "_toz", "vepr12"};
		Add(config, "Shotguns", ICON_RIFLES, SCR_EArsenalItemType.RIFLE, 0, shotgunsNames, null, 0, notWeaponModes);

		// stand-alone launchers only (GM94, M320, M79); rifles with M203/GP25 stay rifles
		array<string> grenadeLaunchersNames = {"/grenadelaunchers/", "/gl_", "gm94", "/m320", "_m320", "/m79", "_m79", "_rg6", "_mgl", "milkor"};
		Add(config, "Grenade Launchers", ICON_LAUNCHERS, SCR_EArsenalItemType.ROCKET_LAUNCHER | SCR_EArsenalItemType.RIFLE, 0, grenadeLaunchersNames, null, 0, notWeaponModes);

		Add(config, "Assault Rifles", ICON_RIFLES, SCR_EArsenalItemType.RIFLE, 0, null, null, 0, notWeaponModes);
		Add(config, "Sniper Rifles", ICON_SNIPERS, SCR_EArsenalItemType.SNIPER_RIFLE, 0, null, null, 0, notWeaponModes);
		Add(config, "Machine Guns", ICON_MACHINE_GUNS, SCR_EArsenalItemType.MACHINE_GUN, 0, null, null, 0, notWeaponModes);
		Add(config, "Pistols", ICON_PISTOLS, SCR_EArsenalItemType.PISTOL, 0, null, null, 0, notWeaponModes);
		// rocket launchers and mortar tubes; ballistic tables are tagged MORTARS and go to Navigation
		array<string> launchersExcludes = {"ballistictable", "balistictable"};
		Add(config, "Launchers", ICON_LAUNCHERS, SCR_EArsenalItemType.ROCKET_LAUNCHER | SCR_EArsenalItemType.MORTARS, 0, null, launchersExcludes, 0, notWeaponModes);

		// turret boxes, rocket pods, pylons
		Add(config, "Vehicle & Aircraft", ICON_LAUNCHERS, SCR_EArsenalItemType.VEHICLE | SCR_EArsenalItemType.HELICOPTER, 0);

		Add(config, "Rockets & Shells", ICON_LAUNCHERS, SCR_EArsenalItemType.ROCKET_LAUNCHER | SCR_EArsenalItemType.MORTARS, SCR_EArsenalItemMode.AMMUNITION);
		// UGL/GL rounds and flare cartridges: 40mm, VOG, GM94 magazines, 26.5mm flares
		array<string> launcherRoundsNames = {"40mm", "40x46", "40x53", "43x30", "vog25", "vog30", "/gm94/", "ammo_grenade_", "ammo_flare", "_gp25"};
		Add(config, "Launcher Rounds", ICON_GRENADES, 0, SCR_EArsenalItemMode.AMMUNITION, launcherRoundsNames);

		Add(config, "Magazines", ICON_MAGAZINES, 0, SCR_EArsenalItemMode.AMMUNITION);
		array<string> opticsNames = {"/optics/", "optic_", "_optic", "scope", "sight", "/magnifier", "_magnifier", "acog", "elcan", "eotech", "aimpoint", "_pso", "_1p", "kobra", "holo", "reddot", "_rds"};
		Add(config, "Optics", ICON_OPTICS, 0, SCR_EArsenalItemMode.ATTACHMENT, opticsNames);

		array<string> muzzleDevicesNames = {"/muzzle/", "suppressor", "silencer", "flashhider", "flash_hider", "compensator", "muzzlebrake", "muzzle_brake", "_brake", "_supr", "_sd."};
		Add(config, "Muzzle Devices", ICON_OPTICS, 0, SCR_EArsenalItemMode.ATTACHMENT, muzzleDevicesNames);

		array<string> lasersAndLightsNames = {"/lasers/", "/lights/", "laser", "flashlight", "weaponlight", "_light", "peq", "dbal", "perst", "surefire", "_lam", "klesch", "zenitco_2", "_2p", "_2d"};
		Add(config, "Lasers & Lights", ICON_OPTICS, 0, SCR_EArsenalItemMode.ATTACHMENT, lasersAndLightsNames);

		array<string> gripsAndBipodsNames = {"/grips/", "/bipods/", "grip", "bipod", "_rvg", "_afg", "handstop"};
		Add(config, "Grips & Bipods", ICON_OPTICS, 0, SCR_EArsenalItemMode.ATTACHMENT, gripsAndBipodsNames);

		// handguards, rail covers, bayonets, underbarrel launchers, stocks
		Add(config, "Attachments", ICON_OPTICS, 0, SCR_EArsenalItemMode.ATTACHMENT);

		Add(config, "Grenades", ICON_GRENADES, SCR_EArsenalItemType.LETHAL_THROWABLE, 0);
		// smoke, flashbang, chemlights
		Add(config, "Smokes & Signals", ICON_GRENADES, SCR_EArsenalItemType.NON_LETHAL_THROWABLE, 0);

		// mines, demolition charges, detonators
		Add(config, "Explosives", ICON_EXPLOSIVES, SCR_EArsenalItemType.EXPLOSIVES, 0);

		array<string> helmetsNames = {"helmet", "helm_", "/helm", "_ssh", "sph4", "zsh"};
		Add(config, "Helmets", ICON_JACKETS, SCR_EArsenalItemType.HEADWEAR, 0, helmetsNames);

		array<string> faceAndEyewearNames = {"/eyewear/", "eyewear", "balaclava", "/mask", "mask_", "gasmask", "goggle", "glasses", "headphone", "headset", "shemagh", "keffiyeh", "scarf", "facewear", "facewrap", "facecover", "_beard"};
		Add(config, "Face & Eyewear", ICON_JACKETS, SCR_EArsenalItemType.HEADWEAR, 0, faceAndEyewearNames);

		// caps, hats, berets, boonies: any headwear not caught above
		Add(config, "Headwear", ICON_JACKETS, SCR_EArsenalItemType.HEADWEAR, 0);

		Add(config, "Jackets & Shirts", ICON_JACKETS, SCR_EArsenalItemType.TORSO, 0);
		Add(config, "Trousers", ICON_JACKETS, SCR_EArsenalItemType.LEGS, 0);
		Add(config, "Boots & Gloves", ICON_JACKETS, SCR_EArsenalItemType.FOOTWEAR | SCR_EArsenalItemType.HANDWEAR, 0);
		Add(config, "Vests", ICON_VESTS, SCR_EArsenalItemType.VEST_AND_WAIST, 0);
		// RHS modular vest pouches and armour plates are tagged EQUIPMENT
		array<string> pouchesAndPlatesNames = {"/characters/vests/", "pouch", "armorplate", "armor_plate", "_plate_", "/plates/"};
		Add(config, "Pouches & Plates", ICON_VESTS, SCR_EArsenalItemType.EQUIPMENT, 0, pouchesAndPlatesNames, null, 0, SCR_EArsenalItemMode.SUPPORT_STATION);

		// manpack radios fall through to Radios
		array<string> backpacksExcludes = {"/radios/", "radio_"};
		Add(config, "Backpacks", ICON_VESTS, SCR_EArsenalItemType.BACKPACK | SCR_EArsenalItemType.RADIO_BACKPACK, 0, null, backpacksExcludes);

		// bandages, tourniquets, morphine, saline, medical kits
		Add(config, "Medical", ICON_MEDICAL, SCR_EArsenalItemType.HEAL, 0);

		// map, compass, GPS, watch, DAGR, ballistic tables
		array<string> navigationNames = {"/maps/", "map_", "/compass", "compass_", "/navigation/", "/gps", "gps_", "_gps", "dagr", "/watches/", "watch_", "ballistictable", "balistictable", "/orion/", "garmin", "foretrex"};
		Add(config, "Navigation", ICON_ACCESSORIES, 0, 0, navigationNames, null, 0, notGearModes);

		array<string> binocularsAndRangefindersNames = {"binocular", "rangefinder", "vector21", "lrf_", "_lrf", "spotting", "monocular"};
		Add(config, "Binoculars & Rangefinders", ICON_ACCESSORIES, 0, 0, binocularsAndRangefindersNames, null, 0, notGearModes);

		array<string> radiosNames = {"/radios/", "radio_", "_radio", "anprc", "/prc", "r187", "r148", "r107", "r168", "walkie"};
		Add(config, "Radios", ICON_ACCESSORIES, 0, 0, radiosNames, null, 0, SCR_EArsenalItemMode.AMMUNITION | SCR_EArsenalItemMode.SUPPORT_STATION);

		// goggles, mounts, counterweights, IR strobes and thermals
		array<string> nightVisionNames = {"/nightvision/", "/thermals/", "nvg", "pvs", "gpnvg", "1pn", "thermal", "counterweight", "nv_goggle", "nightvision", "strobe", "ir_"};
		Add(config, "Night Vision", ICON_ACCESSORIES, 0, 0, nightVisionNames, null, 0, notGearModes);

		array<string> flaresAndLightsNames = {"/weapons/flares/", "/flashlights/", "flashlight", "flare_", "chemlight", "glowstick", "xmaslights", "torch_"};
		Add(config, "Flares & Lights", ICON_ACCESSORIES, 0, 0, flaresAndLightsNames, null, 0, notGearModes);

		// repair and rearming kits, entrenching tools, mine flags, jerrycans
		array<string> toolsAndKitsNames = {"/kits/", "repairkit", "rearmingkit", "etool", "shovel", "toolkit", "wirecutter", "/demining/", "mineflag", "minedetector", "jerrycan", "/fuel/", "crowbar", "axe_", "_axe", "saw_"};
		Add(config, "Tools & Kits", ICON_ACCESSORIES, 0, 0, toolsAndKitsNames);

		// mortar and tripod parts, sandbags, barbed tape, ammo boxes
		Add(config, "Deployables", ICON_ACCESSORIES, 0, SCR_EArsenalItemMode.SUPPORT_STATION);

		array<string> patchesNames = {"/patches/", "patch_", "insignia", "armband"};
		Add(config, "Patches", ICON_ACCESSORIES, 0, 0, patchesNames);

		// everything else: personal belongings, mounts, misc accessories
		Add(config, "Equipment", ICON_ACCESSORIES, SCR_EArsenalItemType.EQUIPMENT, 0);

		return config;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Add(notnull ARC_ArsenalCategoryConfig config, string name, ResourceName icon, SCR_EArsenalItemType types, SCR_EArsenalItemMode modes, array<string> prefabContains = null, array<string> prefabExcludes = null, SCR_EArsenalItemType typesExclude = 0, SCR_EArsenalItemMode modesExclude = 0)
	{
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create(name, icon, types, modes, prefabContains, prefabExcludes, typesExclude, modesExclude));
	}
}

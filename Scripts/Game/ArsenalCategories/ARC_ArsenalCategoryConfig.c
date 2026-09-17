//! Ordered list of categories shown in the arsenal filter bar.
//! Edit Configs/ArsenalCategories/ARC_ArsenalCategories.conf in Workbench to change them; if the
//! config cannot be loaded the built-in defaults below are used and a warning is logged.
[BaseContainerProps(configRoot: true)]
class ARC_ArsenalCategoryConfig
{
	static const ResourceName CONFIG_PATH = "{73AC35200B110147}Configs/ArsenalCategories/ARC_ArsenalCategories.conf";

	// Vanilla arsenal attribute icons, so the bar looks native and no textures ship with the addon.
	static const ResourceName ICON_RIFLES = "{71648F15B3984B87}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_AssaultRifles.edds";
	static const ResourceName ICON_MAGAZINES = "{A3D157619860A564}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Magazines.edds";
	static const ResourceName ICON_OPTICS = "{CB055708E982C0A5}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Optics.edds";
	static const ResourceName ICON_GRENADES = "{233A8BC0520B1B8B}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Grenades.edds";
	static const ResourceName ICON_JACKETS = "{92245C15E122EDB2}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Jackets.edds";
	static const ResourceName ICON_VESTS = "{2DCA69EEB8628C06}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Vests.edds";
	static const ResourceName ICON_MEDICAL = "{CDF868B16669B7EA}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Medical.edds";
	static const ResourceName ICON_ACCESSORIES = "{CDF94F179A33CFB9}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_Accessories.edds";

	[Attribute("", UIWidgets.Object, "Categories in display order")]
	protected ref array<ref ARC_ArsenalCategory> m_aCategories;

	//------------------------------------------------------------------------------------------------
	array<ref ARC_ArsenalCategory> GetCategories()
	{
		return m_aCategories;
	}

	//------------------------------------------------------------------------------------------------
	//! Load the shipped config, or fall back to the defaults so the filter bar always works.
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

		SCR_EArsenalItemType weaponTypes = SCR_EArsenalItemType.RIFLE | SCR_EArsenalItemType.PISTOL | SCR_EArsenalItemType.MACHINE_GUN
			| SCR_EArsenalItemType.SNIPER_RIFLE | SCR_EArsenalItemType.ROCKET_LAUNCHER | SCR_EArsenalItemType.MORTARS;
		SCR_EArsenalItemMode weaponModes = SCR_EArsenalItemMode.WEAPON | SCR_EArsenalItemMode.WEAPON_VARIANTS;
		SCR_EArsenalItemType throwableTypes = SCR_EArsenalItemType.LETHAL_THROWABLE | SCR_EArsenalItemType.NON_LETHAL_THROWABLE | SCR_EArsenalItemType.EXPLOSIVES;
		SCR_EArsenalItemType clothingTypes = SCR_EArsenalItemType.HEADWEAR | SCR_EArsenalItemType.TORSO | SCR_EArsenalItemType.LEGS
			| SCR_EArsenalItemType.FOOTWEAR | SCR_EArsenalItemType.HANDWEAR;
		SCR_EArsenalItemType carryTypes = SCR_EArsenalItemType.VEST_AND_WAIST | SCR_EArsenalItemType.BACKPACK | SCR_EArsenalItemType.RADIO_BACKPACK;

		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Weapons", ICON_RIFLES, weaponTypes, weaponModes));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Ammunition", ICON_MAGAZINES, 0, SCR_EArsenalItemMode.AMMUNITION));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Attachments", ICON_OPTICS, 0, SCR_EArsenalItemMode.ATTACHMENT));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Throwables and Explosives", ICON_GRENADES, throwableTypes, 0));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Clothing", ICON_JACKETS, clothingTypes, 0));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Vests and Backpacks", ICON_VESTS, carryTypes, 0));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Medical", ICON_MEDICAL, SCR_EArsenalItemType.HEAL, 0));
		config.m_aCategories.Insert(ARC_ArsenalCategory.Create("Equipment", ICON_ACCESSORIES, SCR_EArsenalItemType.EQUIPMENT, SCR_EArsenalItemMode.DEFAULT | SCR_EArsenalItemMode.CONSUMABLE));

		return config;
	}
}

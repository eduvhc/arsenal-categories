//! One filter button in the arsenal panel: a name, an icon and the item types / modes it shows.
//! An item matches when its type is in m_eItemTypes (or the mask is 0) AND its mode is in
//! m_eItemModes (or the mask is 0). Magazines carry the type of their weapon but mode AMMUNITION,
//! so "Weapons" limits modes to WEAPON while "Ammo" limits modes to AMMUNITION with any type.
[BaseContainerProps(), BaseContainerCustomStringTitleField("m_sName")]
class ARC_ArsenalCategory
{
	[Attribute("", UIWidgets.EditBox, "Shown in the storage title while this category is selected")]
	protected string m_sName;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Icon texture (.edds). Leave empty to use an imageset entry instead", params: "edds imageset")]
	protected ResourceName m_sIcon;

	[Attribute("", UIWidgets.EditBox, "Image name inside m_sIcon when m_sIcon is an .imageset")]
	protected string m_sImageName;

	[Attribute("0", UIWidgets.Flags, "Arsenal item types included. 0 = any type", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType))]
	protected SCR_EArsenalItemType m_eItemTypes;

	[Attribute("0", UIWidgets.Flags, "Arsenal item modes included. 0 = any mode", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode))]
	protected SCR_EArsenalItemMode m_eItemModes;

	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetIcon()
	{
		return m_sIcon;
	}

	//------------------------------------------------------------------------------------------------
	string GetImageName()
	{
		return m_sImageName;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true when the arsenal item belongs to this category
	bool Matches(notnull SCR_ArsenalItem item)
	{
		if (m_eItemTypes != 0 && (item.GetItemType() & m_eItemTypes) == 0)
			return false;

		if (m_eItemModes != 0 && (item.GetItemMode() & m_eItemModes) == 0)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Script-side constructor used for the built-in defaults when no config is available.
	static ARC_ArsenalCategory Create(string name, ResourceName icon, SCR_EArsenalItemType types, SCR_EArsenalItemMode modes, string imageName = "")
	{
		ARC_ArsenalCategory category = new ARC_ArsenalCategory();
		category.m_sName = name;
		category.m_sIcon = icon;
		category.m_sImageName = imageName;
		category.m_eItemTypes = types;
		category.m_eItemModes = modes;
		return category;
	}
}

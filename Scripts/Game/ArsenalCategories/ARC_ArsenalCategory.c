//! One entry in the arsenal category list: a name plus the rules that put an item into it.
//! An item matches when ALL of the following hold (a mask of 0 / an empty list means "any"):
//!   - its arsenal type is in m_eItemTypes
//!   - its arsenal mode is in m_eItemModes
//!   - its prefab path contains one of m_aPrefabContains (case-insensitive)
//!   - its prefab path contains none of m_aPrefabExcludes
//! Categories are tested in config order and the first match wins, so put narrow rules (e.g.
//! "Submachine Guns" by prefab name) before broad ones ("Assault Rifles" by type RIFLE).
//! Magazines carry the type of their weapon but mode AMMUNITION, which is why weapon categories
//! restrict modes to WEAPON | WEAPON_VARIANTS while "Ammunition" restricts the mode only.
[BaseContainerProps(), BaseContainerCustomStringTitleField("m_sName")]
class ARC_ArsenalCategory
{
	[Attribute("", UIWidgets.EditBox, "Button caption")]
	protected string m_sName;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Button icon (.edds). Empty = no icon", params: "edds")]
	protected ResourceName m_sIcon;

	[Attribute("0", UIWidgets.Flags, "Arsenal item types included. 0 = any type", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType))]
	protected SCR_EArsenalItemType m_eItemTypes;

	[Attribute("0", UIWidgets.Flags, "Arsenal item modes included. 0 = any mode", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode))]
	protected SCR_EArsenalItemMode m_eItemModes;

	[Attribute("", UIWidgets.EditBox, "Prefab path must contain one of these (case-insensitive). Empty = any prefab")]
	protected ref array<string> m_aPrefabContains;

	[Attribute("", UIWidgets.EditBox, "Prefab path must contain none of these (case-insensitive)")]
	protected ref array<string> m_aPrefabExcludes;

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
	SCR_EArsenalItemType GetItemTypes()
	{
		return m_eItemTypes;
	}

	//------------------------------------------------------------------------------------------------
	SCR_EArsenalItemMode GetItemModes()
	{
		return m_eItemModes;
	}

	//------------------------------------------------------------------------------------------------
	array<string> GetPrefabContains()
	{
		return m_aPrefabContains;
	}

	//------------------------------------------------------------------------------------------------
	array<string> GetPrefabExcludes()
	{
		return m_aPrefabExcludes;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true when the arsenal item belongs to this category
	bool Matches(notnull SCR_ArsenalItem item)
	{
		if (m_eItemTypes != 0 && (item.GetItemType() & m_eItemTypes) == 0)
			return false;

		if (m_eItemModes != 0 && (item.GetItemMode() & m_eItemModes) == 0)
			return false;

		bool needsPath = (m_aPrefabContains && !m_aPrefabContains.IsEmpty()) || (m_aPrefabExcludes && !m_aPrefabExcludes.IsEmpty());
		if (!needsPath)
			return true;

		string path = item.GetItemResourceName();
		path.ToLower();

		if (m_aPrefabExcludes)
		{
			foreach (string excluded : m_aPrefabExcludes)
			{
				if (ContainsLower(path, excluded))
					return false;
			}
		}

		if (!m_aPrefabContains || m_aPrefabContains.IsEmpty())
			return true;

		foreach (string needle : m_aPrefabContains)
		{
			if (ContainsLower(path, needle))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Script-side constructor used for the built-in defaults when no config is available.
	static ARC_ArsenalCategory Create(string name, ResourceName icon, SCR_EArsenalItemType types, SCR_EArsenalItemMode modes, array<string> prefabContains = null, array<string> prefabExcludes = null)
	{
		ARC_ArsenalCategory category = new ARC_ArsenalCategory();
		category.m_sName = name;
		category.m_sIcon = icon;
		category.m_eItemTypes = types;
		category.m_eItemModes = modes;
		category.m_aPrefabContains = {};
		category.m_aPrefabExcludes = {};

		if (prefabContains)
			category.m_aPrefabContains.Copy(prefabContains);

		if (prefabExcludes)
			category.m_aPrefabExcludes.Copy(prefabExcludes);

		return category;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool ContainsLower(string lowerHaystack, string needle)
	{
		if (needle.IsEmpty())
			return false;

		string lowerNeedle = needle;
		lowerNeedle.ToLower();
		return lowerHaystack.Contains(lowerNeedle);
	}
}

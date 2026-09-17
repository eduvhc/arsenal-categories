//! One "show" or "hide" rule for arsenal items. Rules are evaluated in order and the first rule
//! whose conditions all hold decides; an item matching no rule is shown. A rule without any
//! condition matches everything, which makes "hide the rest" a one-liner at the end of the list.
//!
//! Conditions (empty = not checked):
//!   m_aAddons          item comes from one of these addon IDs (the ID field of addon.gproj,
//!                      e.g. ArmaReforger, RHS_Core, RHS_Content_01, NCMGPS)
//!   m_aAddonsExclude   item comes from none of these addons
//!   m_eItemTypes       arsenal item type mask
//!   m_eItemModes       arsenal item mode mask
//!   m_aPrefabContains  prefab path contains one of these (case-insensitive)
//!   m_aPrefabExcludes  prefab path contains none of these
class ARC_VisibilityRule
{
	protected bool m_bShow;
	protected ref array<string> m_aAddons = {};
	protected ref array<string> m_aAddonsExclude = {};
	protected SCR_EArsenalItemType m_eItemTypes;
	protected SCR_EArsenalItemMode m_eItemModes;
	protected ref array<string> m_aPrefabContains = {};
	protected ref array<string> m_aPrefabExcludes = {};

	//------------------------------------------------------------------------------------------------
	static ARC_VisibilityRule Create(bool show, array<string> addons, array<string> addonsExclude, SCR_EArsenalItemType types, SCR_EArsenalItemMode modes, array<string> prefabContains, array<string> prefabExcludes)
	{
		ARC_VisibilityRule rule = new ARC_VisibilityRule();
		rule.m_bShow = show;
		rule.m_eItemTypes = types;
		rule.m_eItemModes = modes;

		if (addons)
			rule.m_aAddons.Copy(addons);

		if (addonsExclude)
			rule.m_aAddonsExclude.Copy(addonsExclude);

		if (prefabContains)
			rule.m_aPrefabContains.Copy(prefabContains);

		if (prefabExcludes)
			rule.m_aPrefabExcludes.Copy(prefabExcludes);

		return rule;
	}

	//------------------------------------------------------------------------------------------------
	bool IsShow()
	{
		return m_bShow;
	}

	//------------------------------------------------------------------------------------------------
	array<string> GetAddons()
	{
		return m_aAddons;
	}

	//------------------------------------------------------------------------------------------------
	array<string> GetAddonsExclude()
	{
		return m_aAddonsExclude;
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
	//! \param item the arsenal entry
	//! \param lowerPath item prefab path, already lower-cased
	//! \param addons addon IDs that own or modify the prefab
	//! \return true when every condition of this rule holds
	bool Matches(notnull SCR_ArsenalItem item, string lowerPath, notnull array<string> addons)
	{
		if (m_eItemTypes != 0 && (item.GetItemType() & m_eItemTypes) == 0)
			return false;

		if (m_eItemModes != 0 && (item.GetItemMode() & m_eItemModes) == 0)
			return false;

		if (!m_aAddons.IsEmpty() && !ContainsAny(addons, m_aAddons))
			return false;

		if (!m_aAddonsExclude.IsEmpty() && ContainsAny(addons, m_aAddonsExclude))
			return false;

		foreach (string excluded : m_aPrefabExcludes)
		{
			if (ContainsLower(lowerPath, excluded))
				return false;
		}

		if (m_aPrefabContains.IsEmpty())
			return true;

		foreach (string needle : m_aPrefabContains)
		{
			if (ContainsLower(lowerPath, needle))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool ContainsAny(notnull array<string> haystack, notnull array<string> wanted)
	{
		foreach (string value : wanted)
		{
			if (haystack.Contains(value))
				return true;
		}

		return false;
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

//! Enforces the visibility rules of the active configuration on the item list every arsenal
//! consumer uses (UI grid, take action, resupply, displays). Runs on server and clients alike, so
//! a hidden item cannot be requested by a client that simply skips the UI.
modded class SCR_ArsenalComponent
{
	//------------------------------------------------------------------------------------------------
	override bool GetFilteredArsenalItems(out notnull array<SCR_ArsenalItem> filteredArsenalItems, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		bool result = super.GetFilteredArsenalItems(filteredArsenalItems, requiresDisplayType);
		if (!result)
			return false;

		ARC_ArsenalCategoryConfig config = ARC_ArsenalCategoryConfig.GetActive();
		if (!config || !config.HasVisibilityRules())
			return result;

		// New array rather than in-place removal: the source list may be owned by a catalog or config.
		array<SCR_ArsenalItem> visible = {};
		foreach (SCR_ArsenalItem item : filteredArsenalItems)
		{
			if (item && config.IsVisible(item))
				visible.Insert(item);
		}

		filteredArsenalItems = visible;
		return !filteredArsenalItems.IsEmpty();
	}
}

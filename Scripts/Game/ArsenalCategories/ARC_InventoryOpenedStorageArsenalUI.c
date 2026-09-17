//! Adds a category filter bar to the arsenal storage panel (the "Open Arsenal" grid).
//! Vanilla lists every available prefab in one grid; this keeps that behaviour under "All" and
//! narrows the grid to one category when a filter button is selected. Purely client-side UI:
//! nothing is sent to the server and the arsenal contents are untouched.
modded class SCR_InventoryOpenedStorageArsenalUI
{
	protected ref ARC_ArsenalFilterBar m_ARC_FilterBar;
	protected ref ARC_ArsenalCategoryConfig m_ARC_Config;
	protected int m_iARC_SelectedCategory = ARC_ArsenalFilterBar.ALL_INDEX;
	protected string m_sARC_StorageName;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		ARC_CreateFilterBar(w);
	}

	//------------------------------------------------------------------------------------------------
	override event void HandlerDeattached(Widget w)
	{
		if (m_ARC_FilterBar)
			m_ARC_FilterBar.m_OnCategoryChanged.Remove(ARC_OnCategoryChanged);

		m_ARC_FilterBar = null;

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	override protected void GetAllItems(out notnull array<IEntity> pItemsInStorage, BaseInventoryStorageComponent pStorage = null)
	{
		// Traversal into a nested storage or "All" selected: vanilla behaviour.
		if (pStorage || m_iARC_SelectedCategory == ARC_ArsenalFilterBar.ALL_INDEX)
		{
			super.GetAllItems(pItemsInStorage, pStorage);
			return;
		}

		array<SCR_ArsenalItem> arsenalItems = {};
		if (!ARC_GetArsenalItems(arsenalItems))
			return;

		ChimeraWorld world = GetGame().GetWorld();
		ItemPreviewManagerEntity previewManager = world.GetItemPreviewManager();
		if (!previewManager)
			return;

		foreach (SCR_ArsenalItem item : arsenalItems)
		{
			if (ARC_ItemCategory(item) == m_iARC_SelectedCategory)
				pItemsInStorage.Insert(previewManager.ResolvePreviewEntityForPrefab(item.GetItemResourceName()));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Build the bar once, listing only categories that actually contain something in this arsenal.
	protected void ARC_CreateFilterBar(Widget root)
	{
		if (m_ARC_FilterBar || !root)
			return;

		Widget host = root.FindAnyWidget("titleLayout");
		if (!host)
		{
			Print("[ARC] titleLayout not found in arsenal panel; filter bar disabled", LogLevel.WARNING);
			return;
		}

		Print("[ARC] Arsenal panel attached; building filter bar");

		m_ARC_Config = ARC_ArsenalCategoryConfig.Load();
		array<ref ARC_ArsenalCategory> categories = m_ARC_Config.GetCategories();

		array<SCR_ArsenalItem> arsenalItems = {};
		if (!ARC_GetArsenalItems(arsenalItems))
		{
			Print("[ARC] No arsenal items available yet; filter bar disabled", LogLevel.WARNING);
			return;
		}

		array<int> available = {};
		bool hasOther = false;
		foreach (SCR_ArsenalItem item : arsenalItems)
		{
			int category = ARC_ItemCategory(item);
			if (category == ARC_ArsenalFilterBar.OTHER_INDEX)
				hasOther = true;
			else if (!available.Contains(category))
				available.Insert(category);
		}

		int buttonCount = available.Count();
		if (hasOther)
			buttonCount++;

		PrintFormat("[ARC] %1 item(s), %2 category button(s)", arsenalItems.Count(), buttonCount);

		// One category (or none) gives the player nothing to choose; keep the panel untouched.
		if (buttonCount < 2)
			return;

		available.Sort();

		if (m_wStorageName)
			m_sARC_StorageName = m_wStorageName.GetText();

		m_ARC_FilterBar = new ARC_ArsenalFilterBar(host, categories, available, hasOther);
		if (!m_ARC_FilterBar.IsValid())
		{
			Print("[ARC] Filter bar widgets could not be created", LogLevel.WARNING);
			m_ARC_FilterBar = null;
			return;
		}

		m_ARC_FilterBar.m_OnCategoryChanged.Insert(ARC_OnCategoryChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void ARC_OnCategoryChanged(int categoryIndex)
	{
		if (categoryIndex == m_iARC_SelectedCategory)
			return;

		m_iARC_SelectedCategory = categoryIndex;
		Refresh();
		ARC_UpdateTitle();
	}

	//------------------------------------------------------------------------------------------------
	//! Show the active category and item count next to the storage name.
	protected void ARC_UpdateTitle()
	{
		if (!m_wStorageName)
			return;

		if (m_iARC_SelectedCategory == ARC_ArsenalFilterBar.ALL_INDEX)
		{
			m_wStorageName.SetText(m_sARC_StorageName);
			return;
		}

		string categoryName = "Other";
		if (m_iARC_SelectedCategory >= 0)
		{
			ARC_ArsenalCategory category = m_ARC_Config.GetCategories()[m_iARC_SelectedCategory];
			if (category)
				categoryName = category.GetName();
		}

		m_wStorageName.SetTextFormat("%1 - %2 (%3)", m_sARC_StorageName, categoryName, m_aSlots.Count());
	}

	//------------------------------------------------------------------------------------------------
	//! \return index of the first matching category, or OTHER_INDEX when none matches
	protected int ARC_ItemCategory(notnull SCR_ArsenalItem item)
	{
		if (!m_ARC_Config)
			return ARC_ArsenalFilterBar.OTHER_INDEX;

		array<ref ARC_ArsenalCategory> categories = m_ARC_Config.GetCategories();
		foreach (int i, ARC_ArsenalCategory category : categories)
		{
			if (category && category.Matches(item))
				return i;
		}

		return ARC_ArsenalFilterBar.OTHER_INDEX;
	}

	//------------------------------------------------------------------------------------------------
	//! Same source list vanilla uses for the grid, but keeping the SCR_ArsenalItem so type and mode are known.
	protected bool ARC_GetArsenalItems(out notnull array<SCR_ArsenalItem> arsenalItems)
	{
		if (!m_Storage || !m_Storage.GetOwner())
			return false;

		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(m_Storage.GetOwner().FindComponent(SCR_ArsenalComponent));
		if (!arsenalComponent)
			return false;

		return arsenalComponent.GetFilteredArsenalItems(arsenalItems);
	}
}

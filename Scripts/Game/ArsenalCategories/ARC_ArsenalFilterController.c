//! Glue shared by every inventory panel that can display an arsenal: owns the category config,
//! the selected category and the filter bar widgets, and produces the filtered preview-entity list
//! the panel puts into its grid. Panels stay thin: they call Sync() after the grid is (re)built and
//! GetItems() from their GetAllItems() override.
class ARC_ArsenalFilterController
{
	//! Invoked with no arguments when the player picks another category; the panel should Refresh().
	ref ScriptInvoker m_OnCategoryChanged = new ScriptInvoker();

	protected ref ARC_ArsenalCategoryConfig m_Config;
	protected ref ARC_ArsenalFilterBar m_FilterBar;
	protected int m_iSelectedCategory = ARC_ArsenalFilterBar.ALL_INDEX;
	protected BaseInventoryStorageComponent m_Storage;

	//------------------------------------------------------------------------------------------------
	void ARC_ArsenalFilterController()
	{
		m_Config = ARC_ArsenalCategoryConfig.Load();
	}

	//------------------------------------------------------------------------------------------------
	void ~ARC_ArsenalFilterController()
	{
		Destroy();
	}

	//------------------------------------------------------------------------------------------------
	//! \return true when a category other than "All" is selected
	bool IsFiltering()
	{
		return m_iSelectedCategory != ARC_ArsenalFilterBar.ALL_INDEX;
	}

	//------------------------------------------------------------------------------------------------
	int GetSelectedCategory()
	{
		return m_iSelectedCategory;
	}

	//------------------------------------------------------------------------------------------------
	//! Name of the selected category for titles; empty for "All".
	string GetSelectedCategoryName()
	{
		if (m_iSelectedCategory == ARC_ArsenalFilterBar.ALL_INDEX)
			return string.Empty;

		if (m_iSelectedCategory == ARC_ArsenalFilterBar.OTHER_INDEX)
			return "Other";

		ARC_ArsenalCategory category = m_Config.GetCategories()[m_iSelectedCategory];
		if (!category)
			return string.Empty;

		return category.GetName();
	}

	//------------------------------------------------------------------------------------------------
	//! \return the SCR_ArsenalComponent behind an arsenal storage, or null when the storage is not an arsenal
	static SCR_ArsenalComponent FindArsenal(BaseInventoryStorageComponent storage)
	{
		if (!storage)
			return null;

		IEntity owner = storage.GetOwner();
		if (!owner || !owner.FindComponent(SCR_ArsenalInventoryStorageManagerComponent))
			return null;

		return SCR_ArsenalComponent.Cast(owner.FindComponent(SCR_ArsenalComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Fill pItems with the preview entities of the selected category. Mirrors the vanilla arsenal
	//! branch of SCR_InventoryStorageBaseUI.GetAllItems() so costs, ranks and enabled types match.
	//! \return false when nothing could be produced (caller should fall back to vanilla)
	bool GetItems(notnull SCR_ArsenalComponent arsenal, out notnull array<IEntity> pItems)
	{
		array<SCR_ArsenalItem> arsenalItems = {};
		if (!arsenal.GetFilteredArsenalItems(arsenalItems))
			return false;

		ChimeraWorld world = GetGame().GetWorld();
		ItemPreviewManagerEntity previewManager = world.GetItemPreviewManager();
		if (!previewManager)
			return false;

		foreach (SCR_ArsenalItem item : arsenalItems)
		{
			if (CategoryOf(item) == m_iSelectedCategory)
				pItems.Insert(previewManager.ResolvePreviewEntityForPrefab(item.GetItemResourceName()));
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! (Re)build the filter bar under host for the given arsenal storage, or remove it when storage
	//! is not an arsenal. Call after the panel has rebuilt its grid so the bar ends up below any
	//! traverse title the panel created in the same host.
	void Sync(Widget host, BaseInventoryStorageComponent storage)
	{
		Destroy();

		SCR_ArsenalComponent arsenal = FindArsenal(storage);
		if (!arsenal || !host)
		{
			m_Storage = null;
			m_iSelectedCategory = ARC_ArsenalFilterBar.ALL_INDEX;
			return;
		}

		// Switching to a different arsenal starts from "All" again.
		if (storage != m_Storage)
		{
			m_Storage = storage;
			m_iSelectedCategory = ARC_ArsenalFilterBar.ALL_INDEX;
		}

		array<SCR_ArsenalItem> arsenalItems = {};
		if (!arsenal.GetFilteredArsenalItems(arsenalItems))
		{
			Print("[ARC] Arsenal has no items yet; filter bar not shown", LogLevel.WARNING);
			return;
		}

		array<int> available = {};
		bool hasOther = false;
		foreach (SCR_ArsenalItem item : arsenalItems)
		{
			int category = CategoryOf(item);
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

		m_FilterBar = new ARC_ArsenalFilterBar(host, m_Config.GetCategories(), available, hasOther);
		if (!m_FilterBar.IsValid())
		{
			Print("[ARC] Filter bar widgets could not be created", LogLevel.WARNING);
			m_FilterBar = null;
			return;
		}

		m_FilterBar.Select(m_iSelectedCategory, false);
		m_FilterBar.m_OnCategoryChanged.Insert(OnBarCategoryChanged);
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		if (!m_FilterBar)
			return;

		m_FilterBar.m_OnCategoryChanged.Remove(OnBarCategoryChanged);
		m_FilterBar = null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return index of the first matching category, or OTHER_INDEX when none matches
	protected int CategoryOf(notnull SCR_ArsenalItem item)
	{
		array<ref ARC_ArsenalCategory> categories = m_Config.GetCategories();
		foreach (int i, ARC_ArsenalCategory category : categories)
		{
			if (category && category.Matches(item))
				return i;
		}

		return ARC_ArsenalFilterBar.OTHER_INDEX;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBarCategoryChanged(int categoryIndex)
	{
		if (categoryIndex == m_iSelectedCategory)
			return;

		m_iSelectedCategory = categoryIndex;
		m_OnCategoryChanged.Invoke();
	}
}

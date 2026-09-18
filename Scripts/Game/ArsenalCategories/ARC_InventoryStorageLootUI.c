//! The "Vicinity" panel. Opening an arsenal from the world traverses this panel into the arsenal
//! storage, so this is where players normally see the flat item grid. The filter bar is rebuilt
//! after every FillItemsFromStorage().
modded class SCR_InventoryStorageLootUI
{
	protected ref ARC_ArsenalFilterController m_ARC_Filter;
	protected ref array<IEntity> m_aARC_PendingItems;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_ARC_Filter = new ARC_ArsenalFilterController();
		m_ARC_Filter.m_OnCategoryChanged.Insert(ARC_OnCategoryChanged);

		super.HandlerAttached(w);
	}

	//------------------------------------------------------------------------------------------------
	override event void HandlerDeattached(Widget w)
	{
		GetGame().GetCallqueue().Remove(ARC_SortAndShowChunk);

		if (m_ARC_Filter)
		{
			m_ARC_Filter.m_OnCategoryChanged.Remove(ARC_OnCategoryChanged);
			m_ARC_Filter.Destroy();
			m_ARC_Filter = null;
		}

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	//! The vanilla call must run even when a category is selected: it is what marks this panel as
	//! an arsenal (m_bIsArsenal, OnArsenalEnter), which the refund-on-drop flow and the arsenal
	//! refresh subscription depend on. The filtered list then replaces the full one.
	override protected void GetAllItems(out notnull array<IEntity> pItemsInStorage, BaseInventoryStorageComponent pStorage = null)
	{
		super.GetAllItems(pItemsInStorage, pStorage);

		if (!m_ARC_Filter || !m_ARC_Filter.IsFiltering())
			return;

		SCR_ArsenalComponent arsenal = ARC_ArsenalFilterController.FindArsenal(pStorage);
		if (!arsenal)
			return;

		array<IEntity> filtered = {};
		if (m_ARC_Filter.GetItems(arsenal, filtered))
			pItemsInStorage.Copy(filtered);
	}

	//------------------------------------------------------------------------------------------------
	override protected void FillItemsFromStorage(BaseInventoryStorageComponent storage)
	{
		super.FillItemsFromStorage(storage);

		if (m_ARC_Filter && m_widget)
			m_ARC_Filter.Sync(m_widget, ARC_GetMenuRoot(), GetCurrentNavigationStorage());
	}

	//------------------------------------------------------------------------------------------------
	override void Home()
	{
		super.Home();

		if (m_ARC_Filter)
			m_ARC_Filter.Sync(null, null, null);
	}

	//------------------------------------------------------------------------------------------------
	protected Widget ARC_GetMenuRoot()
	{
		SCR_InventoryMenuUI menu = GetInventoryMenuHandler();
		if (!menu)
			return null;

		return menu.GetRootWidget();
	}

	//------------------------------------------------------------------------------------------------
	//! Deferred one frame: the click arrives from a button the refresh is about to destroy.
	protected void ARC_OnCategoryChanged()
	{
		GetGame().GetCallqueue().CallLater(Refresh, 0, false);
	}
	//------------------------------------------------------------------------------------------------
	//! Arsenal listings are built in chunks: the first slotsPerFrame tiles now, the rest one chunk per
	//! frame, each chunk re-sorting and re-showing the current page. A 300-item RHS arsenal then
	//! opens on the first frame instead of after all tiles exist. Arsenal items are unique preview
	//! entities, so the stacking pass of the vanilla method is not needed for them; every other
	//! storage (ground loot, crates) keeps the vanilla path.
	override protected void UpdateOwnedSlots(notnull array<IEntity> pItemsInStorage)
	{
		GetGame().GetCallqueue().Remove(ARC_SortAndShowChunk);

		int perFrame = ARC_ArsenalCategoryConfig.GetActiveLayout().GetSlotsPerFrame();
		if (perFrame <= 0 || pItemsInStorage.Count() <= perFrame || !ARC_ArsenalFilterController.FindArsenal(GetCurrentNavigationStorage()))
		{
			super.UpdateOwnedSlots(pItemsInStorage);
			return;
		}

		DeleteSlots();

		m_aARC_PendingItems = {};
		m_aARC_PendingItems.Copy(pItemsInStorage);
		ARC_CreateSlotChunk(0, perFrame);
	}

	//------------------------------------------------------------------------------------------------
	protected void ARC_CreateSlotChunk(int from, int amount)
	{
		if (!m_aARC_PendingItems || !m_wGrid)
			return;

		int end = Math.ClampInt(from + amount, 0, m_aARC_PendingItems.Count());
		for (int i = from; i < end; i++)
		{
			IEntity item = m_aARC_PendingItems[i];
			if (!item)
				continue;

			InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
			if (!itemComponent)
				continue;

			SCR_InventorySlotUI slot = CreateSlotUI(itemComponent);
			if (slot)
				m_aSlots.Insert(slot);
		}

		if (end >= m_aARC_PendingItems.Count())
		{
			m_aARC_PendingItems = null;
			return;
		}

		// The caller sorts and shows the first chunk itself; later chunks do it here.
		GetGame().GetCallqueue().Call(ARC_SortAndShowChunk, end, amount);
	}

	//------------------------------------------------------------------------------------------------
	protected void ARC_SortAndShowChunk(int from, int amount)
	{
		ARC_CreateSlotChunk(from, amount);
		SortSlots();
		ShowPage(m_iLastShownPage);
	}
}

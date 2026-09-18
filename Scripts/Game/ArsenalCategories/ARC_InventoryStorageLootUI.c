//! The "Vicinity" panel. Opening an arsenal from the world traverses this panel into the arsenal
//! storage, so this is where players normally see the flat item grid. The filter bar is rebuilt
//! after every FillItemsFromStorage().
modded class SCR_InventoryStorageLootUI
{
	protected ref ARC_ArsenalFilterController m_ARC_Filter;

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
}

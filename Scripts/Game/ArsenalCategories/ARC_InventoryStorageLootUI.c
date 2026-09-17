//! The "Vicinity" panel. Opening an arsenal from the world traverses this panel into the arsenal
//! storage, so this is where players normally see the flat item grid. The filter bar is rebuilt
//! after every FillItemsFromStorage() so it sits right under the "Arsenal" traverse title.
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
	override protected void GetAllItems(out notnull array<IEntity> pItemsInStorage, BaseInventoryStorageComponent pStorage = null)
	{
		if (m_ARC_Filter && m_ARC_Filter.IsFiltering())
		{
			SCR_ArsenalComponent arsenal = ARC_ArsenalFilterController.FindArsenal(pStorage);
			if (arsenal && m_ARC_Filter.GetItems(arsenal, pItemsInStorage))
				return;
		}

		super.GetAllItems(pItemsInStorage, pStorage);
	}

	//------------------------------------------------------------------------------------------------
	override protected void FillItemsFromStorage(BaseInventoryStorageComponent storage)
	{
		super.FillItemsFromStorage(storage);

		if (m_ARC_Filter && m_widget)
			m_ARC_Filter.Sync(m_widget.FindAnyWidget("titleLayout"), GetCurrentNavigationStorage());
	}

	//------------------------------------------------------------------------------------------------
	override void Home()
	{
		super.Home();

		if (m_ARC_Filter)
			m_ARC_Filter.Sync(null, null);
	}

	//------------------------------------------------------------------------------------------------
	//! Deferred one frame: the click arrives from a button the refresh is about to destroy.
	protected void ARC_OnCategoryChanged()
	{
		GetGame().GetCallqueue().CallLater(Refresh, 0, false);
	}
}

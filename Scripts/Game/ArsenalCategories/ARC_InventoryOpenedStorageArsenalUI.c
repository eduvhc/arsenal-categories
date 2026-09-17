//! Arsenal opened as its own column (the "Open" action on a storage), as opposed to being browsed
//! through the Vicinity panel. Same controller, different refresh hooks.
modded class SCR_InventoryOpenedStorageArsenalUI
{
	protected ref ARC_ArsenalFilterController m_ARC_Filter;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_ARC_Filter = new ARC_ArsenalFilterController();
		m_ARC_Filter.m_OnCategoryChanged.Insert(ARC_OnCategoryChanged);

		super.HandlerAttached(w);

		ARC_Sync();
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
	override void Refresh()
	{
		super.Refresh();

		ARC_Sync();
	}

	//------------------------------------------------------------------------------------------------
	override protected void GetAllItems(out notnull array<IEntity> pItemsInStorage, BaseInventoryStorageComponent pStorage = null)
	{
		if (!pStorage && m_ARC_Filter && m_ARC_Filter.IsFiltering())
		{
			SCR_ArsenalComponent arsenal = ARC_ArsenalFilterController.FindArsenal(m_Storage);
			if (arsenal && m_ARC_Filter.GetItems(arsenal, pItemsInStorage))
				return;
		}

		super.GetAllItems(pItemsInStorage, pStorage);
	}

	//------------------------------------------------------------------------------------------------
	protected void ARC_Sync()
	{
		if (!m_ARC_Filter || !m_widget)
			return;

		// Only the root arsenal listing gets a bar; a nested storage shows its own contents.
		if (m_aTraverseStorage.Count() > 1)
		{
			m_ARC_Filter.Sync(null, null);
			return;
		}

		m_ARC_Filter.Sync(m_widget, m_Storage);
	}

	//------------------------------------------------------------------------------------------------
	//! Deferred one frame: the click arrives from a button the refresh is about to destroy.
	protected void ARC_OnCategoryChanged()
	{
		GetGame().GetCallqueue().CallLater(Refresh, 0, false);
	}
}

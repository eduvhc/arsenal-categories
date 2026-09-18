//! Swaps the Vicinity panel for the wide arsenal panel when the active layout asks for it.
//! Only this one panel gets the ARC layout (a copy of the vanilla InventoryContainerGrid with the
//! item grid wrapped next to a category grid), so backpacks, vests and crates keep the vanilla
//! layout untouched. Nothing is overridden by GUID; disabling "widePanel" in the JSON or .conf
//! brings the vanilla panel back without a restart.
modded class SCR_InventoryMenuUI
{
	static const ResourceName ARC_WIDE_STORAGE_LAYOUT = "{D696AC58ADB614A4}UI/layouts/Menus/Inventory/ARC_InventoryContainerGrid.layout";
	static const int ARC_COMPACT_ROWS = 4;

	//------------------------------------------------------------------------------------------------
	override void ShowVicinity(bool compact = false)
	{
		ARC_LayoutSettings layout = ARC_ArsenalCategoryConfig.GetActiveLayout();
		if (!layout.IsWidePanel())
		{
			super.ShowVicinity(compact);
			return;
		}

		if (!m_pVicinity)
		{
			Print("No vicnity component on character!", LogLevel.DEBUG);
			return;
		}

		if (m_wLootStorage)
		{
			m_wLootStorage.RemoveHandler(m_wLootStorage.FindHandler(SCR_InventoryStorageLootUI));
			m_wLootStorage.RemoveFromHierarchy();
		}

		Widget parent = m_widget.FindAnyWidget("StorageLootSlot");
		m_wLootStorage = GetGame().GetWorkspace().CreateWidgets(ARC_WIDE_STORAGE_LAYOUT, parent);
		if (!m_wLootStorage)
		{
			Print("[ARC] Wide arsenal layout could not be created; using the vanilla panel", LogLevel.WARNING);
			super.ShowVicinity(compact);
			return;
		}

		int rows = layout.GetRows();
		if (compact)
			rows = ARC_COMPACT_ROWS;

		m_wLootStorage.AddHandler(new SCR_InventoryStorageLootUI(null, null, this, 0, null, m_Player, rows, layout.GetColumns()));
		m_pStorageLootUI = SCR_InventoryStorageBaseUI.Cast(m_wLootStorage.FindHandler(SCR_InventoryStorageLootUI));
	}
}

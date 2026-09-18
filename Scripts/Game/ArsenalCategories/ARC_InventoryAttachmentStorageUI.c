//! The row of compatible items that opens under an attachment slot while inspecting a weapon.
//! Vanilla lists what the character carries; while an arsenal is being browsed this also lists
//! the arsenal's compatible attachments and magazines as buyable arsenal tiles (WCS-style), after
//! the carried ones. Hidden by the visibility rules = not offered here either.
modded class SCR_InventoryAttachmentStorageUI
{
	static const int ARC_ROWS = 1;

	//! Preview entities that stand for arsenal items, so CreateSlotUI can tell them apart.
	protected ref set<IEntity> m_ARC_ArsenalEntities = new set<IEntity>();
	protected IEntity m_ARC_ArsenalEntity;

	//------------------------------------------------------------------------------------------------
	void SCR_InventoryAttachmentStorageUI(BaseInventoryStorageComponent storage, LoadoutAreaType slotID = null, SCR_InventoryMenuUI menuManager = null, int iPage = 0, array<BaseInventoryStorageComponent> aTraverseStorage = null, InventorySearchPredicate searchPredicate = null)
	{
		ARC_Settings settings = ARC_ArsenalCategoryConfig.GetActiveLayout();
		if (!settings.IsArsenalAttachments())
			return;

		// Wider row: the arsenal usually has more than four options per slot.
		m_iMaxRows = ARC_ROWS;
		m_iMaxColumns = settings.GetColumns();
		m_iMatrix = new SCR_Matrix(m_iMaxColumns, m_iMaxRows);
		m_iPageSize = m_iMaxRows * m_iMaxColumns;
	}

	//------------------------------------------------------------------------------------------------
	override protected int GetRelevantItems(out notnull array<IEntity> items)
	{
		int count = super.GetRelevantItems(items);

		m_ARC_ArsenalEntities.Clear();
		m_ARC_ArsenalEntity = null;

		if (!m_SearchPredicate || !ARC_ArsenalCategoryConfig.GetActiveLayout().IsArsenalAttachments())
			return count;

		SCR_ArsenalComponent arsenal = ARC_FindBrowsedArsenal();
		if (!arsenal)
			return count;

		array<SCR_ArsenalItem> arsenalItems = {};
		if (!arsenal.GetFilteredArsenalItems(arsenalItems))
			return count;

		ChimeraWorld world = GetGame().GetWorld();
		ItemPreviewManagerEntity previewManager = world.GetItemPreviewManager();
		if (!previewManager)
			return count;

		m_ARC_ArsenalEntity = arsenal.GetOwner();

		// Only attachments and ammunition can ever match an attachment slot; skip the rest early.
		SCR_EArsenalItemMode candidateModes = SCR_EArsenalItemMode.ATTACHMENT | SCR_EArsenalItemMode.AMMUNITION;
		foreach (SCR_ArsenalItem arsenalItem : arsenalItems)
		{
			if (!arsenalItem)
				continue;

			if ((arsenalItem.GetItemMode() & candidateModes) == 0 && arsenalItem.GetItemType() != SCR_EArsenalItemType.WEAPON_ATTACHMENT)
				continue;

			IEntity preview = previewManager.ResolvePreviewEntityForPrefab(arsenalItem.GetItemResourceName());
			if (!preview || !m_SearchPredicate.ARC_Matches(preview))
				continue;

			items.Insert(preview);
			m_ARC_ArsenalEntities.Insert(preview);
			count++;
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	override protected SCR_InventorySlotUI CreateSlotUI(InventoryItemComponent pComponent, SCR_ItemAttributeCollection pAttributes = null)
	{
		if (!pComponent || !m_ARC_ArsenalEntity || !m_ARC_ArsenalEntities.Contains(pComponent.GetOwner()))
			return super.CreateSlotUI(pComponent, pAttributes);

		SCR_ArsenalInventorySlotUI slot = new SCR_ArsenalInventorySlotUI(pComponent, this, false, -1, null);
		slot.SetArsenalResourceComponent(SCR_ResourceComponent.FindResourceComponent(m_ARC_ArsenalEntity));
		SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Insert(slot, m_ARC_ArsenalEntity);
		return slot;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true when the given slot is one of the arsenal tiles of this row
	bool ARC_IsArsenalSlot(SCR_InventorySlotUI slot)
	{
		if (!slot || !slot.GetInventoryItemComponent())
			return false;

		return m_ARC_ArsenalEntities.Contains(slot.GetInventoryItemComponent().GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	//! The arsenal the Vicinity panel is currently browsing, if any.
	protected SCR_ArsenalComponent ARC_FindBrowsedArsenal()
	{
		if (!m_MenuHandler)
			return null;

		SCR_InventoryStorageBaseUI lootUI = m_MenuHandler.GetLootStorage();
		if (!lootUI)
			return null;

		return ARC_ArsenalFilterController.FindArsenal(lootUI.GetCurrentNavigationStorage());
	}
}

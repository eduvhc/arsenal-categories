//! Two things WCS does that vanilla does not, both behind settings:
//!  - ShowVicinity: the Vicinity panel becomes the wide arsenal panel (widePanel);
//!  - MoveBetweenFromVicinity_VirtualArsenal (right-click / Buy on an arsenal item): magazines go to
//!    a pouch instead of into the weapon, an occupied holster slot swaps weapons, and items the
//!    best-fit storage rejects fall back to equipment/deposit storages. Anything not covered here
//!    goes to super, so RHS and other mods of the same method keep their behaviour.
//!
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
		ARC_Settings layout = ARC_ArsenalCategoryConfig.GetActiveLayout();
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

	//------------------------------------------------------------------------------------------------
	override protected bool MoveBetweenFromVicinity_VirtualArsenal()
	{
		SCR_ArsenalInventorySlotUI arsenalSlot = SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI);
		if (!arsenalSlot)
			return super.MoveBetweenFromVicinity_VirtualArsenal();

		ARC_Settings settings = ARC_ArsenalCategoryConfig.GetActiveLayout();
		if (!settings.IsMagazinesToStorage() && !settings.IsWeaponSwap() && !settings.IsFallbackStorages())
			return super.MoveBetweenFromVicinity_VirtualArsenal();

		// Rank lock: vanilla also stops here, silently.
		if (!arsenalSlot.IsAvailable())
			return true;

		InventoryItemComponent itemComponent = arsenalSlot.GetInventoryItemComponent();
		if (!itemComponent || !itemComponent.GetOwner())
			return super.MoveBetweenFromVicinity_VirtualArsenal();

		IEntity item = itemComponent.GetOwner();
		IEntity arsenalEntity = SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Get(arsenalSlot);
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
		SCR_ResourcePlayerControllerInventoryComponent resourceInventory = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		if (!resourceComponent || !resourceInventory)
			return super.MoveBetweenFromVicinity_VirtualArsenal();

		ResourceName resourceName = arsenalSlot.GetItemResource();
		RplId resourceId = Replication.FindItemId(resourceComponent);

		// 1. Magazines: a pouch or backpack first, so the loaded magazine stays where it is.
		if (settings.IsMagazinesToStorage() && item.FindComponent(BaseMagazineComponent))
		{
			BaseInventoryStorageComponent deposit = m_InventoryManager.FindStorageForItem(item, EStoragePurpose.PURPOSE_DEPOSIT);
			if (deposit && m_InventoryManager.CanInsertItemInStorage(item, deposit))
			{
				resourceInventory.RpcAsk_ArsenalRequestItem(resourceId, Replication.FindItemId(deposit), resourceName, EResourceType.SUPPLIES);
				return true;
			}
		}

		// 2. Vanilla choice: best-fit storage of any purpose.
		BaseInventoryStorageComponent storageTo = m_InventoryManager.FindStorageForItem(item, EStoragePurpose.PURPOSE_ANY);
		if (storageTo && m_InventoryManager.CanInsertItemInStorage(item, storageTo))
		{
			resourceInventory.RpcAsk_ArsenalRequestItem(resourceId, Replication.FindItemId(storageTo), resourceName, EResourceType.SUPPLIES);
			return true;
		}

		// 3. Weapon whose holster slot is taken: refund the old one, take the new one (server checks both).
		if (settings.IsWeaponSwap() && ARC_TrySwapWeapon(item, resourceInventory, resourceId, resourceName))
			return true;

		// 4. Anything else the best fit rejected (grenades, mines...): equipment, then deposit storages.
		if (settings.IsFallbackStorages())
		{
			array<EStoragePurpose> purposes = {EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT, EStoragePurpose.PURPOSE_DEPOSIT};
			foreach (EStoragePurpose purpose : purposes)
			{
				BaseInventoryStorageComponent candidate = m_InventoryManager.FindStorageForItem(item, purpose);
				if (!candidate || candidate == storageTo || !m_InventoryManager.CanInsertItemInStorage(item, candidate))
					continue;

				resourceInventory.RpcAsk_ArsenalRequestItem(resourceId, Replication.FindItemId(candidate), resourceName, EResourceType.SUPPLIES);
				return true;
			}
		}

		return super.MoveBetweenFromVicinity_VirtualArsenal();
	}

	//------------------------------------------------------------------------------------------------
	//! eturn true when a swap request was sent
	protected bool ARC_TrySwapWeapon(notnull IEntity item, notnull SCR_ResourcePlayerControllerInventoryComponent resourceInventory, RplId resourceId, ResourceName resourceName)
	{
		WeaponComponent weapon = WeaponComponent.Cast(item.FindComponent(WeaponComponent));
		if (!weapon || !m_pWeaponStorage || !m_pWeaponStorageComp)
			return false;

		array<SCR_InventorySlotUI> slots = {};
		m_pWeaponStorage.GetSlots(slots);

		foreach (SCR_InventorySlotUI slot : slots)
		{
			SCR_InventorySlotWeaponSlotsUI weaponSlot = SCR_InventorySlotWeaponSlotsUI.Cast(slot);
			if (!weaponSlot || weaponSlot.ARC_GetWeaponSlotType() != weapon.GetWeaponSlotType())
				continue;

			InventoryItemComponent current = weaponSlot.GetInventoryItemComponent();
			if (!current)
				continue;

			resourceInventory.ARC_RpcAsk_ArsenalSwapItem(resourceId, Replication.FindItemId(current), Replication.FindItemId(m_pWeaponStorageComp), resourceName, EResourceType.SUPPLIES);
			return true;
		}

		return false;
	}
}

//! Server side of the weapon swap: refund the weapon currently in the holster slot, then request
//! the new one into the same weapon storage. Both halves are the vanilla request/refund handlers
//! with all their checks (ownership, distance, refundable flag, supplies, allocation), so nothing
//! can be swapped that could not be sold and bought separately.
modded class SCR_ResourcePlayerControllerInventoryComponent
{
	//------------------------------------------------------------------------------------------------
	//! \param resourceComponentRplId arsenal resource component
	//! \param inventoryItemRplId item component of the weapon to refund
	//! \param storageComponentRplId weapon storage the new weapon goes to
	//! \param resourceNameItem prefab to buy
	//! \param resourceType supplies
	void ARC_RpcAsk_ArsenalSwapItem(RplId resourceComponentRplId, RplId inventoryItemRplId, RplId storageComponentRplId, ResourceName resourceNameItem, EResourceType resourceType)
	{
		Rpc(ARC_RpcAsk_ArsenalSwapItem_, resourceComponentRplId, inventoryItemRplId, storageComponentRplId, resourceNameItem, resourceType);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void ARC_RpcAsk_ArsenalSwapItem_(RplId resourceComponentRplId, RplId inventoryItemRplId, RplId storageComponentRplId, ResourceName resourceNameItem, EResourceType resourceType)
	{
		if (!inventoryItemRplId.IsValid() || !storageComponentRplId.IsValid())
			return;

		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(Replication.FindItem(inventoryItemRplId));
		if (!itemComponent || !itemComponent.GetOwner())
			return;

		IEntity oldWeapon = itemComponent.GetOwner();
		RpcAsk_ArsenalRefundItem_(resourceComponentRplId, inventoryItemRplId, resourceType);

		// The deletion goes through the replication system; check next frame that the slot is free.
		GetGame().GetCallqueue().CallLater(ARC_RequestAfterRefund, 0, false, oldWeapon, resourceComponentRplId, storageComponentRplId, resourceNameItem, resourceType);
	}

	//------------------------------------------------------------------------------------------------
	//! The refund handler returns silently when it rejects, so the new weapon is only requested once
	//! the old one is really gone (a deleted entity reads back as null). Otherwise the supplies for
	//! the new weapon would be consumed and the spawn into the occupied slot would fail.
	protected void ARC_RequestAfterRefund(IEntity oldWeapon, RplId resourceComponentRplId, RplId storageComponentRplId, ResourceName resourceNameItem, EResourceType resourceType)
	{
		if (oldWeapon)
		{
			Print("[ARC] Weapon swap: the current weapon could not be refunded; nothing bought", LogLevel.NORMAL);
			return;
		}

		RpcAsk_ArsenalRequestItem_(resourceComponentRplId, storageComponentRplId, resourceNameItem, resourceType);
	}
}

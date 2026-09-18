//! Exposes the holster slot type ("primary", "secondary", ...) the vanilla slot keeps private, so
//! the buy flow can tell which weapon a new one would replace.
modded class SCR_InventorySlotWeaponSlotsUI
{
	//------------------------------------------------------------------------------------------------
	string ARC_GetWeaponSlotType()
	{
		return m_sWeaponSlotType;
	}
}

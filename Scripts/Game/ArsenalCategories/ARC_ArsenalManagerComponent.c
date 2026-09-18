//! Server-side persistence of the loadout a player saves at an arsenal. Vanilla keeps it in memory
//! only (and in Conflict save games); here it is also written to
//! $profile:ArsenalCategories/loadouts/<player UUID>.json when saved or cleared, and read back
//! when the player's identity is confirmed on connect, so the "saved loadout" spawn option
//! survives restarts of any scenario. The object written is the vanilla SCR_ArsenalPlayerLoadout,
//! exactly what the vanilla save-game serializer writes, so nothing about its content is invented.
modded class SCR_ArsenalManagerComponent
{
	static const string ARC_LOADOUT_DIRECTORY = "$profile:ArsenalCategories/loadouts";
	static const int ARC_LOADOUT_VERSION = 1;

	//------------------------------------------------------------------------------------------------
	override protected void DoSetPlayerLoadout(int playerId, string loadoutString, GameEntity characterEntity, SCR_EArsenalSupplyCostType arsenalSupplyType)
	{
		super.DoSetPlayerLoadout(playerId, loadoutString, characterEntity, arsenalSupplyType);

		if (!ARC_ArsenalCategoryConfig.GetActiveLayout().IsPersistLoadouts())
			return;

		const UUID playerUID = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		string path = ARC_LoadoutPath(playerUID);
		if (path.IsEmpty())
			return;

		if (loadoutString.IsEmpty())
		{
			if (FileIO.FileExists(path))
				FileIO.DeleteFile(path);

			return;
		}

		SCR_ArsenalPlayerLoadout playerLoadout;
		if (!GetPlayerArsenalLoadout(playerUID, playerLoadout) || !playerLoadout)
			return;

		if (!FileIO.FileExists(ARC_LOADOUT_DIRECTORY) && !FileIO.MakeDirectory(ARC_LOADOUT_DIRECTORY))
		{
			Print("[ARC] Cannot create " + ARC_LOADOUT_DIRECTORY + "; loadouts are not persisted", LogLevel.WARNING);
			return;
		}

		PrettyJsonSaveContext context = new PrettyJsonSaveContext();
		context.WriteValue("version", ARC_LOADOUT_VERSION);
		context.WriteValue("loadout", playerLoadout);
		if (!context.SaveToFile(path))
			Print("[ARC] Failed to write " + path, LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPlayerAuditSuccess(int playerId)
	{
		ARC_RestoreLoadout(playerId);
		super.OnPlayerAuditSuccess(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Load the player's file into the in-memory table unless a loadout is already there (a save
	//! game restored earlier in this session wins).
	protected void ARC_RestoreLoadout(int playerId)
	{
		if (!ARC_ArsenalCategoryConfig.GetActiveLayout().IsPersistLoadouts())
			return;

		const UUID playerUID = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		SCR_ArsenalPlayerLoadout existing;
		if (GetPlayerArsenalLoadout(playerUID, existing) && existing && !existing.loadout.IsEmpty())
			return;

		string path = ARC_LoadoutPath(playerUID);
		if (path.IsEmpty() || !FileIO.FileExists(path))
			return;

		JsonLoadContext context = new JsonLoadContext();
		if (!context.LoadFromFile(path))
		{
			Print("[ARC] " + path + " is not valid JSON; ignored", LogLevel.WARNING);
			return;
		}

		int version;
		context.ReadValue("version", version);

		SCR_ArsenalPlayerLoadout playerLoadout = new SCR_ArsenalPlayerLoadout();
		if (!context.ReadValue("loadout", playerLoadout) || playerLoadout.loadout.IsEmpty())
		{
			Print("[ARC] " + path + " holds no loadout; ignored", LogLevel.WARNING);
			return;
		}

		DoSetPlayerLoadout(playerUID, playerLoadout);
		PrintFormat("[ARC] Restored saved loadout of player %1", playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! \return file for this identity, empty when the identity is unknown (offline, Workbench)
	protected string ARC_LoadoutPath(UUID playerUID)
	{
		string id = playerUID;
		if (id.IsEmpty())
			return string.Empty;

		// Identities are GUID text; anything else is refused rather than used as a file name.
		for (int i = 0, length = id.Length(); i < length; i++)
		{
			string c = id.Get(i);
			if (!c.IsEmpty() && "0123456789abcdefABCDEF-".IndexOf(c) < 0)
				return string.Empty;
		}

		return ARC_LOADOUT_DIRECTORY + "/" + id + ".json";
	}
}

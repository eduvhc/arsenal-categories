//! Server side of the JSON configuration. On start it writes a template with the built-in
//! categories to $profile:ArsenalCategories/categories.json when none exists, then pushes the file
//! to every player when they spawn so everyone sees the admin's list. Clients never read the
//! server's disk; single player is its own server, so the same file applies there.
modded class SCR_BaseGameMode
{
	protected string m_sARC_ServerCategoriesJson;

	//------------------------------------------------------------------------------------------------
	protected override void OnGameStart()
	{
		super.OnGameStart();

		if (!Replication.IsServer())
			return;

		ARC_LoadServerCategories();
		GetOnPlayerSpawned().Insert(ARC_OnPlayerSpawned);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn is the first moment the player controller is guaranteed to be owned by the client.
	protected void ARC_OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		if (!Replication.IsServer() || m_sARC_ServerCategoriesJson.IsEmpty())
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (playerController)
			playerController.ARC_SendCategories(m_sARC_ServerCategoriesJson);
	}

	//------------------------------------------------------------------------------------------------
	protected void ARC_LoadServerCategories()
	{
		if (!FileIO.FileExists(ARC_CategoryJson.FILE_PATH))
		{
			if (!FileIO.FileExists(ARC_CategoryJson.DIRECTORY) && !FileIO.MakeDirectory(ARC_CategoryJson.DIRECTORY))
			{
				Print("[ARC] Cannot create " + ARC_CategoryJson.DIRECTORY + "; clients use their own configuration", LogLevel.WARNING);
				return;
			}

			ARC_ArsenalCategoryConfig defaults = ARC_ArsenalCategoryConfig.CreateDefault();
			if (ARC_CategoryJson.SaveFile(ARC_CategoryJson.FILE_PATH, defaults.GetCategories()))
				Print("[ARC] Wrote default categories to " + ARC_CategoryJson.FILE_PATH);
		}

		// Validate before pushing: a broken file is reported here, on the server, and not sent at all.
		array<ref ARC_ArsenalCategory> categories = {};
		array<ref ARC_VisibilityRule> rules = {};
		ARC_LayoutSettings layout;
		if (!ARC_CategoryJson.LoadFile(ARC_CategoryJson.FILE_PATH, categories, rules, layout))
		{
			Print("[ARC] " + ARC_CategoryJson.FILE_PATH + " is unusable; clients use their own configuration", LogLevel.WARNING);
			return;
		}

		m_sARC_ServerCategoriesJson = ARC_CategoryJson.ReadFileText(ARC_CategoryJson.FILE_PATH);
		PrintFormat("[ARC] Server categories loaded: %1 categories, %2 visibility rules, %3 chars", categories.Count(), rules.Count(), m_sARC_ServerCategoriesJson.Length());

		// The server enforces visibility in SCR_ArsenalComponent too, so it must use the same document.
		ARC_ArsenalCategoryConfig.SetServerJson(m_sARC_ServerCategoriesJson);
	}
}

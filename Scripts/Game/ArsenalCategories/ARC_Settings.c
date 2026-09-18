//! Panel layout and buy behaviour. Comes from the "layout" and "buy" objects of categories.json
//! (server pushed, so the admin decides), from the .conf, or from these defaults. Every value is
//! clamped on read, so a typo can at worst give an odd-looking panel, never a broken one.
class ARC_Settings
{
	static const int MIN_COLUMNS = 4;
	static const int MAX_COLUMNS = 12;
	static const int MIN_ROWS = 4;
	static const int MAX_ROWS = 12;
	static const int MIN_PER_COLUMN = 0;
	static const int MAX_PER_COLUMN = 30;
	static const int MIN_CATEGORY_WIDTH = 120;
	static const int MAX_CATEGORY_WIDTH = 320;
	static const int MAX_SLOTS_PER_FRAME = 400;

	// ---- layout ----
	//! Use the wide arsenal panel (own layout for the Vicinity panel: categories inside the panel,
	//! more grid columns). Off = vanilla panel with the category column beside it.
	protected bool m_bWidePanel = true;
	//! Item grid columns of the wide panel (vanilla shows 6).
	protected int m_iColumns = 8;
	//! Item grid rows of the wide panel.
	protected int m_iRows = 8;
	//! Category tiles per column before wrapping into the next; 0 = one column that scrolls with
	//! the number of categories (WCS-style).
	protected int m_iCategoriesPerColumn = 0;
	//! Width of one category button in pixels.
	protected int m_iCategoryWidth = 200;
	//! Arsenal tiles created per frame; the rest follow on the next frames so a 300-item arsenal
	//! opens without a hitch. 0 = all at once (vanilla).
	protected int m_iSlotsPerFrame = 48;

	// ---- buy (take from the arsenal with right-click / the Buy button) ----
	//! Magazines and other ammunition go to a deposit storage (pouch, backpack) before anything
	//! else, instead of being loaded into the weapon and ejecting the loaded magazine.
	protected bool m_bMagazinesToStorage = true;
	//! Buying a weapon whose holster slot is occupied refunds the old weapon and takes the new one.
	protected bool m_bWeaponSwap = true;
	//! When the best-fit storage rejects an item, try equipment and deposit storages before giving up.
	protected bool m_bFallbackStorages = true;
	//! While inspecting a weapon next to an arsenal, each attachment slot also lists the arsenal's
	//! compatible attachments and magazines as buyable tiles; buying puts them straight on the weapon.
	protected bool m_bArsenalAttachments = true;

	// ---- loadouts ----
	//! Keep the loadout a player saved at an arsenal on disk ($profile:ArsenalCategories/loadouts),
	//! so it survives server restarts and is offered again when the player reconnects.
	protected bool m_bPersistLoadouts = true;

	//------------------------------------------------------------------------------------------------
	static ARC_Settings Create(bool widePanel, int columns, int rows, int categoriesPerColumn, int categoryWidth, bool magazinesToStorage = true, bool weaponSwap = true, bool fallbackStorages = true, bool arsenalAttachments = true, int slotsPerFrame = 48, bool persistLoadouts = true)
	{
		ARC_Settings settings = new ARC_Settings();
		settings.m_bWidePanel = widePanel;
		settings.m_iColumns = Math.ClampInt(columns, MIN_COLUMNS, MAX_COLUMNS);
		settings.m_iRows = Math.ClampInt(rows, MIN_ROWS, MAX_ROWS);
		settings.m_iCategoriesPerColumn = Math.ClampInt(categoriesPerColumn, MIN_PER_COLUMN, MAX_PER_COLUMN);
		settings.m_iCategoryWidth = Math.ClampInt(categoryWidth, MIN_CATEGORY_WIDTH, MAX_CATEGORY_WIDTH);
		settings.m_bMagazinesToStorage = magazinesToStorage;
		settings.m_bWeaponSwap = weaponSwap;
		settings.m_bFallbackStorages = fallbackStorages;
		settings.m_bArsenalAttachments = arsenalAttachments;
		settings.m_iSlotsPerFrame = Math.ClampInt(slotsPerFrame, 0, MAX_SLOTS_PER_FRAME);
		settings.m_bPersistLoadouts = persistLoadouts;
		return settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsWidePanel()
	{
		return m_bWidePanel;
	}

	//------------------------------------------------------------------------------------------------
	int GetColumns()
	{
		return m_iColumns;
	}

	//------------------------------------------------------------------------------------------------
	int GetRows()
	{
		return m_iRows;
	}

	//------------------------------------------------------------------------------------------------
	int GetCategoriesPerColumn()
	{
		return m_iCategoriesPerColumn;
	}

	//------------------------------------------------------------------------------------------------
	int GetCategoryWidth()
	{
		return m_iCategoryWidth;
	}

	//------------------------------------------------------------------------------------------------
	bool IsMagazinesToStorage()
	{
		return m_bMagazinesToStorage;
	}

	//------------------------------------------------------------------------------------------------
	bool IsWeaponSwap()
	{
		return m_bWeaponSwap;
	}

	//------------------------------------------------------------------------------------------------
	bool IsFallbackStorages()
	{
		return m_bFallbackStorages;
	}

	//------------------------------------------------------------------------------------------------
	bool IsArsenalAttachments()
	{
		return m_bArsenalAttachments;
	}

	//------------------------------------------------------------------------------------------------
	int GetSlotsPerFrame()
	{
		return m_iSlotsPerFrame;
	}

	//------------------------------------------------------------------------------------------------
	bool IsPersistLoadouts()
	{
		return m_bPersistLoadouts;
	}
}

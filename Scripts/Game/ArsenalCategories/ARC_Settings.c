//! Panel layout and buy behaviour. Comes from the "layout" and "buy" objects of categories.json
//! (server pushed, so the admin decides), from the .conf, or from these defaults. Every value is
//! clamped on read, so a typo can at worst give an odd-looking panel, never a broken one.
class ARC_Settings
{
	static const int MIN_COLUMNS = 4;
	static const int MAX_COLUMNS = 12;
	static const int MIN_ROWS = 4;
	static const int MAX_ROWS = 12;
	static const int MIN_PER_COLUMN = 4;
	static const int MAX_PER_COLUMN = 30;
	static const int MIN_CATEGORY_WIDTH = 120;
	static const int MAX_CATEGORY_WIDTH = 320;

	// ---- layout ----
	//! Use the wide arsenal panel (own layout for the Vicinity panel: categories inside the panel,
	//! more grid columns). Off = vanilla panel with the category column beside it.
	protected bool m_bWidePanel = true;
	//! Item grid columns of the wide panel (vanilla shows 6).
	protected int m_iColumns = 8;
	//! Item grid rows of the wide panel.
	protected int m_iRows = 8;
	//! Category buttons per column; longer lists wrap into further columns.
	protected int m_iCategoriesPerColumn = 11;
	//! Width of one category button in pixels.
	protected int m_iCategoryWidth = 200;

	// ---- buy (take from the arsenal with right-click / the Buy button) ----
	//! Magazines and other ammunition go to a deposit storage (pouch, backpack) before anything
	//! else, instead of being loaded into the weapon and ejecting the loaded magazine.
	protected bool m_bMagazinesToStorage = true;
	//! Buying a weapon whose holster slot is occupied refunds the old weapon and takes the new one.
	protected bool m_bWeaponSwap = true;
	//! When the best-fit storage rejects an item, try equipment and deposit storages before giving up.
	protected bool m_bFallbackStorages = true;

	//------------------------------------------------------------------------------------------------
	static ARC_Settings Create(bool widePanel, int columns, int rows, int categoriesPerColumn, int categoryWidth, bool magazinesToStorage = true, bool weaponSwap = true, bool fallbackStorages = true)
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
}

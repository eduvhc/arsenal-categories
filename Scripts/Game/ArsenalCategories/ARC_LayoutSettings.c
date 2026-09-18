//! How the arsenal panel is laid out. Comes from the "layout" object of categories.json (server
//! pushed, so the admin decides), from the .conf, or from these defaults. Every value is clamped
//! on read, so a typo can at worst give an odd-looking panel, never a broken one.
class ARC_LayoutSettings
{
	static const int MIN_COLUMNS = 4;
	static const int MAX_COLUMNS = 12;
	static const int MIN_ROWS = 4;
	static const int MAX_ROWS = 12;
	static const int MIN_PER_COLUMN = 4;
	static const int MAX_PER_COLUMN = 30;
	static const int MIN_CATEGORY_WIDTH = 120;
	static const int MAX_CATEGORY_WIDTH = 320;

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

	//------------------------------------------------------------------------------------------------
	static ARC_LayoutSettings Create(bool widePanel, int columns, int rows, int categoriesPerColumn, int categoryWidth)
	{
		ARC_LayoutSettings settings = new ARC_LayoutSettings();
		settings.m_bWidePanel = widePanel;
		settings.m_iColumns = Math.ClampInt(columns, MIN_COLUMNS, MAX_COLUMNS);
		settings.m_iRows = Math.ClampInt(rows, MIN_ROWS, MAX_ROWS);
		settings.m_iCategoriesPerColumn = Math.ClampInt(categoriesPerColumn, MIN_PER_COLUMN, MAX_PER_COLUMN);
		settings.m_iCategoryWidth = Math.ClampInt(categoryWidth, MIN_CATEGORY_WIDTH, MAX_CATEGORY_WIDTH);
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
}

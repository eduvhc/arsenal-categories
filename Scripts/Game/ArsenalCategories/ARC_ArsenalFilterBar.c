//! Category list for an arsenal panel: inventory-slot-styled tiles (icon, name, count), "All"
//! first and "Other" last, the selected one framed in orange like a selected item. Owns no game
//! state: it only remembers the selected index and tells the controller when the player picks
//! another one.
//!
//! Placement, first that applies:
//!   1. the "ARC_Categories" grid of the wide arsenal panel (ARC_InventoryContainerGrid.layout,
//!      chosen by SCR_InventoryMenuUI.ShowVicinity when the layout says widePanel): one column
//!      that scrolls with the number of categories, or several columns of categoriesPerColumn
//!      tiles when that setting is not 0;
//!   2. a single column inserted into the inventory menu's own content row ("InventoryContent" in
//!      InventoryMain.layout), z-ordered before the vicinity column so it sits to its left;
//!   3. a two-column block between the "Arsenal" title and the item grid.
//! No vanilla layout is overridden and nothing hangs outside a panel, so the list can neither be
//! clipped nor disturb the other storage panels.
class ARC_ArsenalFilterBar
{
	static const ResourceName BUTTON_LAYOUT = "{8C8B0CF4F9950F8E}UI/layouts/Menus/Inventory/ARC_ArsenalCategoryButton.layout";
	static const string ICON_ALL = "gridView";
	static const string ICON_OTHER = "misc";
	static const int ALL_INDEX = -1;
	static const int OTHER_INDEX = -2;
	static const string PANEL_HOST = "ARC_Categories";
	static const string PANEL_SCROLL = "ARC_CategoriesScroll";
	static const string ITEM_GRID = "GridLayout0";
	static const string SIDEBAR_HOST = "InventoryContent";
	static const int SIDEBAR_ZORDER = -1;
	static const float SIDEBAR_GAP = 8;
	static const int INLINE_COLUMNS = 2;
	static const int INLINE_ZORDER = 1000;
	static const float BUTTON_HEIGHT = 36;
	static const float BUTTON_SPACING = 2;

	//! Invoked with the new category index after the player clicks a button.
	ref ScriptInvoker m_OnCategoryChanged = new ScriptInvoker();

	protected Widget m_wRoot;
	protected Widget m_wPanelScroll;
	protected bool m_bSidebar;
	protected bool m_bPanelGrid;
	protected int m_iPerColumn;
	protected float m_fButtonWidth;
	protected ref array<Widget> m_aColumns = {};
	protected ref array<ARC_CategoryButtonComponent> m_aButtons = {};
	protected int m_iSelected = ALL_INDEX;

	//------------------------------------------------------------------------------------------------
	//! \param panelRoot root widget of the storage panel
	//! \param menuRoot root widget of the inventory menu (may be null; enables the side column)
	//! \param labels captions in display order
	//! \param counts item count per label
	//! \param icons icon per label (.edds resource; empty for the built-in All / Other icons)
	//! \param categoryIndices category index per label (ALL_INDEX / OTHER_INDEX allowed)
	void ARC_ArsenalFilterBar(notnull Widget panelRoot, Widget menuRoot, notnull array<string> labels, notnull array<int> counts, notnull array<ResourceName> icons, notnull array<int> categoryIndices)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();

		ARC_Settings settings = ARC_ArsenalCategoryConfig.GetActiveLayout();
		m_iPerColumn = settings.GetCategoriesPerColumn();
		m_fButtonWidth = settings.GetCategoryWidth();

		Widget panelGrid = panelRoot.FindAnyWidget(PANEL_HOST);
		m_bPanelGrid = panelGrid != null;

		Widget contentRow;
		if (!m_bPanelGrid && menuRoot)
			contentRow = menuRoot.FindAnyWidget(SIDEBAR_HOST);

		m_bSidebar = contentRow != null;

		if (m_bPanelGrid)
		{
			// The grid is part of the panel layout: tiles go straight into it, placed by row/column.
			m_wRoot = panelGrid;
			m_wPanelScroll = panelRoot.FindAnyWidget(PANEL_SCROLL);
			if (m_wPanelScroll)
				m_wPanelScroll.SetVisible(true);
		}
		else if (m_bSidebar)
		{
			m_wRoot = workspace.CreateWidget(WidgetType.VerticalLayoutWidgetTypeID, WidgetFlags.VISIBLE, new Color(1, 1, 1, 1), SIDEBAR_ZORDER, contentRow);
			if (!m_wRoot)
			{
				Print("[ARC] CreateWidget(VerticalLayout) returned null", LogLevel.WARNING);
				return;
			}

			// Lower z-order than the vicinity column ("Left", 0) puts the list before it in the row.
			m_wRoot.SetZOrder(SIDEBAR_ZORDER);
			LayoutSlot.SetVerticalAlign(m_wRoot, LayoutVerticalAlign.Top);
			LayoutSlot.SetPadding(m_wRoot, 0, 0, SIDEBAR_GAP, 0);
			m_aColumns.Insert(m_wRoot);
		}
		else
		{
			Widget host = panelRoot.FindAnyWidget("titleLayout");
			if (!host)
			{
				Print("[ARC] titleLayout not found in arsenal panel", LogLevel.WARNING);
				return;
			}

			m_wRoot = workspace.CreateWidget(WidgetType.HorizontalLayoutWidgetTypeID, WidgetFlags.VISIBLE, new Color(1, 1, 1, 1), INLINE_ZORDER, host);
			if (!m_wRoot)
			{
				Print("[ARC] CreateWidget(HorizontalLayout) returned null", LogLevel.WARNING);
				return;
			}

			// High z-order keeps the list below traverse titles the panel recreates on every refresh.
			m_wRoot.SetZOrder(INLINE_ZORDER);
			LayoutSlot.SetPadding(m_wRoot, 0, 2, 0, 4);

			for (int i = 0; i < INLINE_COLUMNS; i++)
			{
				Widget column = workspace.CreateWidget(WidgetType.VerticalLayoutWidgetTypeID, WidgetFlags.VISIBLE, new Color(1, 1, 1, 1), 0, m_wRoot);
				if (!column)
					continue;

				LayoutSlot.SetSizeMode(column, LayoutSizeMode.Fill);
				if (i > 0)
					LayoutSlot.SetPadding(column, BUTTON_SPACING, 0, 0, 0);

				m_aColumns.Insert(column);
			}
		}

		foreach (int i, string label : labels)
		{
			AddButton(categoryIndices[i], label, counts[i], icons[i]);
		}

		if (m_bPanelGrid)
			LinkGridNavigation();

		Select(ALL_INDEX, false);
		PrintFormat("[ARC] Filter list created with %1 button(s)", m_aButtons.Count());
	}

	//------------------------------------------------------------------------------------------------
	void ~ARC_ArsenalFilterBar()
	{
		foreach (ARC_CategoryButtonComponent button : m_aButtons)
		{
			if (button)
				button.m_OnClicked.Remove(OnButtonClicked);
		}

		if (m_bPanelGrid)
		{
			// The grid belongs to the panel layout: empty it and hide its scroll column, keep it.
			foreach (ARC_CategoryButtonComponent button : m_aButtons)
			{
				if (button && button.GetRootWidget())
					button.GetRootWidget().RemoveFromHierarchy();
			}

			if (m_wPanelScroll)
				m_wPanelScroll.SetVisible(false);
		}
		else if (m_wRoot)
		{
			m_wRoot.RemoveFromHierarchy();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return true when the container and at least the "All" button exist
	bool IsValid()
	{
		return m_wRoot && !m_aButtons.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	int GetSelected()
	{
		return m_iSelected;
	}

	//------------------------------------------------------------------------------------------------
	//! Programmatic selection, e.g. to restore the previous category after the panel is rebuilt.
	void Select(int categoryIndex, bool notify = true)
	{
		m_iSelected = categoryIndex;

		foreach (ARC_CategoryButtonComponent button : m_aButtons)
		{
			if (button)
				button.SetSelected(button.GetCategoryIndex() == categoryIndex);
		}

		if (notify)
			m_OnCategoryChanged.Invoke(categoryIndex);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddButton(int categoryIndex, string label, int count, ResourceName icon)
	{
		Widget host;
		if (m_bPanelGrid)
		{
			host = m_wRoot;
		}
		else
		{
			if (m_aColumns.IsEmpty())
				return;

			host = m_aColumns[m_aButtons.Count() % m_aColumns.Count()];
		}

		Widget buttonWidget = GetGame().GetWorkspace().CreateWidgets(BUTTON_LAYOUT, host);
		if (!buttonWidget)
		{
			Print("[ARC] Could not create ARC_ArsenalCategoryButton widget", LogLevel.WARNING);
			return;
		}

		ARC_CategoryButtonComponent button = ARC_CategoryButtonComponent.Cast(buttonWidget.FindHandler(ARC_CategoryButtonComponent));
		if (!button)
		{
			Print("[ARC] ARC_ArsenalCategoryButton has no ARC_CategoryButtonComponent handler", LogLevel.WARNING);
			buttonWidget.RemoveFromHierarchy();
			return;
		}

		int index = m_aButtons.Count();
		if (m_bPanelGrid)
		{
			// 0 tiles per column = one column that scrolls; otherwise wrap into further columns.
			int row = index;
			int column = 0;
			if (m_iPerColumn > 0)
			{
				row = index % m_iPerColumn;
				column = index / m_iPerColumn;
			}

			GridSlot.SetRow(buttonWidget, row);
			GridSlot.SetColumn(buttonWidget, column);
			GridSlot.SetPadding(buttonWidget, 0, 0, BUTTON_SPACING, BUTTON_SPACING);
		}
		else
		{
			LayoutSlot.SetHorizontalAlign(buttonWidget, LayoutHorizontalAlign.Stretch);
			LayoutSlot.SetPadding(buttonWidget, 0, 0, 0, BUTTON_SPACING);
		}

		SizeLayoutWidget size = SizeLayoutWidget.Cast(buttonWidget);
		if (size)
		{
			size.SetHeightOverride(BUTTON_HEIGHT);
			if (m_bSidebar || m_bPanelGrid)
				size.SetWidthOverride(m_fButtonWidth);
			else
				size.EnableWidthOverride(false);
		}

		string imageSetIcon;
		if (categoryIndex == ALL_INDEX)
			imageSetIcon = ICON_ALL;
		else if (categoryIndex == OTHER_INDEX)
			imageSetIcon = ICON_OTHER;

		button.Init(categoryIndex, label, count, icon, imageSetIcon);
		button.m_OnClicked.Insert(OnButtonClicked);
		m_aButtons.Insert(button);
	}

	//------------------------------------------------------------------------------------------------
	//! Gamepad / keyboard: right from the last column of tiles lands on the item grid; the grid's
	//! own left rule (set in the layout) comes back to the category grid.
	protected void LinkGridNavigation()
	{
		int lastColumn = 0;
		if (m_iPerColumn > 0)
			lastColumn = (m_aButtons.Count() - 1) / m_iPerColumn;

		foreach (int i, ARC_CategoryButtonComponent button : m_aButtons)
		{
			int column = 0;
			if (m_iPerColumn > 0)
				column = i / m_iPerColumn;

			if (button && button.GetButton() && column == lastColumn)
				button.GetButton().SetNavigation(WidgetNavigationDirection.RIGHT, WidgetNavigationRuleType.EXPLICIT, ITEM_GRID);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonClicked(ARC_CategoryButtonComponent clicked)
	{
		if (!clicked)
			return;

		Select(clicked.GetCategoryIndex());
	}
}

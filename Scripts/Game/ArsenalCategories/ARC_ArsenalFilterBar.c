//! Category list for an arsenal panel: icon + text buttons ("Assault Rifles (24)"), "All" first
//! and "Other" last. Built from the vanilla WLib_ButtonTextImage layout so it inherits the game's look,
//! focus and gamepad handling. Owns no game state: it only remembers the selected index and tells
//! the controller when the player picks another one.
//!
//! Placement, first that applies:
//!   1. the "ARC_Categories" grid of the wide arsenal panel (ARC_InventoryContainerGrid.layout,
//!      chosen by SCR_InventoryMenuUI.ShowVicinity when the layout says widePanel): buttons fill
//!      it top-down, wrapping into a new column every categoriesPerColumn buttons, WCS-style;
//!   2. a single column inserted into the inventory menu's own content row ("InventoryContent" in
//!      InventoryMain.layout), z-ordered before the vicinity column so it sits to its left;
//!   3. a two-column block between the "Arsenal" title and the item grid.
//! No vanilla layout is overridden and nothing hangs outside a panel, so the list can neither be
//! clipped nor disturb the other storage panels.
class ARC_ArsenalFilterBar
{
	static const ResourceName BUTTON_LAYOUT = "{4913D5BED796721F}UI/layouts/WidgetLibrary/Buttons/WLib_ButtonTextImage.layout";
	static const ResourceName ICON_IMAGESET = "{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset";
	static const string ICON_ALL = "gridView";
	static const string ICON_OTHER = "misc";
	static const int ALL_INDEX = -1;
	static const int OTHER_INDEX = -2;
	static const string PANEL_HOST = "ARC_Categories";
	static const string PANEL_SCROLL = "ARC_CategoriesScroll";
	static const string SIDEBAR_HOST = "InventoryContent";
	static const int SIDEBAR_ZORDER = -1;
	static const float SIDEBAR_WIDTH = 200;
	static const float SIDEBAR_GAP = 8;
	static const int INLINE_COLUMNS = 2;
	static const int INLINE_ZORDER = 1000;
	static const float BUTTON_HEIGHT = 36;
	static const float ICON_SIZE = 20;
	static const float BUTTON_SPACING = 2;

	//! Invoked with the new category index after the player clicks a button.
	ref ScriptInvoker m_OnCategoryChanged = new ScriptInvoker();

	protected Widget m_wRoot;
	protected Widget m_wPanelScroll;
	protected bool m_bSidebar;
	protected bool m_bPanelGrid;
	protected int m_iPerColumn;
	protected float m_fButtonWidth = SIDEBAR_WIDTH;
	protected ref array<Widget> m_aColumns = {};
	protected ref array<SCR_ButtonTextComponent> m_aButtons = {};
	protected ref array<int> m_aButtonCategories = {};
	protected int m_iSelected = ALL_INDEX;

	//------------------------------------------------------------------------------------------------
	//! \param panelRoot root widget of the storage panel (InventoryContainerGrid "ContainerRoot")
	//! \param menuRoot root widget of the inventory menu (may be null; enables the side column)
	//! \param labels button captions in display order (already including counts)
	//! \param icons icon per label (.edds resource; empty for the built-in All / Other icons)
	//! \param categoryIndices category index per label (ALL_INDEX / OTHER_INDEX allowed)
	void ARC_ArsenalFilterBar(notnull Widget panelRoot, Widget menuRoot, notnull array<string> labels, notnull array<ResourceName> icons, notnull array<int> categoryIndices)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		int columns = INLINE_COLUMNS;

		ARC_LayoutSettings layout = ARC_ArsenalCategoryConfig.GetActiveLayout();
		m_iPerColumn = layout.GetCategoriesPerColumn();
		m_fButtonWidth = layout.GetCategoryWidth();

		Widget panelGrid = panelRoot.FindAnyWidget(PANEL_HOST);
		m_bPanelGrid = panelGrid != null;

		Widget contentRow;
		if (!m_bPanelGrid && menuRoot)
			contentRow = menuRoot.FindAnyWidget(SIDEBAR_HOST);

		m_bSidebar = contentRow != null;

		if (m_bPanelGrid)
		{
			// The grid is part of the panel layout: buttons go straight into it, placed by row/column.
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

			for (int i = 0; i < columns; i++)
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
			AddButton(categoryIndices[i], label, icons[i]);
		}

		Select(ALL_INDEX, false);
		PrintFormat("[ARC] Filter list created with %1 button(s)", m_aButtons.Count());
	}

	//------------------------------------------------------------------------------------------------
	void ~ARC_ArsenalFilterBar()
	{
		foreach (SCR_ButtonTextComponent button : m_aButtons)
		{
			if (button)
				button.m_OnClicked.Remove(OnButtonClicked);
		}

		if (m_bPanelGrid)
		{
			// The grid belongs to the panel layout: empty it and hide its scroll column, keep it.
			foreach (SCR_ButtonTextComponent button : m_aButtons)
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

		foreach (int i, SCR_ButtonTextComponent button : m_aButtons)
		{
			if (button)
				button.SetToggled(m_aButtonCategories[i] == categoryIndex, false, false);
		}

		if (notify)
			m_OnCategoryChanged.Invoke(categoryIndex);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddButton(int categoryIndex, string label, ResourceName icon)
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
			Print("[ARC] Could not create WLib_ButtonTextImage widget", LogLevel.WARNING);
			return;
		}

		SCR_ButtonTextComponent button = SCR_ButtonTextComponent.Cast(buttonWidget.FindHandler(SCR_ButtonTextComponent));
		if (!button)
		{
			Print("[ARC] WLib_ButtonTextImage has no SCR_ButtonTextComponent handler", LogLevel.WARNING);
			buttonWidget.RemoveFromHierarchy();
			return;
		}

		if (m_bPanelGrid)
		{
			int index = m_aButtons.Count();
			GridSlot.SetRow(buttonWidget, index % m_iPerColumn);
			GridSlot.SetColumn(buttonWidget, index / m_iPerColumn);
			GridSlot.SetPadding(buttonWidget, 0, 0, BUTTON_SPACING, BUTTON_SPACING);
		}
		else
		{
			LayoutSlot.SetHorizontalAlign(buttonWidget, LayoutHorizontalAlign.Stretch);
			LayoutSlot.SetPadding(buttonWidget, 0, 0, 0, BUTTON_SPACING);
		}

		SizeLayoutWidget size = SizeLayoutWidget.Cast(buttonWidget.FindAnyWidget("SizeLayout"));
		if (size)
		{
			size.SetHeightOverride(BUTTON_HEIGHT);
			if (m_bSidebar || m_bPanelGrid)
				size.SetWidthOverride(m_fButtonWidth);
		}

		ImageWidget image = ImageWidget.Cast(buttonWidget.FindAnyWidget("Image0"));
		if (image)
		{
			if (categoryIndex == ALL_INDEX)
				image.LoadImageFromSet(0, ICON_IMAGESET, ICON_ALL);
			else if (categoryIndex == OTHER_INDEX)
				image.LoadImageFromSet(0, ICON_IMAGESET, ICON_OTHER);
			else if (!icon.IsEmpty())
				image.LoadImageTexture(0, icon);
			else
				image.SetVisible(false);

			image.SetSize(ICON_SIZE, ICON_SIZE);
		}

		button.SetText(label);
		button.m_OnClicked.Insert(OnButtonClicked);
		m_aButtons.Insert(button);
		m_aButtonCategories.Insert(categoryIndex);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonClicked(SCR_ButtonBaseComponent clicked)
	{
		int index = m_aButtons.Find(SCR_ButtonTextComponent.Cast(clicked));
		if (index < 0)
			return;

		Select(m_aButtonCategories[index]);
	}
}

//! Category list shown above the arsenal grid: full-width text buttons ("Assault Rifles (24)")
//! laid out in two columns, "All" first and "Other" last. Built from the vanilla WLib_ButtonText
//! layout so it inherits the game's look, focus and gamepad handling. Owns no game state: it only
//! remembers the selected index and tells the controller when the player picks another one.
class ARC_ArsenalFilterBar
{
	static const ResourceName BUTTON_LAYOUT = "{75C912A1C89BE6C2}UI/layouts/WidgetLibrary/Buttons/WLib_ButtonText.layout";
	static const int ALL_INDEX = -1;
	static const int OTHER_INDEX = -2;
	static const int COLUMNS = 2;
	static const float BUTTON_HEIGHT = 28;
	static const float BUTTON_SPACING = 2;

	//! Invoked with the new category index after the player clicks a button.
	ref ScriptInvoker m_OnCategoryChanged = new ScriptInvoker();

	protected Widget m_wParent;
	protected Widget m_wRoot;
	protected ref array<Widget> m_aColumns = {};
	protected ref array<SCR_ButtonTextComponent> m_aButtons = {};
	protected ref array<int> m_aButtonCategories = {};
	protected int m_iSelected = ALL_INDEX;

	//------------------------------------------------------------------------------------------------
	//! \param parent vertical layout the list is appended to
	//! \param labels button captions in display order (already including counts)
	//! \param categoryIndices category index per label (ALL_INDEX / OTHER_INDEX allowed)
	void ARC_ArsenalFilterBar(notnull Widget parent, notnull array<string> labels, notnull array<int> categoryIndices)
	{
		m_wParent = parent;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		m_wRoot = workspace.CreateWidget(WidgetType.HorizontalLayoutWidgetTypeID, WidgetFlags.VISIBLE, new Color(1, 1, 1, 1), 0, parent);
		if (!m_wRoot)
		{
			Print("[ARC] CreateWidget(HorizontalLayout) returned null", LogLevel.WARNING);
			return;
		}

		LayoutSlot.SetPadding(m_wRoot, 0, 2, 0, 4);

		for (int i = 0; i < COLUMNS; i++)
		{
			Widget column = workspace.CreateWidget(WidgetType.VerticalLayoutWidgetTypeID, WidgetFlags.VISIBLE, new Color(1, 1, 1, 1), 0, m_wRoot);
			if (!column)
				continue;

			LayoutSlot.SetSizeMode(column, LayoutSizeMode.Fill);
			if (i > 0)
				LayoutSlot.SetPadding(column, BUTTON_SPACING, 0, 0, 0);

			m_aColumns.Insert(column);
		}

		foreach (int i, string label : labels)
		{
			AddButton(categoryIndices[i], label);
		}

		Select(ALL_INDEX, false);
		PrintFormat("[ARC] Filter bar created with %1 button(s)", m_aButtons.Count());
	}

	//------------------------------------------------------------------------------------------------
	void ~ARC_ArsenalFilterBar()
	{
		foreach (SCR_ButtonTextComponent button : m_aButtons)
		{
			if (button)
				button.m_OnClicked.Remove(OnButtonClicked);
		}

		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
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
	//! Re-append the list to its parent so it stays below traverse titles the panel recreates on refresh.
	void MoveToEnd()
	{
		if (!m_wRoot || !m_wParent)
			return;

		m_wParent.RemoveChild(m_wRoot);
		m_wParent.AddChild(m_wRoot);
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
	protected void AddButton(int categoryIndex, string label)
	{
		if (m_aColumns.IsEmpty())
			return;

		Widget column = m_aColumns[m_aButtons.Count() % m_aColumns.Count()];
		Widget buttonWidget = GetGame().GetWorkspace().CreateWidgets(BUTTON_LAYOUT, column);
		if (!buttonWidget)
		{
			Print("[ARC] Could not create WLib_ButtonText widget", LogLevel.WARNING);
			return;
		}

		SCR_ButtonTextComponent button = SCR_ButtonTextComponent.Cast(buttonWidget.FindHandler(SCR_ButtonTextComponent));
		if (!button)
		{
			Print("[ARC] WLib_ButtonText has no SCR_ButtonTextComponent handler", LogLevel.WARNING);
			buttonWidget.RemoveFromHierarchy();
			return;
		}

		LayoutSlot.SetHorizontalAlign(buttonWidget, LayoutHorizontalAlign.Stretch);
		LayoutSlot.SetPadding(buttonWidget, 0, 0, 0, BUTTON_SPACING);

		SizeLayoutWidget size = SizeLayoutWidget.Cast(buttonWidget.FindAnyWidget("SizeLayout"));
		if (size)
			size.SetHeightOverride(BUTTON_HEIGHT);

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

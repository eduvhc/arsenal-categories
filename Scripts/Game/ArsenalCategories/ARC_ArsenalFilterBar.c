//! Row of toggle buttons ("All" + one per category that has items) inserted above the arsenal grid.
//! Built from the vanilla WLib_ButtonFilter layout so it inherits the game's look, focus and gamepad
//! handling. Owns no game state: it only remembers the selected index and tells the panel to refresh.
class ARC_ArsenalFilterBar
{
	static const ResourceName BUTTON_LAYOUT = "{588A7F6200246F97}UI/layouts/WidgetLibrary/Buttons/WLib_ButtonFilter.layout";
	static const ResourceName ICON_IMAGESET = "{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset";
	static const string ICON_ALL = "gridView";
	static const string ICON_OTHER = "misc";
	static const int ALL_INDEX = -1;
	static const int OTHER_INDEX = -2;
	static const float BUTTON_SIZE = 44;
	static const float BUTTON_SPACING = 2;

	//! Invoked with the new category index after the player clicks a button.
	ref ScriptInvoker m_OnCategoryChanged = new ScriptInvoker();

	protected Widget m_wRoot;
	protected ref array<SCR_ButtonBaseComponent> m_aButtons = {};
	protected ref array<int> m_aButtonCategories = {};
	protected int m_iSelected = ALL_INDEX;

	//------------------------------------------------------------------------------------------------
	//! \param parent widget the bar is appended to (a vertical layout)
	//! \param categories all configured categories, in display order
	//! \param availableCategories indices into categories that have at least one item in this arsenal
	//! \param hasOther true when some items match no category at all
	void ARC_ArsenalFilterBar(notnull Widget parent, notnull array<ref ARC_ArsenalCategory> categories, notnull array<int> availableCategories, bool hasOther)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		m_wRoot = workspace.CreateWidget(WidgetType.HorizontalLayoutWidgetTypeID, WidgetFlags.VISIBLE, new Color(1, 1, 1, 1), 0, parent);
		if (!m_wRoot)
			return;

		LayoutSlot.SetPadding(m_wRoot, 0, 2, 0, 4);

		AddButton(ALL_INDEX, ResourceName.Empty, ICON_ALL);

		foreach (int index : availableCategories)
		{
			ARC_ArsenalCategory category = categories[index];
			if (category)
				AddButton(index, category.GetIcon(), category.GetImageName());
		}

		if (hasOther)
			AddButton(OTHER_INDEX, ResourceName.Empty, ICON_OTHER);

		Select(ALL_INDEX, false);
	}

	//------------------------------------------------------------------------------------------------
	void ~ARC_ArsenalFilterBar()
	{
		foreach (SCR_ButtonBaseComponent button : m_aButtons)
		{
			if (button)
				button.m_OnClicked.Remove(OnButtonClicked);
		}

		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
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

		foreach (int i, SCR_ButtonBaseComponent button : m_aButtons)
		{
			if (button)
				button.SetToggled(m_aButtonCategories[i] == categoryIndex, false, false);
		}

		if (notify)
			m_OnCategoryChanged.Invoke(categoryIndex);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddButton(int categoryIndex, ResourceName icon, string imageName)
	{
		Widget buttonWidget = GetGame().GetWorkspace().CreateWidgets(BUTTON_LAYOUT, m_wRoot);
		if (!buttonWidget)
			return;

		SCR_ButtonImageComponent button = SCR_ButtonImageComponent.Cast(buttonWidget.FindHandler(SCR_ButtonImageComponent));
		if (!button)
		{
			buttonWidget.RemoveFromHierarchy();
			return;
		}

		// The vanilla layout is 50x50; shrink a little so nine buttons fit the six-column panel.
		SizeLayoutWidget size = SizeLayoutWidget.Cast(buttonWidget.FindAnyWidget("SizeLayout"));
		if (size)
		{
			size.SetWidthOverride(BUTTON_SIZE);
			size.SetHeightOverride(BUTTON_SIZE);
		}

		LayoutSlot.SetPadding(buttonWidget, 0, 0, BUTTON_SPACING, 0);

		if (icon.IsEmpty() || icon.EndsWith(".imageset"))
		{
			ResourceName imageSet = icon;
			if (imageSet.IsEmpty())
				imageSet = ICON_IMAGESET;

			button.SetImage(imageSet, imageName);
		}
		else
		{
			button.SetImage(icon);
		}

		button.m_OnClicked.Insert(OnButtonClicked);
		m_aButtons.Insert(button);
		m_aButtonCategories.Insert(categoryIndex);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonClicked(SCR_ButtonBaseComponent clicked)
	{
		int index = m_aButtons.Find(clicked);
		if (index < 0)
			return;

		// The vanilla button toggles itself on click; clicking the active category must keep it active.
		Select(m_aButtonCategories[index]);
	}
}

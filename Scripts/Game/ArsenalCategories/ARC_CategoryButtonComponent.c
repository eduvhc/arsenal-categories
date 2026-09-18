//! Handler of ARC_ArsenalCategoryButton.layout: an inventory-slot-styled tile (icon, name, count,
//! orange selection frame like a selected item) used for every category button. Owns no state
//! beyond its category index; clicks and gamepad focus go to the filter bar through m_OnClicked.
class ARC_CategoryButtonComponent : SCR_ScriptedWidgetComponent
{
	static const string ICON_IMAGESET = "{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset";

	//! Invoked with this component when the tile is clicked (or focused with a gamepad).
	ref ScriptInvoker m_OnClicked = new ScriptInvoker();

	protected int m_iCategoryIndex;
	protected ImageWidget m_wIcon;
	protected TextWidget m_wLabel;
	protected TextWidget m_wCount;
	protected Widget m_wSelected;
	protected ButtonWidget m_wButton;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wIcon = ImageWidget.Cast(w.FindAnyWidget("ARC_Icon"));
		m_wLabel = TextWidget.Cast(w.FindAnyWidget("ARC_Label"));
		m_wCount = TextWidget.Cast(w.FindAnyWidget("ARC_Count"));
		m_wSelected = w.FindAnyWidget("ARC_Selected");
		m_wButton = ButtonWidget.Cast(w.FindAnyWidget("ARC_Button"));
	}

	//------------------------------------------------------------------------------------------------
	//! \param categoryIndex category index (ARC_ArsenalFilterBar.ALL_INDEX / OTHER_INDEX allowed)
	//! \param label caption
	//! \param count item count shown at the right (< 0 hides it)
	//! \param icon texture resource; empty = imageSetIcon from the wrapper UI image set
	//! \param imageSetIcon icon name in icons_wrapperUI-64 when icon is empty
	void Init(int categoryIndex, string label, int count, ResourceName icon, string imageSetIcon = "")
	{
		m_iCategoryIndex = categoryIndex;

		if (m_wLabel)
			m_wLabel.SetText(label);

		if (m_wCount)
		{
			m_wCount.SetVisible(count >= 0);
			if (count >= 0)
				m_wCount.SetText(count.ToString());
		}

		if (m_wIcon)
		{
			if (!icon.IsEmpty())
				m_wIcon.LoadImageTexture(0, icon);
			else if (!imageSetIcon.IsEmpty())
				m_wIcon.LoadImageFromSet(0, ICON_IMAGESET, imageSetIcon);
			else
				m_wIcon.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	int GetCategoryIndex()
	{
		return m_iCategoryIndex;
	}

	//------------------------------------------------------------------------------------------------
	void SetSelected(bool selected)
	{
		if (m_wSelected)
			m_wSelected.SetVisible(selected);
	}

	//------------------------------------------------------------------------------------------------
	ButtonWidget GetButton()
	{
		return m_wButton;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		m_OnClicked.Invoke(this);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Gamepad: moving the focus onto a tile selects it, so the list is browsable with the stick.
	override bool OnFocus(Widget w, int x, int y)
	{
		if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.GAMEPAD)
			m_OnClicked.Invoke(this);

		return false;
	}
}

class KOTH_VehicleSelectionTileBase : SCR_ButtonImageComponent
{
};

//------------------------------------------------------------------------------------------------
class KOTH_VehicleSelectionTile : KOTH_VehicleSelectionTileBase
{
	protected TextWidget m_wText;
	protected KOTH_VehicleSelectionTileSelection m_Parent;
	protected bool m_bClickEnabled = false;

	//------------------------------------------------------------------------------------------------
	protected void EnableButton()
	{
		m_bClickEnabled = true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_bClickEnabled)
			return false;

		super.OnClick(w, x, y, button);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wText = TextWidget.Cast(w.FindAnyWidget("Text"));
		GetGame().GetCallqueue().CallLater(EnableButton, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	void ShowTile(bool show)
	{
		GetRootWidget().SetVisible(show);
		GetRootWidget().SetEnabled(show);
	}

	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		if (m_wText)
			m_wText.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		if (m_Parent)
			m_Parent.FocusTile(this);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		if (m_Parent)
			m_Parent.FocusTile(null);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void SetParent(KOTH_VehicleSelectionTileSelection parent)
	{
		m_Parent = parent;
	}
};
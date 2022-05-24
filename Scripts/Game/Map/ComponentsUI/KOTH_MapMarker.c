class KOTH_MapMarker
{
	protected Widget m_ParentRoot;
	protected Widget m_Root;
	protected HorizontalLayoutWidget m_Horizontal;
	protected ImageWidget m_Icon;
	protected RichTextWidget m_Text;
	protected vector m_Position;

	void KOTH_MapMarker(Widget parent, vector pos)
	{
		m_ParentRoot = parent;
		m_Position = pos;

		m_Root = GetGame().GetWorkspace().CreateWidgets("{380240BDB1F22EAF}UI/layouts/HUD/KOTH/KOTHMapMarker.layout", parent);
		m_Horizontal = HorizontalLayoutWidget.Cast(m_Root.FindAnyWidget("HorizontalSpacerWidget"));
		m_Icon = ImageWidget.Cast(m_Root.FindAnyWidget("Icon"));
		m_Text = RichTextWidget.Cast(m_Root.FindAnyWidget("Label"));
	}

	void ~KOTH_MapMarker()
	{
		if (m_Root)
			m_Root.RemoveFromHierarchy();
	}

	void SetColor(Color color)
	{
		m_Icon.SetColor(color);
		m_Text.SetColor(color);
	}

	void SetLabel(string text)
	{
		m_Text.SetText(text);
	}

	void SetIcon(ResourceName iconPath)
	{
		m_Icon.LoadImageTexture(0, iconPath);
	}

	void SetIconFromSet(ResourceName setName, string iconName)
	{
		m_Icon.LoadImageFromSet(0, setName, iconName);
	}
	
	void SetIconSize(float x, float y)
	{
		vector horizontalWSize = FrameSlot.GetSize(m_Horizontal);
		FrameSlot.SetSize(m_Horizontal, horizontalWSize[0], y);
		vector rootWSize = FrameSlot.GetSize(m_Root);
		FrameSlot.SetSize(m_Root, rootWSize[0], y);
		FrameSlot.SetSize(m_Icon, x, y);
	}

	void Update()
	{
		if (!m_Root) return;

		float x, y;
		SCR_MapEntity.GetMapInstance().WorldToScreen(m_Position[0], m_Position[2], x, y, true);

		x = GetGame().GetWorkspace().DPIUnscale(x);
		y = GetGame().GetWorkspace().DPIUnscale(y);

		FrameSlot.SetPos(m_Root, x, y);
	}
};
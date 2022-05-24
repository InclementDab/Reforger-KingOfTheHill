class KOTH_MapMarker
{
	protected Widget m_ParentRoot;
	protected Widget m_Marker;
	protected ImageWidget m_Icon;
	protected RichTextWidget m_Text;
	protected vector m_Position;

	void KOTH_MapMarker(Widget parent, vector pos)
	{
		m_ParentRoot = parent;
		m_Position = pos;

		m_Marker = GetGame().GetWorkspace().CreateWidgets("{380240BDB1F22EAF}UI/layouts/HUD/KOTH/KOTHMapMarker.layout", parent);
		m_Icon = ImageWidget.Cast(m_Marker.FindAnyWidget("Icon"));
		m_Text = RichTextWidget.Cast(m_Marker.FindAnyWidget("Label"));
		
		FrameSlot.SetSize(m_Icon, 32, 32);
	}

	void ~KOTH_MapMarker()
	{
		if (m_Marker)
			m_Marker.RemoveFromHierarchy();
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

	void Update()
	{
		if (!m_Marker) return;

		float x, y;
		SCR_MapEntity.GetMapInstance().WorldToScreen(m_Position[0], m_Position[2], x, y, true);

		x = GetGame().GetWorkspace().DPIUnscale(x);
		y = GetGame().GetWorkspace().DPIUnscale(y);

		FrameSlot.SetPos(m_Marker, x, y);
	}
};
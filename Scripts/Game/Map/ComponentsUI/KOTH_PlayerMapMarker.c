class KOTH_PlayerMapMarker
{
	protected Widget m_ParentRoot;
	protected Widget m_Marker;
	protected ImageWidget m_Icon;
	protected RichTextWidget m_Text;

	protected ChimeraCharacter m_Player;

	void KOTH_PlayerMapMarker(Widget parent, ChimeraCharacter player)
	{
		m_ParentRoot = parent;
		m_Player = player;

		m_Marker = GetGame().GetWorkspace().CreateWidgets("{380240BDB1F22EAF}UI/layouts/HUD/KOTH/KOTHMapMarker.layout", parent);
		m_Icon = ImageWidget.Cast(m_Marker.FindAnyWidget("Icon"));
		m_Text = RichTextWidget.Cast(m_Marker.FindAnyWidget("Label"));
	}

	void ~KOTH_PlayerMapMarker()
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
	
	void SetIconSize(float x, float y)
	{
		FrameSlot.SetSize(m_Icon, x, y);
	}

	void Update()
	{
		if (!m_Player || !m_Marker)
			return;

		vector playerPos = m_Player.GetOrigin();
		vector playerDir = m_Player.GetLocalYawPitchRoll();

		float x, y;
		SCR_MapEntity.GetMapInstance().WorldToScreen(playerPos[0], playerPos[2], x, y, true);

		x = GetGame().GetWorkspace().DPIUnscale(x);
		y = GetGame().GetWorkspace().DPIUnscale(y);

		FrameSlot.SetPos(m_Marker, x, y);
		m_Icon.SetRotation(Math.Round(Math.MapAngle(playerDir[0])));
	}
};
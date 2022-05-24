class KOTH_MapUIComponentPlayerPosition : SCR_MapUIBaseComponent
{
	//protected Widget m_widget;
	protected SCR_MapToolEntry m_ToolMenuEntry;
	
	protected Widget m_PlayerMarker;
	protected ImageWidget m_ArrowIcon;
	protected RichTextWidget m_Text;
	protected ChimeraCharacter m_Player;
	protected bool m_Enabled = true;

	override void Update()
	{
		super.Update();

		if (!m_PlayerMarker || !m_Player)
			return;
		
		vector playerPos = m_Player.GetOrigin();
		vector playerDir = m_Player.GetLocalYawPitchRoll();
		
		float x, y;
		m_MapEntity.WorldToScreen(playerPos[0], playerPos[2], x, y, true);

		x = GetGame().GetWorkspace().DPIUnscale(x);
		y = GetGame().GetWorkspace().DPIUnscale(y);
		
		FrameSlot.SetPos(m_PlayerMarker, x, y);
		m_ArrowIcon.SetRotation(Math.Round(Math.MapAngle(playerDir[0])));
	}

	override void OnMapClose(MapConfiguration config)
	{
		super.OnMapClose(config);
		
		if (m_PlayerMarker)
			m_PlayerMarker.RemoveFromHierarchy();
	}

	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);

		if (!m_RootWidget)
			return;
		
		m_Player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		m_PlayerMarker = GetGame().GetWorkspace().CreateWidgets("{DE9B818C6BA4E01C}UI/layouts/KOTHPlayerPositionMarker.layout", m_RootWidget);
		m_ArrowIcon = ImageWidget.Cast(m_PlayerMarker.FindAnyWidget("Icon"));
		m_Text = RichTextWidget.Cast(m_PlayerMarker.FindAnyWidget("Name"));
		m_ArrowIcon.SetColor(Color.FromRGBA(34, 112, 147, 255));
		m_Text.SetColor(Color.FromRGBA(34, 112, 147, 255));
	}

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
	}
};

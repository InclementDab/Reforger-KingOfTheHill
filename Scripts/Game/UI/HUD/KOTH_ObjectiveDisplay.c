class KOTH_ObjectiveDisplayObject
{
	protected Widget m_Root;
	protected ImageWidget m_wWaypoint;
	protected RichTextWidget m_wWaypointDistance;
	protected ChimeraCharacter m_Player;
	
	protected KOTH_GameModeBase m_KOTHGameMode;
	protected KOTH_ZoneManager m_ZoneManager;
	protected KOTH_HUDDisplay m_KOTHHUD;
	protected ScoreDiplayObjectBlinkState m_BlinkState;

	void KOTH_ObjectiveDisplayObject(notnull Widget root, KOTH_HUDDisplay hud)
	{
		m_Root = root;
		m_KOTHHUD = hud;
		m_Player = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		
		m_KOTHGameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		m_ZoneManager = m_KOTHGameMode.GetKOTHZoneManager();
		
		m_wWaypoint = ImageWidget.Cast(m_Root.FindAnyWidget("Icon"));
		m_wWaypointDistance = RichTextWidget.Cast(m_Root.FindAnyWidget("Distance"));
		
		if (m_wWaypoint)
		{
			m_wWaypoint.SetOpacity(0);
			
			if (GetHUD().m_r3DMarkerIconName != string.Empty && GetHUD().m_r3DMarkerIcon != ResourceName.Empty) {
				m_wWaypoint.LoadImageFromSet(0, GetHUD().m_r3DMarkerIcon, GetHUD().m_r3DMarkerIconName);
			} else if (GetHUD().m_r3DMarkerIconName == string.Empty && GetHUD().m_r3DMarkerIcon != ResourceName.Empty) {
				m_wWaypoint.LoadImageTexture(0, GetHUD().m_r3DMarkerIcon);
			}
			
			if (GetHUD().m_bUse3DMarkerColor)
				m_wWaypoint.SetColor(GetHUD().m_i3DMarkerColor);
			
			FrameSlot.SetSize(m_wWaypoint, GetHUD().m_f3DMarkerIconSize, GetHUD().m_f3DMarkerIconSize);
		}
		
		if (m_wWaypointDistance)
		{
			m_wWaypointDistance.SetOpacity(0);
			m_wWaypointDistance.SetColor(GetHUD().m_i3DMarkerTextColor);
		}
	}
		
	void UpdateObjectiveDisplay()
	{
		// Render and update objeczive waypoint if allowed
		if (!m_wWaypoint || !m_ZoneManager)
			return;
		
		vector WPPos = m_ZoneManager.GetZone().GetWorldZoneCenter();
		WPPos[1] = WPPos[1] + 1;
		vector pos = GetGame().GetWorkspace().ProjWorldToScreen(WPPos, GetGame().GetWorld());
		int dist = vector.Distance(m_Player.GetOrigin(), WPPos);

		// Handle off-screen coords
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		int winX = workspace.GetWidth();
		int winY = workspace.GetHeight();
		int posX = workspace.DPIScale(pos[0]);
		int posY = workspace.DPIScale(pos[1]);

		if (posX < 0)
			pos[0] = 0;
		else if (posX > winX)
			pos[0] = workspace.DPIUnscale(winX);

		if (posY < 0)
			pos[1] = 0;
		else if (posY > winY || pos[2] < 0)
			pos[1] = workspace.DPIUnscale(winY);

		FrameSlot.SetPos(m_wWaypoint, pos[0], pos[1]);
		FrameSlot.SetPos(m_wWaypointDistance, pos[0], pos[1]);
		m_wWaypoint.SetOpacity(0.5);
		m_wWaypointDistance.SetTextFormat("#AR-Tutorial_WaypointUnits_meters", dist);
		m_wWaypointDistance.SetOpacity(0.5);
	}
	
	KOTH_HUDDisplay GetHUD()
	{
		return m_KOTHHUD;
	}
};

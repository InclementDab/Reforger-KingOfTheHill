
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
			
		m_Marker = GetGame().GetWorkspace().CreateWidgets("{1AF7B03852C313F6}UI/layouts/KOTHMapMarker.layout", parent);
		m_Icon = ImageWidget.Cast(m_Marker.FindAnyWidget("Icon"));
		m_Text = RichTextWidget.Cast(m_Marker.FindAnyWidget("Label"));
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
		float x, y;
		SCR_MapEntity.GetMapInstance().WorldToScreen(m_Position[0], m_Position[2], x, y, true);

		x = GetGame().GetWorkspace().DPIUnscale(x);
		y = GetGame().GetWorkspace().DPIUnscale(y);
		
		FrameSlot.SetPos(m_Marker, x, y);
	}
};

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

		m_Marker = GetGame().GetWorkspace().CreateWidgets("{1AF7B03852C313F6}UI/layouts/KOTHMapMarker.layout", parent);
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

class KOTH_MapUIComponenMapMarkers : SCR_MapUIBaseComponent
{
	protected ref array<ref KOTH_MapMarker> m_MapMarkers = new array<ref KOTH_MapMarker>;
	protected ref KOTH_PlayerMapMarker m_PlayerMarker;
	protected ChimeraCharacter m_Player;
	protected bool m_Enabled = true;
	
	protected KOTH_GameModeBase m_GameMode;
	protected KOTH_ZoneManager m_ZoneManager;
	
	override void Update()
	{
		super.Update();
		
		if (m_PlayerMarker) m_PlayerMarker.Update();
		
		if (m_MapMarkers && m_MapMarkers.Count() > 0)
		{
			foreach (KOTH_MapMarker mapMarker: m_MapMarkers)
			{		
				mapMarker.Update();
			}
		}
	}

	override void OnMapClose(MapConfiguration config)
	{
		super.OnMapClose(config);
		
		if (m_PlayerMarker)
			delete m_PlayerMarker;
		
		
	}

	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);

		if (!m_RootWidget)
			return;
		
		if (!m_PlayerMarker)
		{
			m_Player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
			m_PlayerMarker = new KOTH_PlayerMapMarker(m_RootWidget, m_Player);
		}
		
		foreach (KOTH_SafeZoneTriggerEntity safeZone: m_ZoneManager.GetSafeZones())
		{
			KOTH_MapMarker safeZoneMarker = new KOTH_MapMarker(m_RootWidget, safeZone.GetWorldSafeZoneCenter());		
			m_MapMarkers.Insert(safeZoneMarker);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
		
		m_GameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		if (!m_GameMode) {
			Print("Could not find game mode!", LogLevel.ERROR);
			return;
		}

		m_ZoneManager = m_GameMode.GetKOTHZoneManager();
		if (!m_ZoneManager) {
			Print("Could not find zone manager!", LogLevel.ERROR);
			return;
		}
	}
};

[ComponentEditorProps(category: "GameScripted/GameMode/KOTH", description: "Manager component allowing access and API over CaptureAndHold areas.")]
class KOTH_GameModeBaseClass : SCR_BaseGameModeClass
{
}

class KOTH_GameModeBase: SCR_BaseGameMode
{
	//! If enabled custom weather Id will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, custom weather Id will be used. Authority only.", category: "CaptureAndHold: Environment")]
	protected bool m_bUseCustomWeather;

	//! Weather IDs are the same as used in the TimeAndWeatherManager. Weather set on game start. Authority only.
	[Attribute(defvalue: "", desc: "Weather IDs are the same as used in the TimeAndWeatherManager. Weather set on game start. Authority only.", category: "CaptureAndHold: Environment")]
	protected string m_sCustomWeatherId;

	//! If enabled custom time of the day will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, custom time of the day will be used. Authority only.", category: "CaptureAndHold: Environment")]
	protected bool m_bUseCustomTime;

	//! Time of the day set on game start. Authority only.
	[Attribute(defvalue: "12", desc: "Time of the day set on game start. Authority only.", category: "CaptureAndHold: Environment", params: "0 24 0.01")]
	protected float m_fCustomTimeOfTheDay;

	protected ImageWidget m_wWaypoint;
	protected RichTextWidget m_wWaypointDistance;
	protected ChimeraCharacter m_Player;
	
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);

		m_Player = ChimeraCharacter.Cast(controlledEntity);
		
		// start game
		if (GetState() == SCR_EGameModeState.PREGAME) {
			StartGameMode();
		}
		
		// Load waypoint UI upon first spawn
		if (!m_wWaypoint)
		{
			Widget waypointFrame = GetGame().GetHUDManager().CreateLayout("{EEDBCD234A118D9F}UI/layouts/HUD/KOTH/KOTHWaypoint.layout", EHudLayers.BACKGROUND);
			m_wWaypoint = ImageWidget.Cast(waypointFrame.FindAnyWidget("Icon"));
			m_wWaypointDistance = RichTextWidget.Cast(waypointFrame.FindAnyWidget("Distance"));
			
			if (m_wWaypoint)
			{
				m_wWaypoint.SetOpacity(0);
				m_wWaypoint.SetColor(Color.FromRGBA(179, 57, 57, 240));
				FrameSlot.SetSize(m_wWaypoint, 46, 46);
			}
			
			if (m_wWaypointDistance)
			{
				m_wWaypointDistance.SetOpacity(0);
				m_wWaypointDistance.SetColor(Color.FromRGBA(179, 57, 57, 240));
			}
		}
	}

	KOTH_ZoneManager GetKOTHZoneManager()
	{
		return KOTH_ZoneManager.Cast(FindComponent(KOTH_ZoneManager));
	}
	
	void KOTH_GameModeBase(IEntitySource src, IEntity parent)
	{
		// This is not the best way of solving this problem,
		// but for a small game mode like this it's completely fine.
		ScriptInvoker onMapOpenInvoker = SCR_MapEntity.GetOnMapOpen();
		if (onMapOpenInvoker) {
			onMapOpenInvoker.Insert(OnMapOpen);
		}
	}

	void ~KOTH_GameModeBase()
	{
		ScriptInvoker onMapOpenInvoker = SCR_MapEntity.GetOnMapOpen();
		if (onMapOpenInvoker)
			onMapOpenInvoker.Remove(OnMapOpen);
		
		if (m_wWaypoint)
			m_wWaypoint.RemoveFromHierarchy();
		
		if (m_wWaypointDistance)
			m_wWaypointDistance.RemoveFromHierarchy();
	}

	protected void DoPanZoomMap(float x, float z, float zoom)
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;

		// Zoom and pan to objectives almost immediately
		mapEntity.ZoomPanSmooth(zoom, x, z, 0.001);
	}

	protected void OnMapOpen(MapConfiguration config)
	{
		// Get average of all positions
		/*if (!m_Area) return;
		
		float x;
		float z;
		vector worldPos = m_Area.GetWorldObjectiveCenter();
		x += worldPos[0];
		z += worldPos[2];
		
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;

		CanvasWidget mapWidget = mapEntity.GetMapWidget();
		vector usize = mapWidget.GetSizeInUnits();
		float zoomVal = usize[0] / (usize[0] * mapWidget.PixelPerUnit());

		// Unfortunately we need to "override" the default respawn menu focus,
		// currently not aware of a nicer way - perhaps it will begone in some future update :)
		GetGame().GetCallqueue().CallLater(DoPanZoomMap, 100, false, x, z, zoomVal);*/
	}
	
	protected void SetWeather(string weatherId)
	{
		if (!IsMaster())
			return;

		if (weatherId.IsEmpty())
			return;

		TimeAndWeatherManagerEntity weatherManager = GetGame().GetTimeAndWeatherManager();
		if (!weatherManager)
		{
			Print("Cannot initialize weather: TimeAndWeatherManagerEntity not found!", LogLevel.WARNING);
			return;
		}

		weatherManager.ForceWeatherTo(true, weatherId, 0.0);
	}

	protected void SetTimeOfTheDay(float timeOfTheDay)
	{
		if (!IsMaster())
			return;

		TimeAndWeatherManagerEntity weatherManager = GetGame().GetTimeAndWeatherManager();
		if (!weatherManager)
		{
			Print("Cannot initialize TimeOfTheDay: TimeAndWeatherManagerEntity not found!", LogLevel.WARNING);
			return;
		}

		weatherManager.SetTimeOfTheDay(timeOfTheDay, true);
	}
	
	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (m_bUseCustomTime)
			SetTimeOfTheDay(m_fCustomTimeOfTheDay);

		if (m_bUseCustomWeather)
			SetWeather(m_sCustomWeatherId);
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{		
		// Render waypoint if allowed
		if (m_wWaypoint)
		{
			vector WPPos = GetKOTHZoneManager().GetZone().GetWorldZoneCenter();
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
			m_wWaypoint.SetOpacity(1);
			m_wWaypointDistance.SetTextFormat("#AR-Tutorial_WaypointUnits_meters", dist);
			m_wWaypointDistance.SetOpacity(1);
		}
		else
		{
			m_wWaypoint.SetOpacity(0);
			m_wWaypointDistance.SetOpacity(0);
		}
	}
}
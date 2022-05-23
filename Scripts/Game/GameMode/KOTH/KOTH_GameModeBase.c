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

	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);

		// start game
		if (GetState() == SCR_EGameModeState.PREGAME) {
			StartGameMode();
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
}
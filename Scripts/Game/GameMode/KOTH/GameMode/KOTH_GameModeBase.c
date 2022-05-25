[ComponentEditorProps(category: "GameScripted/GameMode/KOTH", description: "Manager component allowing access and API over KOTH zones.")]
class KOTH_GameModeBaseClass : SCR_BaseGameModeClass
{
}

class KOTH_GameModeBase: SCR_BaseGameMode
{
	[Attribute(defvalue: "0", desc: "If enabled, the KOTH_MapUIComponenMapMarkers will be used and safe zone and objective markers get created on the players map.", category: "KOTH: Settings")]
	protected bool m_bEnableMapUIComponent;
	
	[Attribute(defvalue: "0", desc: "If enabled, players will see a 3D waypoint marker on the objective zone location on there UI.", category: "KOTH: Settings")]
	protected bool m_bEnable3DObjectiveMarker;
	
	[Attribute(defvalue: "0", desc: "If enabled then the defined marker color will be applied on the 3D marker icon. If not the basic image color will be used.", category: "KOTH: Settings")]
	protected bool m_bUse3DMarkerColor;
	[Attribute("0.000000 0.616999 0.583993 1.000000", UIWidgets.ColorPicker, desc: "Main color that will be used for the 3D objective marker.", category: "KOTH: Settings")]
	protected ref Color m_i3DMarkerColor;
	[Attribute("0.000000 0.616999 0.583993 1.000000", UIWidgets.ColorPicker, desc: "Main color that will be used for the 3D objective marker distance text.", category: "KOTH: Settings")]
	protected ref Color m_i3DMarkerTextColor;
	
	[Attribute("{DE969B7C3B7BBBCA}UI/icons/objective_marker.edds", UIWidgets.ResourceNamePicker, desc: "Main icon or imageset that will be used for the the 3D objective marker.", category: "KOTH: Settings", params: "edds imageset")]
	protected ResourceName m_r3DMarkerIcon;
	[Attribute("", UIWidgets.EditBox , desc: "Imageset icon name if imageset is used for the the 3D objective marker.", category: "KOTH: Settings")]
	protected string m_r3DMarkerIconName;
	[Attribute("34.0", UIWidgets.EditBox , desc: "Size of the marker icon used for the the 3D objective marker.", category: "KOTH: Settings")]
	protected float m_f3DMarkerIconSize;
	
	//! If enabled custom weather Id will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, custom weather Id will be used. Authority only.", category: "KOTH: Environment")]
	protected bool m_bUseCustomWeather;

	//! Weather IDs are the same as used in the TimeAndWeatherManager. Weather set on game start. Authority only.
	[Attribute(defvalue: "", desc: "Weather IDs are the same as used in the TimeAndWeatherManager. Weather set on game start. Authority only.", category: "KOTH: Environment")]
	protected string m_sCustomWeatherId;

	//! If enabled custom time of the day will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, custom time of the day will be used. Authority only.", category: "KOTH: Environment")]
	protected bool m_bUseCustomTime;

	//! Time of the day set on game start. Authority only.
	[Attribute(defvalue: "12", desc: "Time of the day set on game start. Authority only.", category: "KOTH: Environment", params: "0 24 0.01")]
	protected float m_fCustomTimeOfTheDay;
	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Vehicle asset list", "conf", NULL, category: "KOTH: Configuration")]
	protected ResourceName m_VehicleAssetList;
	protected ref array<ref KOTH_VehicleAssetInfo> m_aVehicleAssetList;

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
		
		LoadObjectiveUI();
	}
	
	private void LoadObjectiveUI()
	{
		// Load objective waypoint UI upon first spawn
		if (!m_wWaypoint && m_bEnable3DObjectiveMarker)
		{
			Widget waypointFrame = GetGame().GetHUDManager().CreateLayout("{EEDBCD234A118D9F}UI/layouts/HUD/KOTH/KOTHWaypoint.layout", EHudLayers.BACKGROUND);
			m_wWaypoint = ImageWidget.Cast(waypointFrame.FindAnyWidget("Icon"));
			m_wWaypointDistance = RichTextWidget.Cast(waypointFrame.FindAnyWidget("Distance"));
			
			if (m_wWaypoint)
			{
				m_wWaypoint.SetOpacity(0);
				
				if (m_r3DMarkerIconName != string.Empty && m_r3DMarkerIcon != ResourceName.Empty) {
					m_wWaypoint.LoadImageFromSet(0, m_r3DMarkerIcon, m_r3DMarkerIconName);
				} else if (m_r3DMarkerIconName == string.Empty && m_r3DMarkerIcon != ResourceName.Empty) {
					m_wWaypoint.LoadImageTexture(0, m_r3DMarkerIcon);
				}
				
				if (m_bUse3DMarkerColor)
					m_wWaypoint.SetColor(m_i3DMarkerColor);
				
				FrameSlot.SetSize(m_wWaypoint, m_f3DMarkerIconSize, m_f3DMarkerIconSize);
			}
			
			if (m_wWaypointDistance)
			{
				m_wWaypointDistance.SetOpacity(0);
				m_wWaypointDistance.SetColor(m_i3DMarkerTextColor);
			}
		}
	}

	KOTH_ZoneManager GetKOTHZoneManager()
	{
		return KOTH_ZoneManager.Cast(FindComponent(KOTH_ZoneManager));
	}
	
	void KOTH_GameModeBase(IEntitySource src, IEntity parent)
	{		
		//Parse & register vehicle asset list
		m_aVehicleAssetList = new array<ref KOTH_VehicleAssetInfo>;
		Resource container = BaseContainerTools.LoadContainer(m_VehicleAssetList);
		if (container && container.IsValid())
		{
			KOTH_VehicleAssetList list = KOTH_VehicleAssetList.Cast(BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer()));
			list.GetVehicleAssetList(m_aVehicleAssetList);
		}
	}

	void ~KOTH_GameModeBase()
	{	
		if (m_bEnable3DObjectiveMarker)
		{
			if (m_wWaypoint)
				m_wWaypoint.RemoveFromHierarchy();
			
			if (m_wWaypointDistance)
				m_wWaypointDistance.RemoveFromHierarchy();
		}
	}

	protected void DoPanZoomMap(float x, float z, float zoom)
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;

		// Zoom and pan to objectives almost immediately
		mapEntity.ZoomPanSmooth(zoom, x, z, 0.001);
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
		if (m_bEnable3DObjectiveMarker)
			UpdateObjectiveUI();
	}
	
	private void UpdateObjectiveUI()
	{
		// Render and update objeczive waypoint if allowed
		if (!m_wWaypoint)
			return;
		
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
	
	//------------------------------------------------------------------------------------------------
	//! Returns the asset list for the match (list of all vehicles available for players to request)
	//! \param assetList Array to fill with data
	void GetVehicleAssetList(out notnull array<ref KOTH_VehicleAssetInfo> assetList)
	{
		assetList = m_aVehicleAssetList
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the total number of vehicles in the asset list
	int GetVehicleAssetListCount()
	{
		return m_aVehicleAssetList.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Get the vehicle name to be shown in UI
	//! \param assetID Unique asset ID (its index in asset list)
	string GetVehicleAssetDisplayName(int assetID)
	{
		return m_aVehicleAssetList[assetID].GetDisplayName();
	}

	//------------------------------------------------------------------------------------------------
	string GetVehicleAssetDisplayNameUpperCase(int assetID)
	{
		return m_aVehicleAssetList[assetID].GetDisplayNameUpperCase();
	}

	//------------------------------------------------------------------------------------------------
	//! Get the vehicle prefab used to spawn the vehicle
	//! \param assetID Unique asset ID (its index in asset list)
	ResourceName GetVehicleAssetPrefab(int assetID)
	{
		return m_aVehicleAssetList[assetID].GetPrefab();
	}
	
	bool UseMapMarkerComponent()
	{
		return m_bEnableMapUIComponent;
	}
}
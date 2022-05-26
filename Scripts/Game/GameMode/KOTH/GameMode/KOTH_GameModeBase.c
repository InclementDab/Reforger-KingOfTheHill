[ComponentEditorProps(category: "GameScripted/GameMode/KOTH", description: "Manager component allowing access and API over KOTH zones.")]
class KOTH_GameModeBaseClass : SCR_BaseGameModeClass
{
}

class KOTH_GameModeBase: SCR_BaseGameMode
{
	[Attribute(defvalue: "0", desc: "If enabled, the KOTH_MapUIComponenMapMarkers will be used and safe zone and objective markers get created on the players map.", category: "KOTH: Settings")]
	protected bool m_bEnableMapUIComponent;
		
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
	
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);
		
		// start game
		if (GetState() == SCR_EGameModeState.PREGAME) {
			StartGameMode();
		}
	}
	
	override void OnPlayerAuditSuccess(int iPlayerID)
	{
		super.OnPlayerAuditSuccess(iPlayerID);
		
		if (!Replication.IsServer()) {
			return;
		}
		
		return; // todo ids not working at this stage, fix!
		
		Print("Checking id " + iPlayerID);
		// Handle VIP slots
		PlayerManager player_manager = GetGame().GetPlayerManager();
		string player_uid = GetGame().GetBackendApi().GetPlayerUID(iPlayerID);
		KOTH_MissionHeader header = KOTH_MissionHeader.Cast(GetGame().GetMissionHeader());
		Print(player_uid);
		if (!header) {
			return; // probably offline, dont worry about it
		}
		
		Print(header.m_iPlayerCount);
		Print(player_manager.GetPlayerCount());
		Print(header.GetVIPSlotCount());
		
		if (header.m_iPlayerCount - player_manager.GetPlayerCount() > header.GetVIPSlotCount()) {
			// todo if player is VIP, return
			// if (player.VIP()) {
				//return;
			//}
			
			Print("Kicking player id " + player_uid);
			player_manager.KickPlayer(iPlayerID, PlayerManagerKickReason.KICK);
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
	
	static IEntity FindEntityByName(string entityName)
	{
		if (!GetGame())
			return null;
		
		BaseWorld world = GetGame().GetWorld();
		IEntity foundEntity = world.FindEntityByName(entityName);
		IEntity entity = null;
		
		if (foundEntity)
			entity = IEntity.Cast(foundEntity);
		
		return entity;
	}
	
	static KOTH_DeliveryPoint GetUSDeliveryPoint()
	{
		if (!GetGame())
			return null;
		
		BaseWorld world = GetGame().GetWorld();
		KOTH_DeliveryPoint usPoint = KOTH_DeliveryPoint.Cast(world.FindEntityByName("VehicleRequestSpawnUSA"));
		
		return usPoint;
	}
}
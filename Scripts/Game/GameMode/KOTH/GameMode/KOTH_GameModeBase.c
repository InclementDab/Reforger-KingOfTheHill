
class KOTH_PlayerBackendCb: BackendCallback
{	
	void KOTH_PlayerBackendCb()
	{
		Print("Hello");
	}
	
	override void OnDataReceive( string data, int size )
	{
		Print("[BackendCallback] Data received, size=" + size);
		Print(data);
	}

	/**
	\brief Request finished with error result
	\param code Error code is type of EBackendError
	*/
	override void OnError( int code, int restCode, int apiCode )
	{
		Print("[BackendCallback] OnError: "+ g_Game.GetBackendApi().GetErrorCode(code));
	}

	/**
	\brief Request finished with success result
	\param code Code is type of EBackendRequest
	*/
	override void OnSuccess( int code )
	{
		Print("[BackendCallback] OnSuccess()");
	}

	/**
	\brief Request not finished due to timeout
	*/
	override void OnTimeout()
	{
		Print("[BackendCallback] OnTimeout");
	}
}

class KOTH_GameModeBaseClass : SCR_BaseGameModeClass
{
}

#define GAME_MODE_DEBUG;
#define ENABLE_DIAG;
#define SLOT_ENTITY_DEBUG;

//#define VEHICLE_LOCK;
//#define ENABLE_BASE_DESTRUCTION;
//#define ENABLE_DESTRUCTION;

class KOTH_GameModeBase: SCR_BaseGameMode
{
	[Attribute(defvalue: "0", UIWidgets.EditBox, desc: "Position of the objective zone.", category: "KOTH: Settings")]
	protected vector m_vObjectivePosition;
	
	[Attribute(defvalue: "0", desc: "Radius of the objective zone.", category: "KOTH: Settings")]
	protected float m_vObjectiveRadius;
	
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
	
	protected ref map<int, ref KOTH_PlayerStorage> m_PlayerStorages = new map<int, ref KOTH_PlayerStorage>();		
	
	protected ref KOTH_PlayerBackendCb m_PlayerBackendCb = new KOTH_PlayerBackendCb();
	
	protected KOTH_ZoneTriggerEntity m_Zone;
	
	protected ref map<int, ref KOTH_PlayerUIData> m_PlayerUIDatas = new map<int, ref KOTH_PlayerUIData>;
	
	protected float m_PlayersUpdateTime;
	protected const float PLAYER_UPDATEQUE_TIME = 0.100;
	
	//! Callback for when a tick event is raised by this area
	protected ref KOTHZoneTickEvent m_pOnTickEvent = new KOTHZoneTickEvent();
	
	//------------------------------------------------------------------------------------------------		
	void KOTH_GameModeBase(IEntitySource src, IEntity parent)
	{
		//Parse & register vehicle asset list	
		m_aVehicleAssetList = new array<ref KOTH_VehicleAssetInfo>;
		Resource container = BaseContainerTools.LoadContainer(m_VehicleAssetList);
		if (container && container.IsValid()) {
			KOTH_VehicleAssetList list = KOTH_VehicleAssetList.Cast(BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer()));
			list.GetVehicleAssetList(m_aVehicleAssetList);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ~KOTH_GameModeBase()
	{

	}
	
	//------------------------------------------------------------------------------------------------	
	override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		super.OnPlayerKilled(playerId, player, killer);
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(player);
		GetGame().GetBackendApi().PlayerData(m_PlayerStorages[playerId], playerId);
	}
	
	//------------------------------------------------------------------------------------------------	
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);
				
		if (!Replication.IsServer()) {
			return;
		}
		
		// start game, hack
		if (GetState() == SCR_EGameModeState.PREGAME) {
			StartGameMode();
		}
			
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(controlledEntity);
		if (!character) {
			return;
		}
		
		KOTH_PlayerUIData playerUIData = new KOTH_PlayerUIData();
		playerUIData.SetPosition(character.GetOrigin());
		playerUIData.SetFactionKey(character.GetFactionKey());
		m_PlayerUIDatas.Insert(playerId, playerUIData);
		
		if (!m_PlayerStorages[playerId]) {
			m_PlayerStorages[playerId] = new KOTH_PlayerStorage();
		}
		
		GetGame().GetBackendApi().PlayerRequest(EBackendRequest.EBREQ_GAME_CharacterGet, m_PlayerBackendCb, m_PlayerStorages[playerId], playerId);
		
		// Handle VIP slots
		/*PlayerManager player_manager = GetGame().GetPlayerManager();
		string player_uid = GetGame().GetBackendApi().GetPlayerUID(playerId);
		KOTH_MissionHeader header = KOTH_MissionHeader.Cast(GetGame().GetMissionHeader());
		Print(player_uid);
		if (!header) {
			return; // probably offline, dont worry about it
		}
				
		if (header.m_iPlayerCount - player_manager.GetPlayerCount() > header.GetVIPSlotCount()) {
			// todo if player is VIP, return
			// if (player.VIP()) {
				//return;
			//}
			
			Print("Kicking player id " + player_uid);
			player_manager.KickPlayer(playerId, PlayerManagerKickReason.KICK);
		}*/
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerStorage(int id, KOTH_PlayerStorage storage)
	{
		m_PlayerStorages[id] = storage;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(id));
		character.SetStorage(storage);
	}
	
	//------------------------------------------------------------------------------------------------	
	KOTH_PlayerStorage GetPlayerStorage(int id)
	{
		return m_PlayerStorages[id];
	}
	
	//------------------------------------------------------------------------------------------------	
	KOTH_GameStorage GetKOTHGameStorage()
	{
		KOTH_SaveLoadComponent save_load = KOTH_SaveLoadComponent.Cast(FindComponent(KOTH_SaveLoadComponent));
		if (!save_load) {
			Print("No KOTH_SaveLoadComponent found on gamemode!", LogLevel.WARNING);
			return null;
		}
		
		return save_load.GetGameStorage();
	}
	
	//------------------------------------------------------------------------------------------------	
	KOTH_ZoneManager GetKOTHZoneManager()
	{
		return KOTH_ZoneManager.Cast(FindComponent(KOTH_ZoneManager));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DoPanZoomMap(float x, float z, float zoom)
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;

		// Zoom and pan to objectives almost immediately
		mapEntity.ZoomPanSmooth(zoom, x, z, 0.001);
	}
	
	//------------------------------------------------------------------------------------------------
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

	//------------------------------------------------------------------------------------------------
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
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (m_bUseCustomTime)
			SetTimeOfTheDay(m_fCustomTimeOfTheDay);

		if (m_bUseCustomWeather)
			SetWeather(m_sCustomWeatherId);

		//! Spawn zone trigger entity
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		
		vector matrix[4];
		Math3D.MatrixIdentity4(matrix);
		matrix[3] = m_vObjectivePosition;
		params.Transform = matrix;
		Resource res = Resource.Load("{DA440D2C7D1E29A7}Prefabs/MP/Modes/KOTH_ZoneTrigger.et");
		World world = GetGame().GetWorld();
		m_Zone = KOTH_ZoneTriggerEntity.Cast(GetGame().SpawnEntityPrefab(res, world, params));
		m_Zone.SetSphereRadius(m_vObjectiveRadius);
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
	
	//------------------------------------------------------------------------------------------------
	bool UseMapMarkerComponent()
	{
		return m_bEnableMapUIComponent;
	}
	
	//------------------------------------------------------------------------------------------------
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
	
	//------------------------------------------------------------------------------------------------
	static KOTH_DeliveryPoint GetUSDeliveryPoint()
	{
		if (!GetGame())
			return null;
		
		BaseWorld world = GetGame().GetWorld();
		KOTH_DeliveryPoint usPoint = KOTH_DeliveryPoint.Cast(world.FindEntityByName("VehicleRequestSpawnUSA"));
		
		return usPoint;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId)
	{
		super.OnPlayerDisconnected(playerId);
				
		if (!Replication.IsServer()) {
			return;
		}
		
		if (m_PlayerUIDatas.Contains(playerId))
			m_PlayerUIDatas.Remove(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		m_PlayersUpdateTime += timeSlice;
		if (m_PlayersUpdateTime >= PLAYER_UPDATEQUE_TIME) {
			foreach (int playerId, KOTH_PlayerUIData data: m_PlayerUIDatas) {
				IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
				if (!player)
					continue;
				
				vector position = player.GetOrigin();
				data.SetPosition(position);
				m_PlayerUIDatas.Set(playerId, data);
			}
			
			m_PlayersUpdateTime = 0;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	map<int, ref KOTH_PlayerUIData> GetPlayerUIDatas()
	{
		return m_PlayerUIDatas;
	}

	//------------------------------------------------------------------------------------------------
	/*void EndGame(Faction faction)
	{		
		int factionIndex = GetGame().GetFactionManager().GetFactionIndex(faction);
		Print(ToString() + "::EndGame - Faction Index: " + factionIndex);
		//ref SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(SCR_GameModeEndData.ENDREASON_EDITOR_FACTION_VICTORY, winnerFactionId: factionIndex);
		ref SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(SCR_GameModeEndData.ENDREASON_SCORELIMIT, winnerFactionId: factionIndex);
		EndGameMode(endData);
	}*/
	
	//------------------------------------------------------------------------------------------------
	protected override void OnGameModeEnd(SCR_GameModeEndData endData)
	{
		Print(ToString() + "::OnGameModeEnd - Start");
		
		super.OnGameModeEnd(endData);
		// Terminate the session in provided time
		GetGame().GetCallqueue().CallLater(RestartSession, 30 * 1000.0, false);
		
		Print(ToString() + "::OnGameModeEnd - End");
	}

	//------------------------------------------------------------------------------------------------
	protected void RestartSession()
	{
		Print(ToString() + "::RestartSession - Start");
		
		if (RplSession.Mode() == RplMode.Dedicated)
		{
			Print("KOTH_GameModeBase::RestartSession() - Game mode is over, terminating server session!");
		    GetGame().RequestReload();
		}
		else
		{
			Print("KOTH_GameModeBase::RestartSession() - Game mode is over, requesting gameplay end transition!");
		    GameStateTransitions.RequestServerReload();
		}
		
		Print(ToString() + "::RestartSession - End");
	}
}

class KOTH_PlayerUIData
{
	protected vector m_Position;
	protected string m_FactionKey;
	
	void KOTH_PlayerUIData()
	{
	}
	
	void ~KOTH_PlayerUIData()
	{
	}
	
	void SetPosition(vector pos)
	{
		m_Position = pos;
	}
	
	vector GetPosition()
	{
		return m_Position;
	}
	
	void SetFactionKey(string key)
	{
		m_FactionKey = key;
	}
	
	string GetFactionKey()
	{
		return m_FactionKey;
	}
}
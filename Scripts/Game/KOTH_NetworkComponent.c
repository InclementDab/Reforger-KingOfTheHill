//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/KOTH", description: "Handles client > server communication in KOTH. Should be attached to PlayerController.", color: "0 0 255 255")]
class KOTH_NetworkComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! Used to identify various notifications for client
enum EKOTHClientNotificationID
{
	VEHICLE_SPAWNED,
	COOLDOWN,
	ZONE_BLOCKED
};

class KOTH_NetworkComponent : ScriptComponent
{
	[RplProp(condition: RplCondition.OwnerOnly)]
	protected float m_fLastAssetRequestTimestamp = -int.MAX;
	
	// Member variables
	protected SCR_PlayerController m_PlayerController;
	protected RplComponent m_RplComponent;
	protected KOTH_GameModeBase m_KOTHGameMode;
	protected const int VEHICLE_REQUEST_COOLDOWN = 5;
	protected bool m_bIsSpawnZoneFree = false;
	protected ref array<Vehicle> m_VehiclesInZone = {};
	
	//------------------------------------------------------------------------------------------------
	static KOTH_NetworkComponent GetKOTHNetworkComponent(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);

		if (!playerController)
			return null;

		KOTH_NetworkComponent networkComponent = KOTH_NetworkComponent.Cast(playerController.FindComponent(KOTH_NetworkComponent));

		return networkComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	void OpenKOTHMenu()
	{
		Rpc(RpcDo_OnOpenKOTHMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnVehicle(KOTH_DeliveryPoint vehicleSpawnPoint, int assetID)
	{
		Print(ToString() + "::SpawnVehicle - Start");
		
		//RplId spawnPointID = Replication.FindId(vehicleSpawnPoint);
		int pointID = vehicleSpawnPoint.GetDeliveryPointID();
				
		if (!m_PlayerController)
			return;
		
		int playerID = m_PlayerController.GetPlayerId();
		Rpc(RpcAsk_SpawnVehicle, pointID, assetID, playerID);
		
		Print(ToString() + "::SpawnVehicle - End");
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CanRequestVehicle(int spawnPointID)
	{
		if (!GetGame().AreGameFlagsSet(EGameFlags.SpawnVehicles))
			return false;
	
		float fTimeout = m_fLastAssetRequestTimestamp + GetVehicleRequestCooldown();
		if (fTimeout > Replication.Time()) {
			RpcDo_PlayerFeedbackImpl(EKOTHClientNotificationID.COOLDOWN);
			return false;
		}
		
		KOTH_DeliveryPoint spawnPoint;
		if (spawnPointID == 1)
			spawnPoint = m_KOTHGameMode.GetUSDeliveryPoint();
		
		m_VehiclesInZone.Clear();
		
		GetGame().GetWorld().QueryEntitiesBySphere(spawnPoint.GetOrigin(), 20, QueryNearSpawnPoint, null, EQueryEntitiesFlags.ALL);
		
		if (m_VehiclesInZone.Count() > 0) {
			RpcDo_PlayerFeedbackImpl(EKOTHClientNotificationID.ZONE_BLOCKED);
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool QueryNearSpawnPoint(IEntity entity)
	{
		Print(ToString() + "::QueryNearSpawnPoint - Enity: " + entity.ToString());
		
		Vehicle veh = Vehicle.Cast(entity);
		if (veh) {
			Print(ToString() + "::QueryNearSpawnPoint - Vehicle: " + veh.ToString());
			m_VehiclesInZone.Insert(veh);
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void SendNotificationPlayer(ENotification messageID, int playerID)
	{		
		SCR_NotificationsComponent.SendToPlayer(playerID, messageID);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if the session is run as client
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//***********//
	//RPC METHODS//
	//***********//
	
	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	//! \param msgID Message ID (see ECampaignClientNotificationID)
	protected void RpcDo_PlayerFeedbackImpl(int msgID, float value = 0, int assetID = -1)
	{
		LocalizedString msg;
		LocalizedString msg2;
		int duration = 2;
		int prio = -1;
		string msg1param1;
		string msg2param1;
		string msg2param2;
		
		switch (msgID) {
			case EKOTHClientNotificationID.VEHICLE_SPAWNED: {
				msg = "#AR-Campaign_VehicleReady-UC";
				msg2 = m_KOTHGameMode.GetVehicleAssetDisplayName(assetID);
								
				SCR_UISoundEntity.SoundEvent("SOUND_LOADSUPPLIES");
				break;
			};
			case EKOTHClientNotificationID.COOLDOWN: {
				float fTimeout = m_fLastAssetRequestTimestamp + GetVehicleRequestCooldown();
				float timediff = fTimeout - Replication.Time();
				msg = "VEHICLE SPAWN COOLDOWN";
				msg2 = "You can spawn your next vehicle in " + FormatTime(timediff, false);
								
				SCR_UISoundEntity.SoundEvent("SOUND_HUD_NOTIFICATION");
				break;
			};
			case EKOTHClientNotificationID.ZONE_BLOCKED: {
				msg = "SPAWN ZONE BLOCKED";
				msg2 = "A other vehicle is blocking the spawn zone!";
								
				SCR_UISoundEntity.SoundEvent("SOUND_HUD_NOTIFICATION");
				break;
			};
			default: {return;};
		}
		
		SCR_PopUpNotification.GetInstance().PopupMsg(msg, duration, 0.5, msg2, param1: msg1param1, text2param1: msg2param1, text2param2: msg2param2);
	}
	
	//------------------------------------------------------------------------------------------------
	protected string FormatTime( float time, bool include_ms = true )
	{
		return FormatTimestamp(time / 1000, include_ms);
	}
	
	//------------------------------------------------------------------------------------------------	
	protected string FormatTimestamp(float time, bool include_ms = true)
	{
		int hours = (int) time / 3600;
		time -= hours * 3600;
		int minutes = (int) time / 60;
		time -= minutes * 60;
		int seconds = (int) time;
		
		string timestring = hours.ToString(2) + ":" + minutes.ToString(2) + ":" + seconds.ToString(2);

		if (include_ms) {
			time -= seconds;
			int ms = time * 1000;
			timestring += "." + ms.ToString(3);
		}

		return timestring;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetLastAssetRequestTimestamp()
	{
		return m_fLastAssetRequestTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------	
	protected int GetVehicleRequestCooldown()
	{
		return VEHICLE_REQUEST_COOLDOWN * 1000;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawn a vehicle at given depot
	//! \param deliveryPointID Delivery point entity ID
	//! \param assetID Unique asset ID (its index in asset list)
	//! \param playerID Requester entity ID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SpawnVehicle(int spawnPointID, int assetID, int playerID)
	{
		Print(ToString() + "::RpcAsk_SpawnVehicle - Start");
		
		if (!CanRequestVehicle(spawnPointID))
			return;
		
		KOTH_DeliveryPoint spawnPoint;
		if (spawnPointID == 1)
			spawnPoint = m_KOTHGameMode.GetUSDeliveryPoint();

		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);

		if (!player) {
			Print(ToString() + "::RpcAsk_SpawnVehicle - F 2");
			return;
		}
		
		array<ref KOTH_VehicleAssetInfo> vehicles = new array<ref KOTH_VehicleAssetInfo>;
		m_KOTHGameMode.GetVehicleAssetList(vehicles);
		Resource res = Resource.Load(vehicles[assetID].GetPrefab());
		if (!res) {
			Print(ToString() + "::RpcAsk_SpawnVehicle - F 3");
			return;
		}
				
		// Spawn the vehicle
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		spawnPoint.GetWorldTransform(params.Transform);
		Vehicle veh = Vehicle.Cast(GetGame().SpawnEntityPrefab(res, null, params));

		if (!veh) {
			Print(ToString() + "::RpcAsk_SpawnVehicle - F 4");
			return;
		}
		
		Physics physicsComponent = veh.GetPhysics();
		if (!physicsComponent)
			return;

		veh.GetPhysics().SetVelocity("0 -0.1 0"); // Make the vehicle copy the terrain properly
		SCR_VehicleSpawnProtectionComponent protectionComponent = SCR_VehicleSpawnProtectionComponent.Cast(veh.FindComponent(SCR_VehicleSpawnProtectionComponent));
	
		if (protectionComponent) {
			protectionComponent.SetVehicleOwner(playerID);
			protectionComponent.SetProtectionTime(120.0);
		}
		
		m_fLastAssetRequestTimestamp = Replication.Time();
		Print(FormatTime(m_fLastAssetRequestTimestamp));
		RpcDo_PlayerFeedbackImpl(EKOTHClientNotificationID.VEHICLE_SPAWNED, 0, assetID);
		Replication.BumpMe();
		
		Print(ToString() + "::RpcAsk_SpawnVehicle - End");
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_OnOpenKOTHMenu()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.KOTHSuperMenu, 0, true, true);
	}

	//------------------------------------------------------------------------------------------------
	// Init
	override void EOnInit(IEntity owner)
	{
		m_PlayerController = SCR_PlayerController.Cast(PlayerController.Cast(owner));
		
		if (!m_PlayerController) {
			Print("KOTH_NetworkComponent must be attached to PlayerController!", LogLevel.ERROR);
			return;
		}
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_RplComponent) {
			Print("Could not find RplComponent on PlayerController!", LogLevel.ERROR);
			return;
		}
		
		m_KOTHGameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		if (!m_KOTHGameMode) {
			Print("Could not find KOTH_GameModeBase!", LogLevel.ERROR);
			return;
		}
		
		m_fLastAssetRequestTimestamp = Replication.Time();
	}

	//------------------------------------------------------------------------------------------------
	// PostInit
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
		owner.SetFlags(EntityFlags.ACTIVE, true);
	}

	//------------------------------------------------------------------------------------------------
	// Constructor
	void KOTH_NetworkComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		
	}

	//------------------------------------------------------------------------------------------------
	// Destructor
	void ~KOTH_NetworkComponent()
	{

	}
};
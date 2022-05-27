//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/KOTH", description: "Handles client > server communication in KOTH. Should be attached to PlayerController.", color: "0 0 255 255")]
class KOTH_NetworkComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! Used to identify various notifications for client
enum EKOTHClientNotificationID
{
	VEHICLE_SPAWNED
};

class KOTH_NetworkComponent : ScriptComponent
{
	// Member variables
	protected SCR_PlayerController m_PlayerController;
	protected RplComponent m_RplComponent;
	protected KOTH_GameModeBase m_KOTHGameMode;
	
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
		
		switch (msgID)
		{
			case EKOTHClientNotificationID.VEHICLE_SPAWNED:
			{
				msg = "#AR-Campaign_VehicleReady-UC";
				msg2 = m_KOTHGameMode.GetVehicleAssetDisplayName(assetID);
								
				SCR_UISoundEntity.SoundEvent("SOUND_LOADSUPPLIES");
				break;
			};
			default: {return;};
		}
		
		SCR_PopUpNotification.GetInstance().PopupMsg(msg, duration, 0.5, msg2, param1: msg1param1, text2param1: msg2param1, text2param2: msg2param2);
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

		Replication.BumpMe();
		
		RpcDo_PlayerFeedbackImpl(EKOTHClientNotificationID.VEHICLE_SPAWNED, 0, assetID);
		
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

		if (!m_PlayerController)
		{
			Print("KOTH_NetworkComponent must be attached to PlayerController!", LogLevel.ERROR);
			return;
		}
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_RplComponent)
		{
			Print("Could not find RplComponent on PlayerController!", LogLevel.ERROR);
			return;
		}
		
		m_KOTHGameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		if (!m_KOTHGameMode)
		{
			Print("Could not find KOTH_GameModeBase!", LogLevel.ERROR);
			return;
		}
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
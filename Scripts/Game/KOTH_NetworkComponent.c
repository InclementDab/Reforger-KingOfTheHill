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
	//! Checks if the session is run as client
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//***********//
	//RPC METHODS//
	//***********//

	//------------------------------------------------------------------------------------------------
	// Sends player notification to players inside Vehicle
	// parameters: entity of vehicle, int ID of player to notify about, optional second parameter for another int number
	/*protected void SendToVehicleOccupants(ENotification messageID, IEntity vehicleEntity, int playerID, int number = 0)
	{
		array<IEntity> occupants = {};
		IEntity parentVehicle = vehicleEntity.GetParent();

		//Gettings players from inside of vehicle. Condition allows spawning only Cargo.
		SCR_BaseCompartmentManagerComponent comp = SCR_BaseCompartmentManagerComponent.Cast(parentVehicle.FindComponent(SCR_BaseCompartmentManagerComponent));

		if(!parentVehicle)
			return;

		comp = SCR_BaseCompartmentManagerComponent.Cast(parentVehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		comp.GetOccupants(occupants);

		if(occupants.IsEmpty())
			return;

		foreach(IEntity occupant : occupants)
		{
			int occupantID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(occupant);
			if(number != 0)
				SCR_NotificationsComponent.SendToPlayer(occupantID, messageID, playerID, number);
			else
				SCR_NotificationsComponent.SendToPlayer(occupantID, messageID, playerID);
		}
	}*/

	//------------------------------------------------------------------------------------------------
	/*void SendPlayerMessage(SCR_ERadioMsg msgType, int baseCallsign = SCR_CampaignBase.INVALID_BASE_INDEX, int calledID = SCR_CampaignBase.INVALID_PLAYER_INDEX, bool public = true, int param = SCR_CampaignRadioMsg.INVALID_RADIO_MSG_PARAM, bool checkHQReached = false)
	{
		if (!m_PlayerController)
			return;

		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();

		if (!campaign)
			return;

		SCR_CallsignManagerComponent callsignManager = SCR_CallsignManagerComponent.Cast(campaign.FindComponent(SCR_CallsignManagerComponent));

		if (!callsignManager)
			return;

		IEntity player = m_PlayerController.GetMainEntity();

		if (!player)
			return;

		int companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller, characterCallsignIndexCaller;

		if (!callsignManager.GetEntityCallsignIndexes(player, companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller, characterCallsignIndexCaller))
    		return;

		SCR_GadgetManagerComponent gadgetMan = SCR_GadgetManagerComponent.Cast(player.FindComponent(SCR_GadgetManagerComponent));

		if (!gadgetMan)
			return;

		IEntity radioEnt = gadgetMan.GetGadgetByType(EGadgetType.RADIO);

		if (!radioEnt)
			return;

		BaseRadioComponent radio = BaseRadioComponent.Cast(radioEnt.FindComponent(BaseRadioComponent));

		if (!radio)
			return;

		IEntity called = GetGame().GetPlayerManager().GetPlayerControlledEntity(calledID);

		SCR_CampaignRadioMsg msg = new SCR_CampaignRadioMsg;
		msg.SetRadioMsg(msgType);
		msg.SetBaseCallsign(baseCallsign);
		msg.SetCallerCallsign(companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller);
		msg.SetIsPublic(public);
		msg.SetParam(param);
		msg.SetPlayerID(m_PlayerController.GetPlayerId());

		int companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled, characterCallsignIndexCalled;

		if (called && callsignManager.GetEntityCallsignIndexes(called, companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled, characterCallsignIndexCalled))
			msg.SetCalledCallsign(companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled);

		Rpc(RpcDo_PlayRadioMsg, msgType, baseCallsign, CompressCallsign(companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller), CompressCallsign(companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled), param, msg.GetSeed(), 1, checkHQReached);

		radio.Transmit(msg);
	}*/

	//------------------------------------------------------------------------------------------------
	//! Send server request to spawn a vehicle at given depot
	//! \param deliveryPointID Delivery point entity ID
	//! \param assetID Unique asset ID (its index in asset list)
	//! \param playerID Requester entity ID
	/*void SpawnVehicle(IEntity deliveryPoint, int assetID)
	{
		RplId deliveryPointID = Replication.FindId(deliveryPoint);

		if (!m_PlayerController)
			return;

		int playerID = m_PlayerController.GetPlayerId();
		Rpc(RpcAsk_SpawnVehicle, deliveryPointID, assetID, playerID);
	}*/

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

		if (!player)
		{
			Print(ToString() + "::RpcAsk_SpawnVehicle - F 2");
			return;
		}
		
		array<ref KOTH_VehicleAssetInfo> vehicles = new array<ref KOTH_VehicleAssetInfo>;
		m_KOTHGameMode.GetVehicleAssetList(vehicles);
		Resource res = Resource.Load(vehicles[assetID].GetPrefab());
		if (!res)
		{
			Print(ToString() + "::RpcAsk_SpawnVehicle - F 3");
			return;
		}
				
		// Spawn the vehicle
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		spawnPoint.GetWorldTransform(params.Transform);
		Vehicle veh = Vehicle.Cast(GetGame().SpawnEntityPrefab(res, null, params));

		if (!veh)
		{
			Print(ToString() + "::RpcAsk_SpawnVehicle - F 4");
			return;
		}
		
		Physics physicsComponent = veh.GetPhysics();
		if (!physicsComponent)
			return;

		veh.GetPhysics().SetVelocity("0 -0.1 0"); // Make the vehicle copy the terrain properly
		//BaseRadioComponent radioComponent = BaseRadioComponent.Cast(veh.FindComponent(BaseRadioComponent));
		SCR_VehicleSpawnProtectionComponent protectionComponent = SCR_VehicleSpawnProtectionComponent.Cast(veh.FindComponent(SCR_VehicleSpawnProtectionComponent));
		//SCR_ECampaignHints hintID = SCR_ECampaignHints.NONE;

		if (protectionComponent)
		{
			protectionComponent.SetVehicleOwner(playerID);
			protectionComponent.SetProtectionTime(120.0);
		}

		// Vehicle spawned, inform requester
		//Rpc(RpcDo_PlayerFeedbackAsset, ECampaignClientNotificationID.VEHICLE_SPAWNED, assetID, hintID);
		Replication.BumpMe();
		
		Print(ToString() + "::RpcAsk_SpawnVehicle - End");
	}

	//------------------------------------------------------------------------------------------------
	//! Bump supply truck lifetime in garbage manager if it's parked near a base
	/*protected void OnSupplyTruckLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		SCR_CampaignGarbageManager gManager = SCR_CampaignGarbageManager.Cast(GetGame().GetGarbageManager());

		if (!gManager)
			return;

		if (!gManager.IsInserted(vehicle))
			return;

		array<SCR_CampaignBase> bases = SCR_CampaignBaseManager.GetBases();
		int baseDistanceSq = Math.Pow(SCR_CampaignGarbageManager.MAX_BASE_DISTANCE, 2);
		vector vehPos = vehicle.GetOrigin();
		bool baseNearby;

		foreach (SCR_CampaignBase base: bases)
		{
			if (!base)
				continue;

			baseNearby = vector.DistanceSqXZ(vehPos, base.GetOrigin()) <= baseDistanceSq;

			if (baseNearby)
				break;
		}

		if (!baseNearby)
			return;

		float curLifetime = gManager.GetLifetime(vehicle);

		if (curLifetime >= SCR_CampaignGarbageManager.PARKED_SUPPLY_TRUCK_LIFETIME)
			return;

		gManager.Bump(vehicle, SCR_CampaignGarbageManager.PARKED_SUPPLY_TRUCK_LIFETIME - curLifetime);
	}*/

	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	//! \param msgID Message ID (see ECampaignClientNotificationID)
	/*[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_PlayerFeedback(int msgID)
	{
		RpcDo_PlayerFeedbackImpl(msgID);
	}*/

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
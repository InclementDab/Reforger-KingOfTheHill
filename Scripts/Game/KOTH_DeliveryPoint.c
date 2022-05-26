[EntityEditorProps(category: "GameScripted/KOTH", description: "KOTH delivery point allowing asset acquisition", color: "0 0 255 255")]
class KOTH_DeliveryPointClass: GenericEntityClass
{
};


//------------------------------------------------------------------------------------------------
//! Serves as template for all spots where players can request assets (vehicle depots etc.) in Campaign
class KOTH_DeliveryPoint : GenericEntity
{
	//static const string VARNAME_KOTH_POINT_ID = "m_iDeliveryPointID";
	
	[Attribute("-1")]
	protected int m_iDeliveryPointID;
	
	// Member variables
	protected KOTH_GameModeBase m_GameMode;
	protected RplComponent m_RplComponent;

	//------------------------------------------------------------------------------------------------
	//! Checks if the session is run as client
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_GameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
		ClearFlags(EntityFlags.ACTIVE, true);
	}

	//------------------------------------------------------------------------------------------------
	// Constructor
	void KOTH_DeliveryPoint(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.ACTIVE, true);
	}

	//------------------------------------------------------------------------------------------------
	void ~KOTH_DeliveryPoint()
	{

	}
	
	int GetDeliveryPointID()
	{
		return m_iDeliveryPointID;
	}
};
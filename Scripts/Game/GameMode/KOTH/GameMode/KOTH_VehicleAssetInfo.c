//! Config template for vehicles available for request in Campaign
[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Prefab", true)]
class KOTH_VehicleAssetInfo
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "", "et")]
	protected ResourceName m_Prefab;
	[Attribute("Vehicle", desc: "Display name to be shown in UI.")]
	protected string m_sDisplayName;
	[Attribute("VEHICLE", desc: "Display name to be shown in UI (upper case).")]
	protected string m_sDisplayNameUC;
	[Attribute("", desc: "Description to be shown in UI.")]
	protected string m_sDescription;
	[Attribute("", desc: "Vehicle armor to be shown in UI.")]
	protected int m_sArmor;
	[Attribute("", desc: "Vehicle max. speed to be shown in UI.")]
	protected int m_sSpeed;
	[Attribute("", desc: "Vehicle required rank.")]
	protected string m_sRank;
	[Attribute("", desc: "Vehicle price.")]
	protected int m_sPrice;

	ResourceName GetPrefab()
	{
		return m_Prefab;
	}

	string GetDisplayName()
	{
		return m_sDisplayName;
	}

	string GetDisplayNameUpperCase()
	{
		return m_sDisplayNameUC;
	}
	
	string GetDescription()
	{
		return m_sDescription;
	}
	
	int GetArmor()
	{
		return m_sArmor;
	}
	
	int GetMaxSpeed()
	{
		return m_sSpeed;
	}
	
	string GetRequiredRank()
	{
		return m_sRank;
	}
	
	int GetPrice()
	{
		return m_sPrice;
	}
};

[BaseContainerProps(configRoot: true)]
class KOTH_VehicleAssetList
{
	[Attribute(desc: "Vehicle asset list.")]
	private ref array<ref KOTH_VehicleAssetInfo> m_VehicleAssetList;


	void GetVehicleAssetList(out notnull array<ref KOTH_VehicleAssetInfo> vehicleAssetList)
	{
		vehicleAssetList = m_VehicleAssetList;
	}

	void ~KOTH_VehicleAssetList()
	{
		m_VehicleAssetList = null;
	}
};
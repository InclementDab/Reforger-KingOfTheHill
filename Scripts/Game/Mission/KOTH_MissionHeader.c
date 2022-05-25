class KOTH_MissionHeader: SCR_MissionHeader
{
	[Attribute("10", UIWidgets.EditBox, "The threshold for VIP / admin slots")]
	protected int m_VIPSlotCount;
	
	int GetVIPSlotCount()
	{
		return m_VIPSlotCount;
	}
}
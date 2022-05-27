class KOTH_PlayerStorageComponentClass: ScriptComponentClass
{
	
}

class KOTH_PlayerStorageComponent: ScriptComponent
{
	protected string m_PlayerUid;
	
	[RplProp()]
	protected int m_Currency;
	
	[RplProp()]
	protected int m_Experience;
	
	void SetCurrency(int currency)
	{
		if (!Replication.IsServer()) {
			return;
		}
		
		m_Currency = currency;
	}
	
	int GetCurrency()
	{
		return m_Currency;
	}
		
}

modded class SCR_ChimeraCharacter
{
	// Get bohemia id associated with this character
	string GetUid()
	{
		if (RplSession.Mode() != RplMode.Dedicated) {
			return "1234567";
		}
		
		return GetGame().GetBackendApi().GetPlayerUID(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(this));
	}
}
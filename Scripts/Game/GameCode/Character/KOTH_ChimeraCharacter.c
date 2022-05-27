class KOTH_PlayerStorageComponentClass: ScriptComponentClass
{
	
}

class KOTH_PlayerStorageComponent: ScriptComponent
{
	protected string m_PlayerUid;
	
	[RplProp()]
	protected int m_Currency;
	
	
	protected int m_Experience;
	

		
}

modded class SCR_ChimeraCharacter
{
	[RplProp()] // we are replicating this to our client so they know whats goin on
	protected KOTH_PlayerStorage m_PlayerStorage;
	
	// assigned on spawn
	void SetPlayerStorage(KOTH_PlayerStorage player_storage) 
	{
		m_PlayerStorage = player_storage;
	}
	
	void SetCurrency(int currency)
	{
		if (!Replication.IsServer() || !m_PlayerStorage) {
			return;
		}
		
		m_PlayerStorage.Currency = currency;
		Replication.BumpMe();
	}
	
	int GetCurrency()
	{
		if (!m_PlayerStorage) {
			Print("PlayerStorage was invalid!", LogLevel.ERROR);
			return -1;
		}
		
		return m_PlayerStorage.Currency;
	}
	
	void SetExperience(int experience)
	{
		if (!Replication.IsServer() || !m_PlayerStorage) {
			return;
		}
		
		m_PlayerStorage.Experience = experience;
		Replication.BumpMe();
	}
	
	int GetExperience()
	{
		if (!m_PlayerStorage) {
			Print("PlayerStorage was invalid!", LogLevel.ERROR);
			return -1;
		}
		
		return m_PlayerStorage.Experience;
	}
	
	// just adds, todo
	void AddClass(KOTH_PlayerClasses classes)
	{
		if (!Replication.IsServer() || !m_PlayerStorage) {
			return;
		}
		
		
		Replication.BumpMe();
	}
	
	// overrwrite
	void SetClass(KOTH_PlayerClasses classes)
	{
		if (!Replication.IsServer() || !m_PlayerStorage) {
			return;
		}
		
		m_PlayerStorage.Classes = classes;
		Replication.BumpMe();
	}
	
	KOTH_PlayerClasses GetClasses()
	{
		if (!m_PlayerStorage) {
			Print("PlayerStorage was invalid!", LogLevel.ERROR);
			return -1;
		}
		
		return m_PlayerStorage.Classes;
	}
	
	// Get bohemia id associated with this character
	string GetUid()
	{
		if (RplSession.Mode() != RplMode.Dedicated) {
			return "1234567";
		}
		
		return GetGame().GetBackendApi().GetPlayerUID(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(this));
	}
}
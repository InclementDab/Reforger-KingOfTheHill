modded class SCR_ChimeraCharacter
{
	[RplProp()] // we are replicating this to our client so they know whats goin on
	protected ref KOTH_PlayerStorage m_PlayerStorage; // extra ref, todo
	
	[RplProp()]
	protected string m_ClientId;
		
	override void EOnInit(IEntity owner)
	{			
		//GetGame().GetBackendApi().GetCredentialsItem(EBackendCredentials.EBCRED_NAME);
	}
		
	void SetStorage(KOTH_PlayerStorage storage)
	{
		m_PlayerStorage = storage;
		Replication.BumpMe();
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
	
	
	// Get unique id associated with this character
	string GetUid()
	{
		return m_ClientId;
		return "1234567";
		if (RplSession.Mode() != RplMode.Dedicated) {
			return "1234567";
		}		
		
		return GetGame().GetBackendApi().GetPlayerUID(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(this));
	}
}
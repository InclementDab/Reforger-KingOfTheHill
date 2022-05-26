typedef func OnFactionTicketChanged;
void OnFactionTicketChanged(KOTH_Faction faction, int tickets);

enum KOTHZoneContestType
{
	EMPTY,
	TIE,
	OWNED,
}

class KOTH_ZoneManagerClass: GenericComponentClass
{
}

class KOTH_ZoneManager: GenericComponent
{
	[Attribute("100", desc: "Amount of tickets required by a team to win the game.")]
	protected int m_TicketCountToWin;

	[Attribute("10", desc: "Update rate of tickets, in seconds.")]
	protected float m_TicketUpdateInterval;

	protected KOTH_ZoneTriggerEntity m_Zone;
	protected ref ScriptInvoker<OnFactionTicketChanged> OnFactionTicketChangedScript = new ScriptInvoker<OnFactionTicketChanged>();

	[RplProp()]
	protected KOTHZoneContestType m_KOTHZoneContestType;

	[RplProp()]
	protected int m_Tickets;

	protected ref array<KOTH_Faction> m_ZoneOwners = {};
	protected KOTH_GameModeBase m_GameMode;
	protected SCR_FactionManager m_FactionManager;
	protected ref array<KOTH_SafeZoneTriggerEntity> m_SafeZones = {};

	void KOTH_ZoneManager()
	{
		if (!m_GameMode) {
			m_GameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		}

		m_GameMode.GetOnGameStart().Insert(OnGameStart);
		m_GameMode.GetOnGameEnd().Insert(OnGameEnd);
	}

	void SetZone(KOTH_ZoneTriggerEntity zone)
	{
		m_Zone = zone;
	}
	
	void AddSafeZone(KOTH_SafeZoneTriggerEntity safe_zone)
	{
		m_SafeZones.Insert(safe_zone);
	}
	
	array<KOTH_SafeZoneTriggerEntity> GetSafeZones()
	{
		return m_SafeZones;
	}

	void OnGameStart()
	{
		if (Replication.IsServer()) {
			GetGame().GetCallqueue().CallLater(DoTicketUpdate, m_TicketUpdateInterval * 1000, true);
		}
	}

	void OnGameEnd()
	{
		if (Replication.IsServer()) {
			GetGame().GetCallqueue().Remove(DoTicketUpdate);
		}
	}

	//! Server Only
	void DoTicketUpdate()
	{
		if (!m_Zone) {
			return;
		}

		// todo plz fix
		if (m_GameMode.GetState() != SCR_EGameModeState.GAME) {
			return;
		}

		m_ZoneOwners.Clear();
		int max_fact_count;
		foreach (KOTH_Faction faction, set<SCR_ChimeraCharacter> characters: m_Zone.GetCharactersInZone()) {
			if (characters.Count() == 0) { // dont want a tie with 0's
				continue;
			}

			if (characters.Count() >= max_fact_count) {
				if (characters.Count() > max_fact_count) {
					m_ZoneOwners.Clear();
					max_fact_count = characters.Count();
				}

				m_ZoneOwners.Insert(faction);
			}
		}

		// no ticket updates, no one is in zone
		if (m_ZoneOwners.Count() == 0) {
			m_KOTHZoneContestType = KOTHZoneContestType.EMPTY;
		}

		if (m_ZoneOwners.Count() == 1) {
			KOTH_Faction zone_owner = m_ZoneOwners[0];
			m_KOTHZoneContestType = KOTHZoneContestType.OWNED;
			SetTickets(zone_owner, GetTickets(zone_owner) + 1);
			OnFactionTicketChangedScript.Invoke(zone_owner, GetTickets(zone_owner));
		}

		// contested!
		if (m_ZoneOwners.Count() > 1) {
			m_KOTHZoneContestType = KOTHZoneContestType.TIE;
		}

		// check our ticket counts
		array<KOTH_Faction> fctn = GetCurrentFactions();
		foreach (KOTH_Faction faction: fctn) {
			if (GetTickets(faction) >= m_TicketCountToWin) {
				if (m_GameMode) {
					m_GameMode.EndGameMode(SCR_GameModeEndData.CreateSimple(SCR_GameModeEndData.ENDREASON_SCORELIMIT, winnerFactionId: GetGame().GetFactionManager().GetFactionIndex(faction)));
				}
			}
		}
	}

	void SetTickets(KOTH_Faction faction, int tickets)
	{
		m_Tickets |= tickets << GetFactionId(faction) * 8;
		Replication.BumpMe();
	}

	int GetTickets(KOTH_Faction faction)
	{
		return ((m_Tickets >> GetFactionId(faction) * 8) & 0x000000FF);
	}

	bool IsZoneOwner(KOTH_Faction faction)
	{
		return (m_ZoneOwners.Find(faction) != -1);
	}

	KOTH_Faction GetZoneOwner()
	{
		if (m_KOTHZoneContestType != KOTHZoneContestType.OWNED) {
			return null;
		}

		return m_ZoneOwners[0];
	}

	array<KOTH_Faction> GetZoneOwners()
	{
		return m_ZoneOwners;
	}

	KOTHZoneContestType GetZoneContestType()
	{
		return m_KOTHZoneContestType;
	}

	int GetAmountOfPlayersInZone(KOTH_Faction faction)
	{
		return m_Zone.GetAmountOfPlayersInZone(faction);
	}

	int GetTicketCountToWin()
	{
		return m_TicketCountToWin;
	}

	bool IsInZone(SCR_ChimeraCharacter character)
	{
		return (m_Zone.IsInZone(character));
	}

	// TODO: dynamically generate these based on the mission loaded
	array<KOTH_Faction> GetCurrentFactions()
	{
		return {
			KOTH_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("US")),
			KOTH_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("USSR")),
			KOTH_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("FIA")),
		};
	}

	int GetFactionId(KOTH_Faction faction)
	{
		return GetCurrentFactions().Find(faction);
	}

	static KOTH_ZoneManager GetInstance()
	{
		return KOTH_ZoneManager.Cast(GetGame().GetGameMode().FindComponent(KOTH_ZoneManager));
	}

	KOTH_ZoneTriggerEntity GetZone()
	{
		return m_Zone;
	}
}
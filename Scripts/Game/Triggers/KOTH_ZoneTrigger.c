[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class KOTH_ZoneTriggerEntityClass: ScriptedGameTriggerEntityClass
{
}

class KOTH_ZoneTriggerEntity: ScriptedGameTriggerEntity
{
	protected ref map<Faction, ref set<ChimeraCharacter>> m_CharactersInZone = new map<Faction, ref set<ChimeraCharacter>>();
	
	protected KOTH_GameModeBase m_GameMode;
	protected KOTH_ZoneManager m_ZoneManager;
	protected FactionManager m_FactionManager;
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_FactionManager = GetGame().GetFactionManager();
		m_GameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		m_ZoneManager = m_GameMode.GetKOTHZoneManager();
		if (!m_ZoneManager) {
			Print("Could not find zone manager!", LogLevel.ERROR);
			return;
		}
		
		m_ZoneManager.SetZone(this);
	}
	
	override void OnActivate(IEntity ent)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(ent);
		if (!character) {
			return;
		}
		
		Faction player_faction = GetFactionFromCharacter(character);				
		if (!m_CharactersInZone[player_faction]) {
			m_CharactersInZone[player_faction] = new set<ChimeraCharacter>();
		}
		
		// do we need to remove the player from the zone?
		if (character.GetCharacterController().IsDead()) {
			if (m_CharactersInZone[player_faction].Find(character) != -1) {
				m_CharactersInZone[player_faction].Remove(m_CharactersInZone[player_faction].Find(character));
			}
			
			return;
		}
		
		m_CharactersInZone[player_faction].Insert(character);
	}
	
	override void OnDeactivate(IEntity ent)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(ent);
		if (!character) {
			return;
		}
		
		// reeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
		Faction faction = GetFactionFromCharacter(character);
		m_CharactersInZone[faction].Remove(m_CharactersInZone[faction].Find(character));
	}
	
	static Faction GetFactionFromCharacter(ChimeraCharacter character)
	{
		FactionAffiliationComponent faction_affiliation = FactionAffiliationComponent.Cast(character.FindComponent(FactionAffiliationComponent));
		if (!faction_affiliation) {
			return null;
		}
		
		return faction_affiliation.GetAffiliatedFaction();
	}
	
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		return ent.IsInherited(ChimeraCharacter);
	}
	
	int GetAmountOfPlayersInZone(Faction faction)
	{
		return m_CharactersInZone[faction].Count();
	}
	
	map<Faction, ref set<ChimeraCharacter>> GetCharactersInZone()
	{
		return m_CharactersInZone;
	}
}

class KOTH_GameModeBaseClass: SCR_BaseGameModeClass
{
}

class KOTH_GameModeBase: SCR_BaseGameMode
{
	KOTH_ZoneManager GetKOTHZoneManager()
	{
		return KOTH_ZoneManager.Cast(FindComponent(KOTH_ZoneManager));
	}
}

typedef func OnFactionTicketChanged;
void OnFactionTicketChanged(Faction faction, int tickets);

enum KOTHZoneContestType
{
	OWNED,
	TIE,
	EMPTY
}

class KOTH_ZoneManagerClass: GenericComponentClass
{
}

class KOTH_ZoneManager: GenericComponent
{
	[Attribute("100", desc: "Amount of tickets required by a team to win the game.")]
	protected int m_TicketCountToWin = 100;
	
	[Attribute("10", desc: "Update rate of tickets, in seconds.")]
	protected float m_TicketUpdateInterval = 10.0;
	
	protected KOTH_ZoneTriggerEntity m_Zone;
	protected ref map<Faction, int> m_Tickets = new map<Faction, int>();
	protected ref ScriptInvoker<OnFactionTicketChanged> OnFactionTicketChangedScript = new ScriptInvoker<OnFactionTicketChanged>();
	protected KOTHZoneContestType m_KOTHZoneContestType;
	
	protected KOTH_GameModeBase m_pGameMode;
	protected SCR_KOTHTeamScoreDisplay m_ScoreDisplay;
	
	protected int m_USAPlayerCount;
	protected int m_USSRPlayerCount;
	protected int m_FIAPlayerCount;
	
	void KOTH_ZoneManager()
	{
		if (!m_pGameMode)
			m_pGameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());		
		
		if (!m_ScoreDisplay)	
			m_ScoreDisplay = SCR_KOTHTeamScoreDisplay.Cast(m_pGameMode.FindComponent(SCR_KOTHTeamScoreDisplay));
	}
	
	void SetZone(KOTH_ZoneTriggerEntity zone)
	{
		m_Zone = zone;
		GetGame().GetCallqueue().CallLater(DoTicketUpdate, m_TicketUpdateInterval * 1000, true);
	}
		
	void DoTicketUpdate()
	{
		if (!m_Zone) {
			return;
		}
		
		array<Faction> most_populated_factions = new array<Faction>();
		int max_fact_count;
		foreach (Faction faction, set<ChimeraCharacter> characters: m_Zone.GetCharactersInZone()) {
			if (characters.Count() >= max_fact_count) {
				if (characters.Count() > max_fact_count) {
					most_populated_factions.Clear();
					max_fact_count = characters.Count();
				}
				
				most_populated_factions.Insert(faction);
			}
		}
		
		// no ticket updates, no one is in zone
		if (most_populated_factions.Count() == 0) {
			m_KOTHZoneContestType = KOTHZoneContestType.EMPTY;
			Print("Zone is empty");
		}
		
		if (most_populated_factions.Count() == 1) {
			m_KOTHZoneContestType = KOTHZoneContestType.OWNED;
			Faction owned_faction = most_populated_factions[0];
			Print("Zone is owned by " + owned_faction.GetFactionName());
			m_Tickets[owned_faction] = m_Tickets[owned_faction] + 1;
			OnFactionTicketChangedScript.Invoke(owned_faction, m_Tickets[owned_faction]);
		}
		
		// contested!
		if (most_populated_factions.Count() > 1) {
			m_KOTHZoneContestType = KOTHZoneContestType.TIE;
			Print("Zone is contested by..");
			most_populated_factions.Debug();
		}
		
		// check our ticket counts
		foreach (Faction faction, int ticket_count: m_Tickets) {
			if (ticket_count >= m_TicketCountToWin) {
				KOTH_GameModeBase koth_game_mode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
				if (koth_game_mode) {
					koth_game_mode.EndGameMode(SCR_GameModeEndData.CreateSimple(SCR_GameModeEndData.ENDREASON_SCORELIMIT, winnerFactionId: GetGame().GetFactionManager().GetFactionIndex(faction)));
				}
			}
		}
	}
	
	KOTHZoneContestType GetZoneContestType()
	{
		return m_KOTHZoneContestType;
	}
	
	int GetTicketsForFaction(Faction faction)
	{
		return m_Tickets[faction];
	}
	
	int GetTicketCountToWin()
	{
		return m_TicketCountToWin;
	}
	
	int GetUSAPlayerCount()
	{
		return m_USAPlayerCount;
	}
	
	int GetUSSRPlayerCount()
	{
		return m_USSRPlayerCount;
	}
	
	int GetFIAPlayerCount()
	{
		return m_FIAPlayerCount;
	}
}
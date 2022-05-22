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
		if (!m_CharactersInZone[faction]) {
			return 0;
		}
		
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
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);
		
		// start game
		if (GetState() == SCR_EGameModeState.PREGAME) {
			StartGameMode();
		}
	}
	
	KOTH_ZoneManager GetKOTHZoneManager()
	{
		return KOTH_ZoneManager.Cast(FindComponent(KOTH_ZoneManager));
	}
}

typedef func OnFactionTicketChanged;
void OnFactionTicketChanged(Faction faction, int tickets);

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
	protected ref map<Faction, int> m_Tickets = new map<Faction, int>();
	protected ref ScriptInvoker<OnFactionTicketChanged> OnFactionTicketChangedScript = new ScriptInvoker<OnFactionTicketChanged>();
	protected KOTHZoneContestType m_KOTHZoneContestType;
	protected ref array<Faction> m_ZoneOwners = {};
	
	protected KOTH_GameModeBase m_GameMode;
	protected SCR_KOTHTeamScoreDisplay m_ScoreDisplay;
	
	private static KOTH_ZoneManager s_Instance;
			
	void KOTH_ZoneManager()
	{
		if (!m_GameMode) {
			m_GameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());		
		}
		
		if (!m_ScoreDisplay) {
			m_ScoreDisplay = SCR_KOTHTeamScoreDisplay.Cast(m_GameMode.FindComponent(SCR_KOTHTeamScoreDisplay));
		}
		
		m_GameMode.GetOnGameStart().Insert(OnGameStart);
		m_GameMode.GetOnGameEnd().Insert(OnGameEnd);
	}
	
	void SetZone(KOTH_ZoneTriggerEntity zone)
	{
		m_Zone = zone;
	}
	
	void OnGameStart()
	{
		GetGame().GetCallqueue().CallLater(DoTicketUpdate, m_TicketUpdateInterval * 1000, true);
	}
	
	void OnGameEnd()
	{
		GetGame().GetCallqueue().Remove(DoTicketUpdate);
	}
			
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
		foreach (Faction faction, set<ChimeraCharacter> characters: m_Zone.GetCharactersInZone()) {
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
			Faction zone_owner = m_ZoneOwners[0];
			m_KOTHZoneContestType = KOTHZoneContestType.OWNED;
			m_Tickets[zone_owner] = m_Tickets[zone_owner] + 1;
			OnFactionTicketChangedScript.Invoke(zone_owner, m_Tickets[zone_owner]);
		}
		
		// contested!
		if (m_ZoneOwners.Count() > 1) {
			m_KOTHZoneContestType = KOTHZoneContestType.TIE;
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
	
	Faction GetZoneOwner()
	{
		if (m_KOTHZoneContestType != KOTHZoneContestType.OWNED) {
			return null;
		}
		
		return m_ZoneOwners[0];
	}
	
	array<Faction> GetZoneOwners()
	{
		return m_ZoneOwners;
	}
	
	KOTHZoneContestType GetZoneContestType()
	{
		return m_KOTHZoneContestType;
	}
	
	int GetTicketsForFaction(Faction faction)
	{
		return m_Tickets[faction];
	}
	
	int GetAmountOfPlayersInZone(Faction faction)
	{
		return m_Zone.GetAmountOfPlayersInZone(faction);
	}
	
	int GetTicketCountToWin()
	{
		return m_TicketCountToWin;
	}
	
	// TODO: dynamically generate these based on the mission loaded
	array<Faction> GetCurrentFactions()
	{
		return {
			GetGame().GetFactionManager().GetFactionByKey("US"),
			GetGame().GetFactionManager().GetFactionByKey("USSR"),
			GetGame().GetFactionManager().GetFactionByKey("FIA"),
		};
	}
	
	static KOTH_ZoneManager GetKOTHZoneManager()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;

		if (!s_Instance)
			s_Instance = KOTH_ZoneManager.Cast(gameMode.FindComponent(KOTH_ZoneManager));

		return s_Instance;
	}
	
	void RegisterArea(SCR_KOTHArea area)
	{
		
	}
	
	void UnregisterArea(SCR_KOTHArea area)
	{
		
	}
}

class KOTH_VehicleSpawnerClass: ScriptedGameTriggerEntityClass
{
}

class KOTH_VehicleSpawner: ScriptedGameTriggerEntity
{
	[Attribute(desc: "What vehicle to attempt to spawn.")]
	protected string m_VehicleToSpawn;
	
	[Attribute("30", desc: "Check for new spawn time, in seconds.")]
	protected float m_TimeRespawnCheck;
	
	void KOTH_VehicleSpawner(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
	
	override void OnActivate(IEntity ent)
	{
		super.OnActivate(ent);
		
	}
}

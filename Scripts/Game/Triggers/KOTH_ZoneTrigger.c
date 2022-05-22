[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class KOTH_ZoneTriggerEntityClass: ScriptedGameTriggerEntityClass
{
}

class KOTH_ZoneTriggerEntity: ScriptedGameTriggerEntity
{
	protected ref map<KOTH_Faction, ref set<ChimeraCharacter>> m_CharactersInZone = new map<KOTH_Faction, ref set<ChimeraCharacter>>();

	protected SCR_BaseGameMode m_GameMode;
	protected KOTH_GameModeBase m_KOTHGameMode;
	protected KOTH_ZoneManager m_ZoneManager;
	protected FactionManager m_FactionManager;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_FactionManager = GetGame().GetFactionManager();
		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!m_GameMode) {
			Print("Could not find game mode!", LogLevel.ERROR);
			return;
		}
		m_KOTHGameMode = KOTH_GameModeBase.Cast(m_GameMode.FindComponent(KOTH_GameModeBase));
		m_ZoneManager = m_KOTHGameMode.GetKOTHZoneManager();
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

		KOTH_Faction player_faction = GetFactionFromCharacter(character);
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
		KOTH_Faction faction = GetFactionFromCharacter(character);
		m_CharactersInZone[faction].Remove(m_CharactersInZone[faction].Find(character));
	}

	static KOTH_Faction GetFactionFromCharacter(ChimeraCharacter character)
	{
		FactionAffiliationComponent faction_affiliation = FactionAffiliationComponent.Cast(character.FindComponent(FactionAffiliationComponent));
		if (!faction_affiliation) {
			return null;
		}

		return KOTH_Faction.Cast(faction_affiliation.GetAffiliatedFaction());
	}

	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		return ent.IsInherited(ChimeraCharacter);
	}

	int GetAmountOfPlayersInZone(KOTH_Faction faction)
	{
		if (!m_CharactersInZone[faction]) {
			return 0;
		}

		return m_CharactersInZone[faction].Count();
	}

	map<KOTH_Faction, ref set<ChimeraCharacter>> GetCharactersInZone()
	{
		return m_CharactersInZone;
	}
}

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

	protected ref array<KOTH_Faction> m_ZoneOwners = {};
	protected SCR_BaseGameMode m_GameMode;
	protected SCR_KOTHTeamScoreDisplay m_ScoreDisplay;
	protected SCR_FactionManager m_FactionManager;

	void KOTH_ZoneManager()
	{
		if (!m_GameMode) {
			m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
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
		foreach (KOTH_Faction faction, set<ChimeraCharacter> characters: m_Zone.GetCharactersInZone()) {
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
			zone_owner.SetTickets(zone_owner.GetTickets() + 1);
			//m_Tickets[zone_owner] = m_Tickets[zone_owner] + 1;
			OnFactionTicketChangedScript.Invoke(zone_owner, zone_owner.GetTickets());
		}

		// contested!
		if (m_ZoneOwners.Count() > 1) {
			m_KOTHZoneContestType = KOTHZoneContestType.TIE;
		}

		// check our ticket counts
		foreach (KOTH_Faction faction: GetCurrentFactions()) {
			if (faction.GetTickets() >= m_TicketCountToWin) {
				if (m_GameMode) {
					m_GameMode.EndGameMode(SCR_GameModeEndData.CreateSimple(SCR_GameModeEndData.ENDREASON_SCORELIMIT, winnerFactionId: GetGame().GetFactionManager().GetFactionIndex(faction)));
				}
			}
		}
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

	// TODO: dynamically generate these based on the mission loaded
	array<KOTH_Faction> GetCurrentFactions()
	{
		return {
			KOTH_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("US")),
			KOTH_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("USSR")),
			KOTH_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("FIA")),
		};
	}

	static KOTH_ZoneManager GetInstance()
	{
		return KOTH_ZoneManager.Cast(GetGame().GetGameMode().FindComponent(KOTH_ZoneManager));
	}

	void RegisterArea(KOTH_Area area)
	{

	}

	void UnregisterArea(KOTH_Area area)
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

// lol
modded class SCR_FlushToilet
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Drink (Thirsty)";
		return true;
	}
}


class KOTH_Faction: SCR_Faction
{
	[RplProp(onRplName: "OnTicketsChanged")]
	protected int m_Tickets;
	
	int GetTickets()
	{
		return m_Tickets;
	}
	
	void SetTickets(int tickets)
	{
		// only allow server modification
		if (!Replication.IsServer()) {
			return;
		}
		
		m_Tickets = tickets;
	}
	
	void OnTicketsChanged()
	{
		Print("Tickets changed");
		Print(m_Tickets);
	}
}
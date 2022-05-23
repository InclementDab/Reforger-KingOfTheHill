// hack
modded class SCR_DamageManagerComponent// hack
{// hack
	// hack
	protected bool m_GodMode;// hack
	// hack
	void SetGodMode(bool god_mode)// hack
	{// hack
		m_GodMode = god_mode;// hack
	}// hack
	// hack
	override void OnDamageStateChanged(EDamageState state)// hack
	{// hack
		super.OnDamageStateChanged(state);// hack
		// hack
		if (m_GodMode) {// hack
			FullHeal();// hack
		}// hack
	}// hack
}// hack

[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class KOTH_SafeZoneTriggerEntityClass: ScriptedGameTriggerEntityClass
{
}

class KOTH_SafeZoneTriggerEntity: ScriptedGameTriggerEntity
{
	[Attribute(desc: "Faction to apply safezone to.", category: "KOTH")]
	protected string m_FactionKey;
	
	[Attribute("0 0 0", UIWidgets.EditBox, "Center of the safe zone in local space.", category: "KOTH", params: "inf inf 0 purposeCoords spaceEntity")]
	protected vector m_SafeZoneCenter;

	protected SCR_MapDescriptorComponent m_MapDescriptor;
	
	override void OnActivate(IEntity ent)
	{
		super.OnActivate(ent);
		
		if (!Replication.IsServer()) {
			Print(ToString() + "::OnActivate - Tryed to call OnActivate on client!", LogLevel.WARNING);
			return;	
		}
				
		SCR_DamageManagerComponent damage_manager;
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		
		if (!character || character.GetFactionKey() != m_FactionKey) 
		{
			Print(ToString() + "::OnActivate - Detected enemy player in save zone! Process kill!");
			damage_manager = character.GetDamageManager();
			if (damage_manager) {
				damage_manager.SetHealthScaled(0);
			}
			
			return;
		}
		
		damage_manager = character.GetDamageManager();
		if (!damage_manager) {
			Print(ToString() + "::OnActivate - Could not find SCR_DamageManagerComponent!", LogLevel.WARNING);
			return;
		}
		
		damage_manager.SetGodMode(true);
	}
	
	override void OnDeactivate(IEntity ent)
	{
		Print(ToString() + "::OnDeactivate - Start");
		
		super.OnDeactivate(ent);
		if (!Replication.IsServer()) {
			return;	
		}
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character || character.GetFactionKey() != m_FactionKey) {
			return;
		}
		
		SCR_DamageManagerComponent damage_manager = character.GetDamageManager();
		if (!damage_manager) {
			return;
		}
		
		damage_manager.SetGodMode(false);
		
		Print(ToString() + "::OnDeactivate - End");
	}
	
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		return ent.IsInherited(ChimeraCharacter);
	}
	
	vector GetWorldSafeZoneCenter()
	{
		return CoordToParent(m_SafeZoneCenter);
	}
	
	protected override bool RplLoad(ScriptBitReader reader)
	{
		super.RplLoad(reader);

		if (m_MapDescriptor)
			UpdateMapDescriptor(m_MapDescriptor);

		return true;
	}
	
	protected override void OnInit(IEntity owner)
	{
		super.OnInit(owner);

		// Supress messages out of playmode, order of things is not quite guaranteed here
		if (!GetGame().InPlayMode())
			return;
		
		// If map descriptor is present, initialize it
		m_MapDescriptor = SCR_MapDescriptorComponent.Cast(FindComponent(SCR_MapDescriptorComponent));
		if (m_MapDescriptor)
		{
			InitializeMapDescriptor(m_MapDescriptor);
			UpdateMapDescriptor(m_MapDescriptor);
		}
	}

	protected void InitializeMapDescriptor(SCR_MapDescriptorComponent target)
	{
		MapItem item = target.Item();
		if (!item)
			return;

		MapDescriptorProps props = item.GetProps();
		Color color = Color.FromRGBA(44, 62, 80, 255);
		props.SetIconSize(0.65, 0.65, 0.65);
		props.SetTextSize(32, 32, 32);
		props.SetTextBold();
		props.SetTextColor(color);
		props.SetTextOffsetX(-10);
		props.SetTextOffsetY(-16.5);
		props.Activate(true);
		props.SetFont("{EABA4FE9D014CCEF}UI/Fonts/RobotoCondensed/RobotoCondensed_Bold.fnt");
		item.SetProps(props);
		item.SetDisplayName("Safe Zone - " + GetAffiliatedFactionName());
		vector xyz = GetWorldSafeZoneCenter();
		item.SetPos(xyz[0], xyz[2]);
		item.SetVisible(true);
	}

	protected string GetAffiliatedFactionName()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		Faction faction = factionManager.GetFactionByKey(m_FactionKey);
		if (faction)
			return faction.GetFactionName();
		
		return "UNKNOWN";
	}
	
	protected void UpdateMapDescriptor(SCR_MapDescriptorComponent target)
	{
		if (!target)
			return;

		Color color = Color.FromRGBA(44, 62, 80, 255);
		MapDescriptorProps props = target.Item().GetProps();
		props.SetTextColor(color);
		props.Activate(true);
		target.Item().SetProps(props);
	}
}

[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class KOTH_ZoneTriggerEntityClass: ScriptedGameTriggerEntityClass
{
}

class KOTH_ZoneTriggerEntity: ScriptedGameTriggerEntity
{
	[Attribute("0 0 0", UIWidgets.EditBox, "Center of the area tigger in local space.", category: "KOTH", params: "inf inf 0 purposeCoords spaceEntity")]
	protected vector m_ZoneCenter;
	
	protected ref map<KOTH_Faction, ref set<SCR_ChimeraCharacter>> m_CharactersInZone = new map<KOTH_Faction, ref set<SCR_ChimeraCharacter>>();

	protected KOTH_GameModeBase m_GameMode;
	protected KOTH_ZoneManager m_ZoneManager;
	protected FactionManager m_FactionManager;
	protected SCR_MapDescriptorComponent m_MapDescriptor;
	
	void KOTH_ZoneTriggerEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_FactionManager = GetGame().GetFactionManager();
		m_GameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		if (!m_GameMode) {
			Print("Could not find game mode!", LogLevel.ERROR);
			return;
		}
		
		m_ZoneManager = m_GameMode.GetKOTHZoneManager();
		if (!m_ZoneManager) {
			Print("Could not find zone manager!", LogLevel.ERROR);
			return;
		}

		m_ZoneManager.SetZone(this);
	}

	protected override void OnActivate(IEntity ent)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character) {
			return;
		}

		KOTH_Faction player_faction = GetFactionFromCharacter(character);
		if (!m_CharactersInZone[player_faction]) {
			m_CharactersInZone[player_faction] = new set<SCR_ChimeraCharacter>();
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

	protected override void OnDeactivate(IEntity ent)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
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

	map<KOTH_Faction, ref set<SCR_ChimeraCharacter>> GetCharactersInZone()
	{
		return m_CharactersInZone;
	}
	
	bool IsInZone(SCR_ChimeraCharacter character)
	{
		foreach (KOTH_Faction faction, set<SCR_ChimeraCharacter> characters: m_CharactersInZone) {
			if (characters.Find(character) != -1) {
				return true;
			}
		}
		
		return false;
	}
	
	vector GetWorldZoneCenter()
	{
		return CoordToParent(m_ZoneCenter);
	}
	
	protected override bool RplLoad(ScriptBitReader reader)
	{
		super.RplLoad(reader);

		if (m_MapDescriptor)
			UpdateMapDescriptor(m_MapDescriptor);

		return true;
	}
	
	protected override void OnInit(IEntity owner)
	{
		super.OnInit(owner);

		// Supress messages out of playmode, order of things is not quite guaranteed here
		if (!GetGame().InPlayMode())
			return;
		
		// If map descriptor is present, initialize it
		m_MapDescriptor = SCR_MapDescriptorComponent.Cast(FindComponent(SCR_MapDescriptorComponent));
		if (m_MapDescriptor)
		{
			InitializeMapDescriptor(m_MapDescriptor);
			UpdateMapDescriptor(m_MapDescriptor);
		}
	}

	protected void InitializeMapDescriptor(SCR_MapDescriptorComponent target)
	{
		MapItem item = target.Item();
		if (!item)
			return;

		MapDescriptorProps props = item.GetProps();
		Color color = Color.FromRGBA(192, 57, 43, 255);
		props.SetIconSize(0.65, 0.65, 0.65);
		props.SetTextSize(32, 32, 32);
		props.SetTextBold();
		props.SetTextColor(color);
		props.SetTextOffsetX(-10);
		props.SetTextOffsetY(-16.5);
		props.Activate(true);
		props.SetFont("{EABA4FE9D014CCEF}UI/Fonts/RobotoCondensed/RobotoCondensed_Bold.fnt");
		item.SetProps(props);
		item.SetDisplayName("KOTH");
		vector xyz = GetWorldZoneCenter();
		item.SetPos(xyz[0], xyz[2]);
		item.SetVisible(true);
	}
	
	protected void UpdateMapDescriptor(SCR_MapDescriptorComponent target)
	{
		if (!target)
			return;

		Color color = Color.FromRGBA(192, 57, 43, 255);
		MapDescriptorProps props = target.Item().GetProps();
		props.SetTextColor(color);
		props.Activate(true);
		target.Item().SetProps(props);
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
	
	[RplProp()]
	protected int m_Tickets1;
	
	[RplProp()]
	protected int m_Tickets2;
	
	[RplProp()]
	protected int m_Tickets3;

	protected ref array<KOTH_Faction> m_ZoneOwners = {};
	protected KOTH_GameModeBase m_GameMode;
	protected KOTH_TeamScoreDisplay m_ScoreDisplay;
	protected SCR_FactionManager m_FactionManager;

	void KOTH_ZoneManager()
	{
		if (!m_GameMode) {
			m_GameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		}

		if (!m_ScoreDisplay) {
			m_ScoreDisplay = KOTH_TeamScoreDisplay.Cast(m_GameMode.FindComponent(KOTH_TeamScoreDisplay));
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
		//m_Tickets = tickets << GetFactionId(faction) * 8;
		switch (faction.GetFactionKey()) {
			case "US": {
				m_Tickets1 = tickets;
				break;
			}
			
			case "USSR": {
				m_Tickets2 = tickets;
				break;
			}
			
			case "FIA": {
				m_Tickets3 = tickets;
				break;
			}
		}
		
		Replication.BumpMe();
	}
	
	int GetTickets(KOTH_Faction faction)
	{
		switch (faction.GetFactionKey()) {
			case "US": {
				return m_Tickets1;
			}
			
			case "USSR": {
				return m_Tickets2;
			}
			
			case "FIA": {
				return m_Tickets3;
			}
		}
		
		return -1;
		//return (m_Tickets >> 0xFFFFFF00 & (GetFactionId(faction) * 8));  
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
	/*
	int GetTickets()
	{
		return KOTH_ZoneManager.GetInstance().GetTickets(this);
	}
	
	void SetTickets(int tickets)
	{
		KOTH_ZoneManager.GetInstance().SetTickets(this, tickets);
	}*/
}

class KOTH_MapModule: SCR_MapModuleBase
{
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		//m_MapEntity
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	override void OnMapClose(MapConfiguration config)
	{
		super.OnMapClose(config);
	}
}


// set up auto balance
modded class SCR_SelectFactionSubMenu
{
	static const int AUTOBALANCE_THRESHOLD = 3;
	
	override void UpdateFactionPlayerList()
	{
		super.UpdateFactionPlayerList();
		
		m_RespawnSystemComponent = SCR_RespawnSystemComponent.GetInstance();
		
		array<Faction> factions = {};
		GetGame().GetFactionManager().GetFactionsList(factions);
		
		int most_populated_count;
		array<Faction> most_populated_teams = {};
		foreach (Faction faction: factions) {
			if (!m_RespawnSystemComponent) {
				continue;
			}
			
			int faction_count = m_RespawnSystemComponent.GetFactionPlayerCount(faction);
			
			if (faction_count >= most_populated_count + AUTOBALANCE_THRESHOLD) {
				if (faction_count > most_populated_count + AUTOBALANCE_THRESHOLD) {
					most_populated_teams.Clear();
				}
				
				most_populated_count = faction_count;
				most_populated_teams.Insert(faction);
			}
		}
		
		// fallback
		if (most_populated_teams.Count() == KOTH_ZoneManager.GetInstance().GetCurrentFactions().Count()) {
			most_populated_teams.Clear();
		}

		foreach (SCR_FactionMenuTile tile, Faction faction : m_mAvailableFactions) {
			if (tile) {
				tile.SetButtonState(most_populated_teams.Find(faction) == -1);
			}
		}
	}
}

modded class SCR_FactionMenuTile
{
	static override SCR_FactionMenuTile InitializeTile(SCR_DeployMenuTileSelection parent, SCR_Faction faction)
	{
		Widget tile = GetGame().GetWorkspace().CreateWidgets("{5968FE6DF3F3853B}UI/layouts/Menus/RoleSelection/DeployMenuTile.layout");
		SCR_FactionMenuTile handler = SCR_FactionMenuTile.Cast(tile.FindHandler(SCR_FactionMenuTile));
		SCR_GalleryComponent gallery_handler = SCR_GalleryComponent.Cast(parent.GetTileContainer().GetHandler(0));
		if (!handler) {
			return null;
		}

		handler.SetParent(parent);
		handler.SetImage(faction.GetFactionFlag());
		handler.SetText(faction.GetFactionName());
		handler.SetFactionBackgroundColor(faction.GetFactionColor());
		gallery_handler.AddItem(tile);
		return handler;
	}
	
	void SetButtonState(bool state, bool animate = true)
	{
		m_bClickEnabled = state;
		if (!m_bClickEnabled) {
			OnDisabled(animate);
			OnMouseLeave(m_wRoot, null, 0, 0);
		} else {
			OnEnabled(animate);
		}
		
		m_wRoot.SetEnabled(m_bClickEnabled);
	}
	
	override bool OnFocus(Widget w, int x, int y)
	{
		if (!m_bClickEnabled) {
			return false;
		}
		
		return super.OnFocus(w, x, y);
	}
}

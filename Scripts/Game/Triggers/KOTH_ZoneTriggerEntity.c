[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class KOTH_ZoneTriggerEntityClass: SCR_CaptureAreaClass
{
}

//! Callback delegate used by events regarding faction ownership in KOTH_ZoneTriggerEntity.
void KOTHZoneTickEventDelegate(KOTH_ZoneTriggerEntity zone, Faction currentOwner, int score);
typedef func KOTHZoneTickEventDelegate;
typedef ScriptInvokerBase<KOTHZoneTickEventDelegate> KOTHZoneTickEvent;

class KOTH_ZoneTriggerEntity: SCR_CaptureArea
{
	[Attribute("0 0 0", UIWidgets.EditBox, "Center of the area tigger in local space.", category: "KOTH", params: "inf inf 0 purposeCoords spaceEntity")]
	protected vector m_ZoneCenter;
	
	[Attribute("15", UIWidgets.Slider, "Duration in seconds before tickets are awarded.", params: "0.1 100.0 0.1", category: "KOTH")]
	protected float m_fTickRate;
	
	[Attribute("1", UIWidgets.Slider, "Tickets awarded per tick.", params: "1 1000 1", category: "KOTH")]
	protected int m_iScorePerTick;
	
	[Attribute("0", UIWidgets.Slider, "Tickets awarded per tick when contested.", params: "0 1000 1", category: "KOTH")]
	protected int m_iScorePerTickContested;
	
	[Attribute("0.5", UIWidgets.Slider, "Percent of players needed (compared to max players) to start contesting.", params: "0 1 0.01", category: "KOTH")]
	protected float m_fContestingRatio;
	
	//! Elapsed time
	protected float m_fTickTime;

	//! Callback for when a tick event is raised by this area
	protected ref KOTHZoneTickEvent m_pOnTickEvent = new KOTHZoneTickEvent();
	
	//! The faction that currently attacking/contesting this point in relation to the owner faction.
	protected Faction m_pContestingFaction;
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns the zone center position
	*/
	vector GetWorldZoneCenter()
	{
		return CoordToParent(m_ZoneCenter);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns the faction that currently owns the area or null if none.
	*/
	Faction GetContestingFaction()
	{
		return m_pContestingFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns true if this area is being contested.
	*/
	bool IsContested()
	{
		return GetContestingFaction() != null;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Called when Item is initialized from replication stream. Carries the data from Master.
	*/
	protected override bool RplLoad(ScriptBitReader reader)
	{
		super.RplLoad(reader);

		int factionIndex = -1;
		reader.ReadInt(factionIndex);

		Faction contestingFaction;
		if (factionIndex != -1)
			contestingFaction = GetGame().GetFactionManager().GetFactionByIndex(factionIndex);

		m_pContestingFaction = contestingFaction;

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Called when Item is getting replicated from Master to Slave connection. The data will be
		delivered to Slave using RplInit method.
	*/
	protected override bool RplSave(ScriptBitWriter writer)
	{
		super.RplSave(writer);

		int factionIndex = -1;
		if (m_pContestingFaction)
			factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_pContestingFaction);

		writer.WriteInt(factionIndex);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Sets internal owner faction and raises corresponding callback.
	*/
	protected void SetContestingFactionInternal(Faction previousFaction, Faction newFaction)
	{
		m_pContestingFaction = newFaction;
		//OnContestingFactionChanged(previousFaction, newFaction);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Notifies all clients of the owning faction change.
		Index of faction is -1 if null.
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetContestingFaction_BC(int previousFactionIndex, int newFactionIndex)
	{
		FactionManager factionManager = GetGame().GetFactionManager();

		Faction previousFaction;
		if (previousFactionIndex != -1)
			previousFaction = factionManager.GetFactionByIndex(previousFactionIndex);

		Faction newFaction;
		if (newFactionIndex != -1)
			newFaction = factionManager.GetFactionByIndex(newFactionIndex);

		SetContestingFactionInternal(previousFaction, newFaction);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Returns invoker that is invoked when this area ticks (awards score).
	*/
	KOTHZoneTickEvent GetOnTickInvoker()
	{
		return m_pOnTickEvent;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns score awarded per area tick.
	*/
	int GetScorePerTick()
	{
		return m_iScorePerTick;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Utility method - returns current scripted game mode.
	*/
	protected KOTH_GameModeBase GetGameMode()
	{
		return KOTH_GameModeBase.Cast(GetGame().GetGameMode());
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns scoring system attached to current gamemode (if any) or null otherwise.
	*/
	protected SCR_BaseScoringSystemComponent GetScoringSystemComponent()
	{
		return GetGameMode().GetScoringSystemComponent();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Initialize this area and register it to parent manager.
	*/
	protected override void OnInit(IEntity owner)
	{
		super.OnInit(owner);

		// Supress messages out of playmode, order of things is not quite guaranteed here
		if (!GetGame().InPlayMode())
			return;

		// Register self in manager
		KOTH_ZoneManager parentManager = GetGameMode().GetKOTHZoneManager();
		if (!parentManager)
		{
			Print("KOTH cannot find KOTH_ZoneManager! Functionality might be limited!", LogLevel.WARNING);
			return;
		}

		parentManager.SetZone(this);

		if (!GetScoringSystemComponent())
		{
			Print("KOTH cannot find SCR_BaseScoringSystemComponent! Points will not be awarded!", LogLevel.WARNING);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Updates the area and awards points if held.
	*/
	protected override void OnFrame(IEntity owner, float timeSlice)
	{
		super.OnFrame(owner, timeSlice);

		// Only the authority shall awards points
		if (!m_pRplComponent || !m_pRplComponent.IsMaster())
			return;

		// Do not update unless the game is started
		// or if no faction holds the point
		if (!GetGameMode().IsRunning() || !GetOwningFaction())
		{
			m_fTickTime = 0.0;
			return;
		}

		// Update tick time, in case time step is too large,
		// multiple ticks can occur within a single frame
		// TODO: Design question @Jesper:
		// tick even with no owner, or only tick when owning faction holds and reset?
		// additionally this logic might be reworked, so keeping it simple now
		m_fTickTime += timeSlice;
		while (m_fTickTime >= m_fTickRate)
		{
			m_fTickTime -= m_fTickRate;
			OnTick();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Called when point ticks, ie. when tickets should be awarded
		Authority only.
	*/
	protected void OnTick()
	{
		// Callback is fired, if a faction is owning this point
		Faction owningFaction = GetOwningFaction();

		bool isContested = IsContested();

		// Award no score if contesting
		int awardedScore;
		if (isContested)
		 	awardedScore = 0;
		else
			awardedScore = GetScorePerTick();

		m_pOnTickEvent.Invoke(this, owningFaction, awardedScore);

		// No scoring at all
		if (isContested)
			return;

		// Must have scoring system to award points
		SCR_BaseScoringSystemComponent scoringSystem = GetScoringSystemComponent();
		if (!scoringSystem)
			return;

		// Add score to faction
		scoringSystem.AddFactionObjective(owningFaction, awardedScore);
		// And additionally to all players of given faction in this point

		array<SCR_ChimeraCharacter> occupants = {};
		int count = GetOccupants(owningFaction, occupants);
		for (int i = 0; i < count; ++i)
		{
			int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(occupants[i]);
			// Not a player
			if (playerId <= 0)
				continue;

			// Award player score, but do not propagate to faction - faction is already awarded
			scoringSystem.AddObjective(playerId, awardedScore, false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Sets the Faction (or none) that currently contests this area.
		Authority only setter, replicated to clients.
	*/
	protected void DoSetContestingFaction(Faction previousFaction, Faction newFaction)
	{
		// For the authority, this is fired straight away above,
		// so we only send the change to all clients as broadcast
		FactionManager factionManager = GetGame().GetFactionManager();
		int previousIndex = factionManager.GetFactionIndex(previousFaction);
		int newIndex = -1;
		if (newFaction)
			newIndex = factionManager.GetFactionIndex(newFaction);

		SetContestingFactionInternal(previousFaction, newFaction);
		Rpc(Rpc_SetContestingFaction_BC, previousIndex, newIndex);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Handles owning and contesting changes based on defined parameters.
	*/
	protected override Faction EvaluateOwnerFaction()
	{
		Faction owningFaction = super.EvaluateOwnerFaction();
		if (!owningFaction)
		{
			// Clear contesting faction, there is no owner, therefore there can be no contester
			if (m_pContestingFaction)
				DoSetContestingFaction(m_pContestingFaction, null);

			return null;
		}

		// If the area is already owned by a faction (m_pOwnerFaction), do not
		// update ownership if the new evaluated faction (owningFaction) has
		// equal number of players in the area to the previous owner
		if (owningFaction && m_pOwnerFaction)
		{
			if (GetOccupantsCount(owningFaction) <= GetOccupantsCount(m_pOwnerFaction))
				owningFaction = m_pOwnerFaction;
		}

		// If there is an owner faction, see how many occupants of given faction
		int ownerCount = GetOccupantsCount(owningFaction);

		Faction contestingFaction;
		int contestingCount;
		foreach (Faction faction, array<SCR_ChimeraCharacter> occupants : m_mOccupants)
		{
			if (faction == owningFaction)
				continue;

			// Find faction that has second most occupants
			int occupantCount = GetOccupantsCount(faction);
			if (occupantCount > contestingCount)
			{
				contestingCount = occupantCount;
				contestingFaction = faction;
			}
		}

		// Contesting faction needs at least n players to pass the contesting ratio
		float minContestingCount = m_fContestingRatio * ownerCount;
		if ((float)contestingCount >= minContestingCount)
		{
			// We are contesting
			if (m_pContestingFaction != contestingFaction)
				DoSetContestingFaction(m_pContestingFaction, contestingFaction);
		}
		else
		{
			if (m_pContestingFaction != null)
				DoSetContestingFaction(m_pContestingFaction, null);
		}

		return owningFaction;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Returns true if provided character is in this area.
		A character must have an affiliated faction using the FactionAffiliationComponent.
	*/
	bool IsCharacterInside(SCR_ChimeraCharacter character)
	{
		if (!character)
			return false;

		Faction faction = character.GetFaction();
		if (!faction)
			return false;

		int indexOf = m_mOccupants[faction].Find(character);
		return indexOf != -1;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetTickRate()
	{
		return m_fTickRate;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Occurs on change of contesting faction of the area.
	 	\param previousFaction Faction which contested the point prior to this change or null if none.
		\param newFaction Faction that contests the point after this change or null if none.
	*/
	protected event void OnContestingFactionChanged(Faction previousFaction, Faction newFaction)
	{

	}

	//------------------------------------------------------------------------------------------------
	/*!
		Occurs on change of owning faction of the area.
	 	\param previousFaction Faction which held the point prior to this change or null if none.
		\param newFaction Faction that holds the point after this change or null if none.
	*/
	protected override void OnOwningFactionChanged(Faction previousFaction, Faction newFaction)
	{
		super.OnOwningFactionChanged(previousFaction, newFaction);
		
	}
	
	/*[Attribute("0 0 0", UIWidgets.EditBox, "Center of the area tigger in local space.", category: "KOTH", params: "inf inf 0 purposeCoords spaceEntity")]
	protected vector m_ZoneCenter;

	protected ref map<SCR_Faction, ref set<SCR_ChimeraCharacter>> m_CharactersInZone = new map<SCR_Faction, ref set<SCR_ChimeraCharacter>>();

	protected KOTH_GameModeBase m_GameMode;
	protected KOTH_ZoneManager m_ZoneManager;
	protected FactionManager m_FactionManager;
	
	void KOTH_ZoneTriggerEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	protected override void EOnInit(IEntity owner)
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

		SCR_Faction player_faction = GetFactionFromCharacter(character);
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
		SCR_Faction faction = GetFactionFromCharacter(character);
		m_CharactersInZone[faction].Remove(m_CharactersInZone[faction].Find(character));
	}

	static SCR_Faction GetFactionFromCharacter(ChimeraCharacter character)
	{
		FactionAffiliationComponent faction_affiliation = FactionAffiliationComponent.Cast(character.FindComponent(FactionAffiliationComponent));
		if (!faction_affiliation) {
			return null;
		}

		return SCR_Faction.Cast(faction_affiliation.GetAffiliatedFaction());
	}

	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		return ent.IsInherited(ChimeraCharacter);
	}

	int GetAmountOfPlayersInZone(SCR_Faction faction)
	{
		if (!m_CharactersInZone[faction]) {
			return 0;
		}

		return m_CharactersInZone[faction].Count();
	}

	map<SCR_Faction, ref set<SCR_ChimeraCharacter>> GetCharactersInZone()
	{
		return m_CharactersInZone;
	}

	bool IsInZone(SCR_ChimeraCharacter character)
	{
		foreach (SCR_Faction faction, set<SCR_ChimeraCharacter> characters: m_CharactersInZone) {
			if (characters.Find(character) != -1) {
				return true;
			}
		}

		return false;
	}

	vector GetWorldZoneCenter()
	{
		return CoordToParent(m_ZoneCenter);
	}*/
}
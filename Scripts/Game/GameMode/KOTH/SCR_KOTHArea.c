[EntityEditorProps(category: "GameScripted/GameMode/KOTH", description: "Area that awards score points when captured.")]
class SCR_KOTHAreaClass : SCR_CaptureAreaClass
{
}

//! Callback delegate used by events regarding faction ownership in SCR_CaptureArea.
void CaptureAreaTickEventDelegate(SCR_KOTHArea area, Faction currentOwner, int score);
typedef func CaptureAreaTickEventDelegate;
typedef ScriptInvokerBase<CaptureAreaTickEventDelegate> CaptureAreaTickEvent;

//------------------------------------------------------------------------------------------------
/*!
	This area awards score to the faction which controls the area periodically.
	Area registers and unregisters from the KOTH_ZoneManager.
*/
class SCR_KOTHArea : SCR_CaptureArea
{
	[Attribute("0 0 0", UIWidgets.EditBox, "Center of the objective in local space.", category: "KOTH", params: "inf inf 0 purposeCoords spaceEntity")]
	protected vector m_vObjectiveCenter;

	[Attribute("1", UIWidgets.Slider, "Duration in seconds before each point is awarded.", params: "0.1 100.0 0.1", category: "KOTH")]
	protected float m_fTickRate;

	[Attribute("5", UIWidgets.Slider, "Score awarded per tick.", params: "1 1000 1", category: "KOTH")]
	protected int m_iScorePerTick;

	[Attribute("0", UIWidgets.Slider, "Score awarded per tick when contested.", params: "0 1000 1", category: "KOTH")]
	protected int m_iScorePerTickContested;

	[Attribute("0.5", UIWidgets.Slider, "Percent of players needed (compared to max players) to start contesting.", params: "0 1 0.01", category: "KOTH")]
	protected float m_fContestingRatio;

	[Attribute("0", UIWidgets.CheckBox, "Is this area a major capture area?", category: "KOTH")]
	protected bool m_bIsMajor;

	//! Elapsed time
	protected float m_fTickTime;

	//! Callback for when a tick event is raised by this area
	protected ref CaptureAreaTickEvent m_pOnTickEvent = new CaptureAreaTickEvent();

	//! Attached map descriptor component used for visualization (if any) or null if none.
	protected SCR_MapDescriptorComponent m_pMapDescriptor;

	//! The faction that currently attacking/contesting this point in relation to the owner faction.
	protected Faction m_pContestingFaction;

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
		Returns true if this area is marked as a major objective.
	*/
	bool IsMajor()
	{
		return m_bIsMajor;
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

		if (m_pMapDescriptor)
			UpdateMapDescriptor(m_pMapDescriptor);

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
		OnContestingFactionChanged(previousFaction, newFaction);
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
	CaptureAreaTickEvent GetOnTickInvoker()
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
		Returns objective center in local space relative to the area.
	*/
	vector GetLocalObjectiveCenter()
	{
		return m_vObjectiveCenter;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Returns objective center in world space.
	*/
	vector GetWorldObjectiveCenter()
	{
		return CoordToParent(m_vObjectiveCenter);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Utility method - returns current scripted game mode.
	*/
	protected SCR_BaseGameMode GetGameMode()
	{
		return SCR_BaseGameMode.Cast(GetGame().GetGameMode());
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
		KOTH_ZoneManager zone_manager = KOTH_ZoneManager.GetInstance();
		if (!zone_manager) {
			Print(ToString() + "::SCR_KOTHArea - Cannot find KOTH_ZoneManager! Functionality might be limited!", LogLevel.WARNING);
			return;
		}

		zone_manager.RegisterArea(this);

		// If map descriptor is present, initialize it
		m_pMapDescriptor = SCR_MapDescriptorComponent.Cast(FindComponent(SCR_MapDescriptorComponent));
		if (m_pMapDescriptor)
		{
			InitializeMapDescriptor(m_pMapDescriptor);
			UpdateMapDescriptor(m_pMapDescriptor);
		}

		if (!GetScoringSystemComponent())
		{
			Print(ToString() + "::SCR_KOTHArea - Cannot find SCR_BaseScoringSystemComponent! Points will not be awarded!", LogLevel.WARNING);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Initializes objective map descriptor for this area.
		\param target Target map descriptor to initialize as this area
	*/
	protected void InitializeMapDescriptor(SCR_MapDescriptorComponent target)
	{
		MapItem item = target.Item();
		if (!item)
			return;

		MapDescriptorProps props = item.GetProps();
		props.SetIconSize(0.65, 0.65, 0.65);
		props.SetTextSize(32, 32, 32);
		props.SetTextBold();
		props.SetFrontColor(Color.White);
		props.SetTextColor(Color.White);
		props.SetTextOffsetX(-10);
		props.SetTextOffsetY(-16.5);
		props.Activate(true);
		props.SetFont("{EABA4FE9D014CCEF}UI/Fonts/RobotoCondensed/RobotoCondensed_Bold.fnt");
		item.SetProps(props);
		item.SetDisplayName("KOTH"); //! Change me!
		vector xyz = GetWorldObjectiveCenter();
		item.SetPos(xyz[0], xyz[2]);
		item.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Updates objective map descriptor for this area with current state.
		\param target Target map descriptor to initialize as this area
	*/
	protected void UpdateMapDescriptor(SCR_MapDescriptorComponent target)
	{
		if (!target)
			return;

		Color color = Color.FromRGBA(249, 210, 103, 255);
		bool friendly = false;
		if (m_pOwnerFaction)
		{
			if (IsContested())
			{
				float val01 = Math.Sin( GetWorld().GetWorldTime() * 0.01 ) * 0.5 + 0.5;
				color.Lerp(m_pOwnerFaction.GetFactionColor(), val01);
			}
			else
			{
				color = m_pOwnerFaction.GetFactionColor();
			}
		}

		MapDescriptorProps props = target.Item().GetProps();
		props.SetFrontColor(color);
		props.SetTextColor(color);

		props.Activate(true);
		target.Item().SetProps(props);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Updates the area and awards points if held.
	*/
	protected override void OnFrame(IEntity owner, float timeSlice)
	{
		super.OnFrame(owner, timeSlice);

		// Update map descriptor
		if (m_pMapDescriptor && IsContested())
			UpdateMapDescriptor(m_pMapDescriptor);

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
		Called when point ticks, ie. when score should be awarded
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
	/*!
		Occurs on change of contesting faction of the area.
	 	\param previousFaction Faction which contested the point prior to this change or null if none.
		\param newFaction Faction that contests the point after this change or null if none.
	*/
	protected event void OnContestingFactionChanged(Faction previousFaction, Faction newFaction)
	{
		if (m_pMapDescriptor)
			UpdateMapDescriptor(m_pMapDescriptor);
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

		if (m_pMapDescriptor)
			UpdateMapDescriptor(m_pMapDescriptor);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Unregisters self from parent manager.
	*/
	protected void ~SCR_KOTHArea()
	{
		// Far from ideal, OnDelete would be better

		// Register self in manager
		KOTH_ZoneManager zone_manager = KOTH_ZoneManager.GetInstance();
		if (!zone_manager) {
			return;
		}

		zone_manager.UnregisterArea(this);
	}
}
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
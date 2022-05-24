// hack
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

	protected KOTH_GameModeBase m_GameMode;
	protected KOTH_ZoneManager m_ZoneManager;

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

	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		// Supress messages out of playmode, order of things is not quite guaranteed here
		if (!GetGame().InPlayMode())
			return;
		
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

		m_ZoneManager.AddSafeZone(this);
	}

	protected string GetAffiliatedFactionName()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		Faction faction = factionManager.GetFactionByKey(m_FactionKey);
		if (faction)
			return faction.GetFactionName();

		return "UNKNOWN";
	}
	
	KOTH_Faction GetAffiliatedFaction()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		KOTH_Faction faction = KOTH_Faction.Cast(factionManager.GetFactionByKey(m_FactionKey));
		if (faction)
			return faction;

		return NULL;
	}
}
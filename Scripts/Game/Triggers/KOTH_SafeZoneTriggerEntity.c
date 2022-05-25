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
			return;
		}

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character || character.GetFactionKey() != m_FactionKey) {
			return;
		}
		
		SCR_DamageManagerComponent damage_manager = character.GetDamageManager();
		if (!damage_manager) {
			Print(ToString() + "::OnActivate - Could not find SCR_DamageManagerComponent!", LogLevel.WARNING);
			return;
		}

		damage_manager.EnableDamageHandling(false);		
	}

	override void OnDeactivate(IEntity ent)
	{
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

		damage_manager.EnableDamageHandling(true);
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
		if (!GetGame().InPlayMode()) {
			return;
		}
		
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
}
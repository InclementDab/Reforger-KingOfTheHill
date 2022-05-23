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
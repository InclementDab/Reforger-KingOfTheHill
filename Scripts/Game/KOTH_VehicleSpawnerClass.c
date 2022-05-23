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
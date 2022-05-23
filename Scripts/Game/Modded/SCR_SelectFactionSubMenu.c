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
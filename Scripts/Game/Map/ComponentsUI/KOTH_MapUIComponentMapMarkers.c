class KOTH_MapUIComponentMapMarkers : SCR_MapUIBaseComponent
{
	protected ref array<ref KOTH_MapMarker> m_MapMarkers = new array<ref KOTH_MapMarker>;
	protected ref KOTH_MapMarker m_ObjectiveMarker;
	protected ref KOTH_PlayerMapMarker m_PlayerMarker;
	protected ChimeraCharacter m_Player;
	protected bool m_Enabled = true;

	protected KOTH_GameModeBase m_GameMode;
	protected KOTH_ZoneManager m_ZoneManager;

	override void Update()
	{
		super.Update();

		if (!m_Enabled) return;

		if (m_PlayerMarker) m_PlayerMarker.Update();
		if (m_ObjectiveMarker) m_ObjectiveMarker.Update();

		if (m_MapMarkers && m_MapMarkers.Count() > 0)
		{
			foreach (KOTH_MapMarker mapMarker: m_MapMarkers)
			{
				mapMarker.Update();
			}
		}
	}

	override void OnMapClose(MapConfiguration config)
	{
		super.OnMapClose(config);

		/*if (!m_Enabled) return;

		if (m_PlayerMarker)
			delete m_PlayerMarker;

		if (m_ObjectiveMarker)
			delete m_ObjectiveMarker;

		for (int i = 0; i < m_MapMarkers.Count(); i++)
		{
			m_MapMarkers.Remove(i);
		}*/
	}

	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);

		if (!m_Enabled) return;

		if (!m_RootWidget)
			return;

		//! Create player position marker
		if (!m_PlayerMarker)
		{
			m_Player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
			m_PlayerMarker = new KOTH_PlayerMapMarker(m_RootWidget, m_Player);
			m_PlayerMarker.SetColor(Color.FromRGBA(0, 0, 0, 255));
			m_PlayerMarker.SetLabel(m_Player.GetName());
		}

		//! Create objhective marker
		if (!m_ObjectiveMarker)
		{
			m_ObjectiveMarker = new KOTH_MapMarker(m_RootWidget, m_ZoneManager.GetZone().GetWorldZoneCenter());
			m_ObjectiveMarker.SetColor(Color.FromRGBA(142, 68, 173, 255));
			m_ObjectiveMarker.SetLabel("KOTH");
		}

		//! Create safe zone markers
		foreach (KOTH_SafeZoneTriggerEntity safeZone: m_ZoneManager.GetSafeZones())
		{
			KOTH_MapMarker safeZoneMarker = new KOTH_MapMarker(m_RootWidget, safeZone.GetWorldSafeZoneCenter());
			KOTH_Faction faction = safeZone.GetAffiliatedFaction();
			if (faction)
			{
				safeZoneMarker.SetColor(faction.GetFactionColor());
				safeZoneMarker.SetLabel("SAFE ZONE - " + faction.GetFactionName());
			}

			m_MapMarkers.Insert(safeZoneMarker);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();

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

		m_Enabled = m_GameMode.UseMapMarkerComponent();
	}
};

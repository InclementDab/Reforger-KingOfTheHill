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

		if (m_PlayerMarker)
			m_PlayerMarker.Update();
		
		if (m_ObjectiveMarker)
			m_ObjectiveMarker.Update();

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

		if (!m_Enabled) 
			return;

		if (m_PlayerMarker)
			delete m_PlayerMarker;

		if (m_ObjectiveMarker)
			delete m_ObjectiveMarker;

		for (int i = 0; i < m_MapMarkers.Count(); i++)
		{
			m_MapMarkers.Remove(i);
		}
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
			if (!m_Player) 
				return;
			
			m_PlayerMarker = new KOTH_PlayerMapMarker(m_RootWidget, m_Player);
			m_PlayerMarker.SetIcon("{EB294B6B8215EC25}UI/icons/arrow_64x64.edds");
			m_PlayerMarker.SetColor(Color.FromRGBA(44, 62, 80, 255));
			
			int playerID = SCR_PlayerController.GetLocalPlayerId();
			PlayerManager playerManager = GetGame().GetPlayerManager();
			string playerName = playerManager.GetPlayerName(playerID);
			/*array<string> charsToRemove = new array<string>;
			for (int i = 0; i < playerName.Length(); i++)
			{
				string char = playerName.Get(i);
				if (char != "\\")
				{
					charsToRemove.Insert(char);
				}
				else if (char == "\\")
				{
					charsToRemove.Insert(char);
					break;
				}
			}*/
			
			//playerName.Substring(0, toRemove);
			string formatedName = FilterName(playerName);
			m_PlayerMarker.SetLabel(formatedName);
		}
		
		//! Create objective marker
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
	
	string FilterName(string name)
    {
		string partToRemove;
		for (int i = 0; i < name.Length(); i++)
		{
			string char = name.Get(i);
			if (char == "\\" )
			{
				partToRemove = name.Substring(0, i + 1);
				break;
			}
		}
		
		name.Replace(partToRemove, "");
		
        return name;
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

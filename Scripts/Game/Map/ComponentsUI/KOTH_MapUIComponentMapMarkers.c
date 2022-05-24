class KOTH_MapUIComponentMapMarkers : SCR_MapUIBaseComponent
{
	//! Objective marker config attributes
	[Attribute("0.000000 0.616999 0.583993 1.000000", UIWidgets.ColorPicker, desc: "Main color that will be used for the main objective zone marker.")]
	protected ref Color m_iObjectiveMarkerColor;
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Main icon or imageset that will be used for the the main objective zone marker.", params: "edds imageset")]
	protected ResourceName m_rObjectiveMarkerIcon;
	[Attribute("", UIWidgets.EditBox , desc: "Imageset icon name if imageset is used for the the main objective zone marker.")]
	protected string m_rObjectiveMarkerIconName;
	[Attribute("34.0", UIWidgets.EditBox , desc: "Size of the marker icon used for the the main objective zone marker.")]
	protected float m_fObjectiveMarkerIconSize;
	
	//! Player marker config attributes
	[Attribute("0.000000 0.616999 0.583993 1.000000", UIWidgets.ColorPicker, desc: "Main color that will be used for the player marker.")]
	protected ref Color m_iPlayerMarkerColor;
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Main icon or imageset that will be used for the the player marker.", params: "edds imageset")]
	protected ResourceName m_rPlayerMarkerIcon;
	[Attribute("", UIWidgets.EditBox , desc: "Imageset icon name if imageset is used for the the player marker.")]
	protected string m_rPlayerMarkerIconName;
	[Attribute("34.0", UIWidgets.EditBox , desc: "Size of the marker icon used for the the player marker.")]
	protected float m_fPlayerMarkerIconSize;
	
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
			foreach (KOTH_MapMarker mapMarker: m_MapMarkers) {
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

		for (int i = 0; i < m_MapMarkers.Count(); i++) {
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
		if (!m_PlayerMarker) {
			m_Player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
			if (!m_Player) 
				return;
			
			m_PlayerMarker = new KOTH_PlayerMapMarker(m_RootWidget, m_Player);
			if (m_rPlayerMarkerIconName != string.Empty && m_rPlayerMarkerIcon != ResourceName.Empty) {
				m_PlayerMarker.SetIconFromSet(m_rPlayerMarkerIcon, m_rPlayerMarkerIconName);
			}
			else if (m_rPlayerMarkerIconName == string.Empty && m_rPlayerMarkerIcon != ResourceName.Empty) {
				m_PlayerMarker.SetIcon(m_rPlayerMarkerIcon);
			}
			
			m_PlayerMarker.SetColor(m_iPlayerMarkerColor);
			
			int playerID = SCR_PlayerController.GetLocalPlayerId();
			PlayerManager playerManager = GetGame().GetPlayerManager();
			string playerName = playerManager.GetPlayerName(playerID);
			string formatedName = FilterName(playerName);
			m_PlayerMarker.SetLabel(formatedName);
			m_PlayerMarker.SetIconSize(m_fPlayerMarkerIconSize, m_fPlayerMarkerIconSize);
		}
		
		//! Create objective marker
		if (!m_ObjectiveMarker)
		{
			m_ObjectiveMarker = new KOTH_MapMarker(m_RootWidget, m_ZoneManager.GetZone().GetWorldZoneCenter());
			if (m_rObjectiveMarkerIconName != string.Empty && m_rObjectiveMarkerIcon != ResourceName.Empty) {
				m_ObjectiveMarker.SetIconFromSet(m_rObjectiveMarkerIcon, m_rObjectiveMarkerIconName);
			}
			else if (m_rObjectiveMarkerIconName == string.Empty && m_rObjectiveMarkerIcon != ResourceName.Empty) {
				m_ObjectiveMarker.SetIcon(m_rObjectiveMarkerIcon);
			}
			
			m_ObjectiveMarker.SetColor(m_iObjectiveMarkerColor);
			m_ObjectiveMarker.SetLabel("KOTH");
			m_ObjectiveMarker.SetIconSize(36, 36);
		}

		//! Create safe zone markers
		foreach (KOTH_SafeZoneTriggerEntity safeZone: m_ZoneManager.GetSafeZones())
		{
			KOTH_MapMarker safeZoneMarker = new KOTH_MapMarker(m_RootWidget, safeZone.GetWorldSafeZoneCenter());
			KOTH_Faction faction = safeZone.GetAffiliatedFaction();
			if (faction)
			{
				safeZoneMarker.SetIcon("{B1E5566B0FA239A4}UI/icons/marker_64x64.edds");
				safeZoneMarker.SetColor(faction.GetFactionColor());
				safeZoneMarker.SetLabel("SAFE ZONE - " + faction.GetFactionName());
				safeZoneMarker.SetIconSize(m_fObjectiveMarkerIconSize, m_fObjectiveMarkerIconSize);
			}

			m_MapMarkers.Insert(safeZoneMarker);
		}
	}
	
	string FilterName(string name)
    {
		string partToRemove;
		for (int i = 0; i < name.Length(); i++) {
			string char = name.Get(i);
			if (char == "\\" ) {
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
		
		//! Disable the component if not enabled in game mode settings
		if (!m_Enabled)
			SetActive(false);
	}
};

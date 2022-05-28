class KOTH_HUDDisplay : SCR_InfoDisplayExtended
{
	[Attribute(defvalue: "1", desc: "If enabled, players will see a 3D waypoint marker on the objective zone location on there UI.", category: "KOTH: Settings")]
	protected bool m_bEnable3DObjectiveMarker;
	[Attribute(defvalue: "0", desc: "If enabled then the defined marker color will be applied on the 3D marker icon. If not the basic image color will be used.", category: "KOTH: Settings")]
	bool m_bUse3DMarkerColor;
	[Attribute("0.000000 0.616999 0.583993 1.000000", UIWidgets.ColorPicker, desc: "Main color that will be used for the 3D objective marker.", category: "KOTH: Settings")]
	ref Color m_i3DMarkerColor;
	[Attribute("0.000000 0.616999 0.583993 1.000000", UIWidgets.ColorPicker, desc: "Main color that will be used for the 3D objective marker distance text.", category: "KOTH: Settings")]
	ref Color m_i3DMarkerTextColor;

	[Attribute("{C3F217CA9CF060B4}UI/Imagesets/Conflict/ConflictIconsSet.imageset", UIWidgets.ResourceNamePicker, desc: "Main icon or imageset that will be used for the the 3D objective marker.", category: "KOTH: Settings", params: "edds imageset")]
	ResourceName m_r3DMarkerIcon;
	[Attribute("Task_Attack", UIWidgets.EditBox , desc: "Imageset icon name if imageset is used for the the 3D objective marker.", category: "KOTH: Settings")]
	string m_r3DMarkerIconName;
	[Attribute("26.0", UIWidgets.EditBox , desc: "Size of the marker icon used for the the 3D objective marker.", category: "KOTH: Settings")]
	float m_f3DMarkerIconSize;
	
	//! Objective waypoint ui element	
	protected ref KOTH_ObjectiveDisplayObject m_ObjectiveElement;
	//! Array of all wrappers for the individual teams
	protected ref map<SCR_Faction, ref KOTH_TeamScoreDisplayObject> m_ScoringElements = new map<SCR_Faction, ref KOTH_TeamScoreDisplayObject>();

	//! Area manager provides us with necessary API
	protected KOTH_ZoneManager m_KOTHManager;
	
	//! KOTH Game mode instance
	protected KOTH_GameModeBase m_KOTHGameMode;
	
	//! Speed used to fade areas hud when hints are shown
	//protected const float POINTS_LAYOUT_FADE_SPEED = 5.0;

	override bool DisplayStartDrawInit(IEntity owner)
	{
		// No ui can be drawn without necessary items	
		m_KOTHGameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		Print(ToString() + "::DisplayStartDrawInit - m_KOTHGameMode: " + m_KOTHGameMode.ToString());
		if (!m_KOTHGameMode) {
			return false;
		}
		
		m_KOTHManager = m_KOTHGameMode.GetKOTHZoneManager();
		Print(ToString() + "::DisplayStartDrawInit - m_KOTHManager: " + m_KOTHManager.ToString());
		if (!m_KOTHManager) {
			return false;
		}

		return true;
	}

	override void DisplayStartDraw(IEntity owner)
	{
		if (RplSession.Mode() == RplMode.Dedicated) {
			delete this; // lol bugfix
		}
		
		if (!m_wRoot || !m_KOTHManager) {
			return;
		}
		
		//! Create 3D objective waypoint marker
		if (m_bEnable3DObjectiveMarker)
			m_ObjectiveElement = KOTH_ObjectiveDisplayObject(GetGame().GetWorkspace().CreateWidgets("{EEDBCD234A118D9F}UI/layouts/HUD/KOTH/KOTHWaypoint.layout", m_wRoot), this);
		
		//! Create score display
		foreach (SCR_Faction faction: m_KOTHManager.GetCurrentFactions()) {		
			// dynamically load widgets based on teams that are active
			m_ScoringElements[faction] = new KOTH_TeamScoreDisplayObject(GetGame().GetWorkspace().CreateWidgets("{DA5637D17656DCA2}UI/layouts/HUD/KOTH/KOTHScore.layout", m_wRoot.FindAnyWidget("Score_Root")), faction)
		}
	}

	override void DisplayStopDraw(IEntity owner)
	{
		super.DisplayStopDraw(owner);
		m_ScoringElements.Clear();
	}

	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (m_bEnable3DObjectiveMarker)
			m_ObjectiveElement.UpdateObjectiveDisplay();
		
		Widget scoring_root = m_wRoot.FindAnyWidget("Score_Frame");
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		
		// Reposition scoring UI based on whether it is in a map or not
		if (scoring_root) {
			SCR_EditorManagerEntity editor_manager = SCR_EditorManagerEntity.GetInstance();
			if (editor_manager && editor_manager.IsOpened()) {
				FrameSlot.SetPos(scoring_root, 0.0, 72.0);
			} else {
				FrameSlot.SetPos(scoring_root, 0.0, 32.0);
			}
		}

		// set money, probably temporary
		RichTextWidget money_widget = RichTextWidget.Cast(m_wRoot.FindAnyWidget("PlayerMoney"));
		if (money_widget) {
			money_widget.SetText(character.GetCurrency().ToString() + " $");
		}
		
		
		// Fade out points when a hint is shown to prevent clipping
		/*if (m_wAreaLayoutWidget) {
			SCR_PopUpNotification notifications = SCR_PopUpNotification.GetInstance();
			float target_opacity = 1.0;
			if (notifications && notifications.IsShowing()) {
				target_opacity = 0.0;
			}

			if (m_wAreaLayoutWidget.GetOpacity() != target_opacity) {
				m_wAreaLayoutWidget.SetOpacity(Math.Lerp(m_wAreaLayoutWidget.GetOpacity(), target_opacity, timeSlice * POINTS_LAYOUT_FADE_SPEED));
			}
		}*/

		// Update scoring
		foreach (SCR_Faction faction, KOTH_TeamScoreDisplayObject scoring_object: m_ScoringElements) {
			if (!scoring_object || !m_KOTHManager) {
				continue;
			}
				
			scoring_object.UpdateScore(m_KOTHManager.GetTickets(faction));
			scoring_object.UpdatePlayerCount(m_KOTHManager.GetAmountOfPlayersInZone(faction), character && character.GetFaction() == faction && m_KOTHManager.IsInZone(character));
			scoring_object.SetBlinkState(ScoreDiplayObjectBlinkState.OFF);
		}
		
		// update blinking on hud
		foreach (SCR_Faction faction, KOTH_TeamScoreDisplayObject score_display: m_ScoringElements) {
			switch (m_KOTHManager.GetZoneContestType()) {
				case KOTHZoneContestType.OWNED: {
					if (faction == m_KOTHManager.GetZoneOwner()) {
						score_display.SetBlinkState(ScoreDiplayObjectBlinkState.OWNED);
					}
							
					break;
				}

				case KOTHZoneContestType.TIE: {
					if (m_KOTHManager.IsZoneOwner(faction)) {
						score_display.SetBlinkState(ScoreDiplayObjectBlinkState.TIE);
					}
					
					break;
				}
			}
		}
	}
}

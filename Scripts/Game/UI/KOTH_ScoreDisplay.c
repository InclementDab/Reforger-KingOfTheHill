/*!
	This InfoDisplay allows drawing of individual HUD scoring elements for individual KOTH factions.
	In addition it draws all available objectives in the screen space.

	Note:
		This HUD element only supports 3 factions at a single given time.
		In addition only the US, USSR and FIA factions are supported.
		Using more than 3 factions will yield unexpected results.
		
		Similar approach can be taken, but a list of elements or any other
		dynamically filled element would be a much more preferable choice.
*/
class KOTH_TeamScoreDisplay : SCR_InfoDisplayExtended
{
	[Attribute("{A1EA87A89C5215AC}UI\\layouts\\HUD\\KOTH\\KOTHObjectiveLayout.layout", params: "layout")]
	protected ResourceName m_ObjectiveHUDLayout;
	
	[Attribute("{5968FE6DF3F3853B}UI\\layouts\\HUD\\KOTH\\KOTHScore.layout", params: "layout")]
	protected ResourceName m_TeamFlagLayout;

	//! Parent frame that holds all area markers
	protected HorizontalLayoutWidget m_wAreaLayoutWidget;

	//! Array of all wrappers for the individual teams
	protected ref map<KOTH_Faction, ref KOTH_TeamScoreDisplayObject> m_ScoringElements = new map<KOTH_Faction, ref KOTH_TeamScoreDisplayObject>();

	//! Area manager provides us with necessary API
	protected KOTH_ZoneManager m_KOTHManager;
	
	//! KOTH Game mode instance
	protected KOTH_GameModeBase m_KOTHGameMode;

	//! Speed used to fade areas hud when hints are shown
	protected const float POINTS_LAYOUT_FADE_SPEED = 5.0;

	protected KOTHZoneContestType m_CurrentContestType = KOTHZoneContestType.EMPTY;
	protected KOTHZoneContestType m_PreviousContestType = KOTHZoneContestType.EMPTY;
	protected ref KOTH_Faction m_LastZoneOwner;
	
	/*!
		Checks the prerequisites for this InfoDisplay.
	*/
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

	/*!
		Creates individual hud elements.
	*/
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot || !m_KOTHManager) {
			return;
		}
		
		foreach (KOTH_Faction faction: m_KOTHManager.GetCurrentFactions()) {		
			// dynamically load widgets based on teams that are active
			m_ScoringElements[faction] = new KOTH_TeamScoreDisplayObject(GetGame().GetWorkspace().CreateWidgets(m_TeamFlagLayout, m_wRoot.FindAnyWidget("Score_Root")), faction)
		}
	}

	/*!
		Clears all hud elements.
	*/
	override void DisplayStopDraw(IEntity owner)
	{
		super.DisplayStopDraw(owner);
		m_ScoringElements.Clear();
	}

	/*!
		Updates the progress and state of all available scoring elements.
	*/
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		Widget scoring_root = m_wRoot.FindAnyWidget("Score_Frame");
		
		// Reposition scoring UI based on whether it is in a map or not
		if (scoring_root) {
			SCR_EditorManagerEntity editor_manager = SCR_EditorManagerEntity.GetInstance();
			if (editor_manager && editor_manager.IsOpened()) {
				FrameSlot.SetPos(scoring_root, 0.0, 72.0);
			} else {
				FrameSlot.SetPos(scoring_root, 0.0, 32.0);
			}
		}

		// Fade out points when a hint is shown to prevent clipping
		if (m_wAreaLayoutWidget) {
			SCR_PopUpNotification notifications = SCR_PopUpNotification.GetInstance();
			float target_opacity = 1.0;
			if (notifications && notifications.IsShowing()) {
				target_opacity = 0.0;
			}

			if (m_wAreaLayoutWidget.GetOpacity() != target_opacity) {
				m_wAreaLayoutWidget.SetOpacity(Math.Lerp(m_wAreaLayoutWidget.GetOpacity(), target_opacity, timeSlice * POINTS_LAYOUT_FADE_SPEED));
			}
		}

		// Update scoring
		foreach (KOTH_Faction faction, KOTH_TeamScoreDisplayObject scoring_object: m_ScoringElements) {
			if (!scoring_object || !m_KOTHManager) {
				continue;
			}
						
			scoring_object.UpdateScore(faction.GetTickets());
			scoring_object.UpdatePlayerCount(m_KOTHManager.GetAmountOfPlayersInZone(faction));
		}
		
		foreach (KOTH_Faction faction, ref KOTH_TeamScoreDisplayObject element: m_ScoringElements) {
			element.StopBlink();
		}
		
		switch(m_KOTHManager.GetZoneContestType()) {
			//! If zone is owned
			case KOTHZoneContestType.OWNED: {
				//! If the zone was empty before
				if (m_PreviousContestType == KOTHZoneContestType.EMPTY) {
					m_ScoringElements[m_KOTHManager.GetZoneOwner()].DoBlink(false);
				}
				//! If the zone was owned before
				else if (m_PreviousContestType == KOTHZoneContestType.OWNED || m_PreviousContestType == KOTHZoneContestType.TIE) {		
					foreach (KOTH_Faction faction, ref KOTH_TeamScoreDisplayObject element: m_ScoringElements) {
						if (faction == m_KOTHManager.GetZoneOwner()) element.DoBlink(false);
					}
				}
								
				m_LastZoneOwner = m_KOTHManager.GetZoneOwner();				
				m_PreviousContestType = KOTHZoneContestType.OWNED;
				break;
			}
			//! If zone is contested
			case KOTHZoneContestType.TIE: {
				//! If the zone was empty before
				if (m_PreviousContestType == KOTHZoneContestType.EMPTY) {
					foreach (KOTH_Faction faction : m_KOTHManager.GetZoneOwners()) {
						m_ScoringElements[faction].DoBlink(true);
					}
				}
				//! If the zone was owned before
				else if (m_PreviousContestType == KOTHZoneContestType.OWNED || m_PreviousContestType == KOTHZoneContestType.TIE)
				{
					foreach (KOTH_Faction faction : m_KOTHManager.GetZoneOwners()) {
						if (m_LastZoneOwner && faction != m_LastZoneOwner) {
							m_ScoringElements[faction].DoBlink(true);
						}
						else if (m_LastZoneOwner && faction == m_LastZoneOwner) {
							m_ScoringElements[faction].DoBlink(false);
						} else if (!m_LastZoneOwner) {
							m_ScoringElements[faction].DoBlink(true);
						}
					}
				}
				
				m_PreviousContestType = KOTHZoneContestType.TIE;
				m_LastZoneOwner = NULL;
				break;
			}
			case KOTHZoneContestType.EMPTY: {
				if (m_LastZoneOwner) {
					m_ScoringElements[m_LastZoneOwner].StopBlink(); 
				} else {
					foreach (KOTH_Faction faction, ref KOTH_TeamScoreDisplayObject element: m_ScoringElements) {
						element.StopBlink();
					}
				}	
				
				m_PreviousContestType = KOTHZoneContestType.EMPTY;
				m_LastZoneOwner = NULL;
				break;
			}
		}
	}
}

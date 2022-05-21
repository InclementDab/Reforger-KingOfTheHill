//------------------------------------------------------------------------------------------------
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
class SCR_KOTHTeamScoreDisplay : SCR_InfoDisplayExtended
{
	[Attribute("{A1EA87A89C5215AC}UI/layouts/HUD/KOTH/KOTHObjectiveLayout.layout", params: "layout")]
	protected ResourceName m_rObjectiveHUDLayout;

	//! Parent frame that holds all area markers
	protected HorizontalLayoutWidget m_wAreaLayoutWidget;

	//! Array of all wrappers for the individual teams
	protected ref array<ref SCR_KOTHTeamScoreDisplayObject> m_aScoringElements = {};

	//! Area manager provides us with necessary API
	protected KOTH_ZoneManager m_KOTHManager;
	//! Game mode instance
	protected KOTH_GameModeBase m_GameMode;

	//! Speed used to fade areas hud when hints are shown
	protected const float POINTS_LAYOUT_FADE_SPEED = 5.0;

	//------------------------------------------------------------------------------------------------
	/*!
		Checks the prerequisites for this InfoDisplay.
	*/
	override bool DisplayStartDrawInit(IEntity owner)
	{
		// No ui can be drawn without necessary items
		m_GameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		if (!m_GameMode) {
			return false;
		}
	
		m_KOTHManager = m_GameMode.GetKOTHZoneManager();
		if (!m_KOTHManager) {
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Creates individual hud elements.
	*/
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot) {
			return;
		}
		
		if (!m_KOTHManager) {
			return;
		}
		
		foreach (Faction faction: m_KOTHManager.GetCurrentFactions()) {
			if (faction == null) {
				continue;
			}
			
			//m_aScoringElements.Insert(new SCR_KOTHTeamScoreDisplayObject(m_wRoot.FindAnyWidget("Score_" + faction.GetFactionName()), faction));
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Clears all hud elements.
	*/
	override void DisplayStopDraw(IEntity owner)
	{
		DebugPrint("::DisplayStopDraw - Start");
		
		// Clear all scoring elements
		for (int i = m_aScoringElements.Count() - 1; i >= 0; i--)
		{
			m_aScoringElements[i] = null;
			m_aScoringElements.Remove(i);
		}
		
		DebugPrint("::DisplayStopDraw - End");
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Updates the progress and state of all available scoring elements.
	*/
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		//DebugPrint("::DisplayUpdate - Start");
		
		Widget scoring_root = m_wRoot.FindAnyWidget("Score_Root");
		
		// Reposition scoring UI based on whether it is in a map or not
		if (scoring_root) {
			SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
			if (editorManager && editorManager.IsOpened()) {
				FrameSlot.SetPos(scoring_root, 0.0, 72.0);
			} else {
				FrameSlot.SetPos(scoring_root, 0.0, 32.0);
			}
		}

		// Fade out points when a hint is shown to prevent clipping
		if (m_wAreaLayoutWidget) {
			SCR_PopUpNotification notifications = SCR_PopUpNotification.GetInstance();
			float targetOpacity = 1.0;
			if (notifications && notifications.IsShowing())
				targetOpacity = 0.0;

			if (m_wAreaLayoutWidget.GetOpacity() != targetOpacity)
				m_wAreaLayoutWidget.SetOpacity(Math.Lerp(m_wAreaLayoutWidget.GetOpacity(), targetOpacity, timeSlice * POINTS_LAYOUT_FADE_SPEED));
		}

		// Update scoring
		foreach (SCR_KOTHTeamScoreDisplayObject scoring_object: m_aScoringElements) {
			if (!scoring_object) {
				continue;
			}

			if (!m_KOTHManager) {
				continue;
			}
			
			if (!scoring_object.GetFaction()) {
				continue;
			}
			
			scoring_object.UpdateScore(m_KOTHManager.GetTicketsForFaction(scoring_object.GetFaction()));
			scoring_object.UpdatePlayerCount(m_KOTHManager.GetAmountOfPlayersInZone(scoring_object.GetFaction()));
		}
	}
	
	void DebugPrint(string text)
	{
		Print(ToString() + text)
	}
}

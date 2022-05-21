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

	//! Parent frame that holds all scoring widgets
	protected Widget m_wScoringFrame;

	//! Used to display remaining time (if any).
	protected TextWidget m_wRemainingTimeWidget;

	//! Area manager provides us with necessary API
	protected KOTH_ZoneManager m_pKOTHManager;
	//! Game mode instance
	protected SCR_BaseGameMode m_pGameMode;

	//! Speed used to fade areas hud when hints are shown
	protected const float POINTS_LAYOUT_FADE_SPEED = 5.0;

	//------------------------------------------------------------------------------------------------
	/*!
		Checks the prerequisites for this InfoDisplay.
	*/
	override bool DisplayStartDrawInit(IEntity owner)
	{
		// No ui can be drawn without necessary items
		m_pGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!m_pGameMode)
			return false;

		if (!m_pGameMode.GetScoringSystemComponent())
			return false;

		m_pKOTHManager = KOTH_ZoneManager.Cast(m_pGameMode.FindComponent(KOTH_ZoneManager));
		if (!m_pKOTHManager)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Creates individual hud elements.
	*/
	override void DisplayStartDraw(IEntity owner)
	{
		if (m_wRoot)
		{
			// Root frame
			m_wScoringFrame = m_wRoot.FindAnyWidget("Score_Root");

			// USA "side"
			Widget usaParent = m_wRoot.FindAnyWidget("Score_USA");
			if (usaParent)
			{
				Faction us = GetGame().GetFactionManager().GetFactionByKey("US");
				ref SCR_KOTHTeamScoreDisplayObject usObj = new SCR_KOTHTeamScoreDisplayObject(usaParent, us);
				m_aScoringElements.Insert(usObj);
			}

			// USSR "side"
			Widget ussrParent = m_wRoot.FindAnyWidget("Score_USSR");
			if (ussrParent)
			{
				Faction ussr = GetGame().GetFactionManager().GetFactionByKey("USSR");
				ref SCR_KOTHTeamScoreDisplayObject ussrObj = new SCR_KOTHTeamScoreDisplayObject(ussrParent, ussr);
				m_aScoringElements.Insert(ussrObj);
			}
			
			// FIA "side"
			Widget fiaParent = m_wRoot.FindAnyWidget("Score_FIA");
			if (fiaParent)
			{
				Faction fia = GetGame().GetFactionManager().GetFactionByKey("FIA");
				ref SCR_KOTHTeamScoreDisplayObject fiaObj = new SCR_KOTHTeamScoreDisplayObject(fiaParent, fia);
				m_aScoringElements.Insert(fiaObj);
			}

			// Remaining time text in middle
			m_wRemainingTimeWidget = TextWidget.Cast(m_wRoot.FindAnyWidget("RemainingTime"));
		}
		else
		{
			Print("SCR_KOTHTeamScoreDisplay could not create scoring layout!");
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Clears all hud elements.
	*/
	override void DisplayStopDraw(IEntity owner)
	{
		// Clear all scoring elements
		for (int i = m_aScoringElements.Count() - 1; i >= 0; i--)
		{
			m_aScoringElements[i] = null;
			m_aScoringElements.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Updates the progress and state of all available elements.
	*/
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		SCR_BaseScoringSystemComponent scoringSystem = m_pGameMode.GetScoringSystemComponent();

		// Reposition scoring UI based on whether it is in a map or not
		if (m_wScoringFrame)
		{
			SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
			if (editorManager && editorManager.IsOpened())
			{
				FrameSlot.SetPos(m_wScoringFrame, 0.0, 72.0);
			}
			else
			{
				FrameSlot.SetPos(m_wScoringFrame, 0.0, 32.0);
			}
		}

		// Fade out points when a hint is shown to prevent clipping
		if (m_wAreaLayoutWidget)
		{
			SCR_PopUpNotification notifications = SCR_PopUpNotification.GetInstance();
			float targetOpacity = 1.0;
			if (notifications && notifications.IsShowing())
				targetOpacity = 0.0;

			if (m_wAreaLayoutWidget.GetOpacity() != targetOpacity)
				m_wAreaLayoutWidget.SetOpacity(Math.Lerp(m_wAreaLayoutWidget.GetOpacity(), targetOpacity, timeSlice * POINTS_LAYOUT_FADE_SPEED));
		}

		// Update scoring
		for (int i = 0, count = m_aScoringElements.Count(); i < count; i++)
		{
			SCR_KOTHTeamScoreDisplayObject scoringObject = m_aScoringElements[i];
			if (!scoringObject)
				continue;

			int score = 0;
			int maxScore = 0;
			if (scoringSystem)
			{
				score = scoringSystem.GetFactionScore(scoringObject.GetFaction());
				maxScore = scoringSystem.GetScoreLimit();
			}

			scoringObject.Update(score, maxScore);
		}

		// Update remaining time widget if any
		if (m_wRemainingTimeWidget)
		{
			if (m_pGameMode.GetTimeLimit() <= 0.0) // Disable showing if no time limit is set
				m_wRemainingTimeWidget.SetVisible(false);
			else
			{
				// Enable if time limit is set, clamp to 0
				float remainingTime = m_pGameMode.GetRemainingTime();
				if (remainingTime < 0.0)
					remainingTime = 0.0;

				m_wRemainingTimeWidget.SetVisible(true);
				m_wRemainingTimeWidget.SetText(SCR_FormatHelper.FormatTime(remainingTime));
			}
		}
	}
}

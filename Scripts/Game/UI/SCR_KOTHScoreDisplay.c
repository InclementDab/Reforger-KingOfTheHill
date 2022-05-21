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
		DebugPrint("::DisplayStartDraw - Start");
		
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
		}
		else
		{
			Print(ToString() + "::DisplayStartDraw - Could not create scoring layout!");
		}
		
		DebugPrint("::DisplayStartDraw - End");
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
		DebugPrint("::DisplayUpdate - Start");
		
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
			{
				DebugPrint("::DisplayUpdate - Big FFFFF");
				continue;
			}

			if (m_pKOTHManager)
			{				
				DebugPrint("::DisplayUpdate - Big POG");
				
				Faction us = GetGame().GetFactionManager().GetFactionByKey("US");				
				Faction ussr = GetGame().GetFactionManager().GetFactionByKey("USSR");				
				Faction fia = GetGame().GetFactionManager().GetFactionByKey("FIA");
				
				Faction elementFaction = scoringObject.GetFaction();
				DebugPrint("::DisplayUpdate - Element related faction:" + elementFaction);
				DebugPrint("::DisplayUpdate - Ticket max:" + m_pKOTHManager.GetTicketCountToWin());
				
				switch (elementFaction)
				{
					case us:
					{
						DebugPrint("::DisplayUpdate - Update USA Tickets");
						int usTickets = m_pKOTHManager.GetTicketsForFaction(us);
						int usPlayerCount = m_pKOTHManager.GetUSAPlayerCount();
						scoringObject.UpdateScore(usTickets);
						scoringObject.UpdatePlayerCount(usPlayerCount);
						break;
					}
					case ussr:
					{
						DebugPrint("::DisplayUpdate - Update USSR Tickets");
						int ussrTickets = m_pKOTHManager.GetTicketsForFaction(ussr);
						int ussrPlayerCount = m_pKOTHManager.GetUSSRPlayerCount();
						scoringObject.UpdateScore(ussrTickets);
						scoringObject.UpdatePlayerCount(ussrPlayerCount);
						break;
					}
					case ussr:
					{
						DebugPrint("::DisplayUpdate - Update FIA Tickets");
						int fiaTickets = m_pKOTHManager.GetTicketsForFaction(fia);
						int fiaPlayerCount = m_pKOTHManager.GetFIAPlayerCount();
						scoringObject.UpdateScore(fiaTickets);
						scoringObject.UpdatePlayerCount(fiaPlayerCount);
						break;
					}
				}
			}
		}
		
		DebugPrint("::DisplayUpdate - Start");
	}
	
	void DebugPrint(string text)
	{
		Print(ToString() + text)
	}
}

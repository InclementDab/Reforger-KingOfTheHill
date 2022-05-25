enum KOTHMenuScreenType
{
	VEHICLES = 0
};

class KOTH_KOTHSuperMenu : SCR_SuperMenuBase
{
	protected static KOTH_KOTHSuperMenu s_Instance;
	protected static bool s_bIsShown;
	protected SCR_LoadingOverlay m_Loading;

	static ref ScriptInvoker Event_OnMenuOpen = new ScriptInvoker();
	static ref ScriptInvoker Event_OnMenuClose = new ScriptInvoker();
	static ref ScriptInvoker Event_OnMenuShow = new ScriptInvoker();
	static ref ScriptInvoker Event_OnMenuHide = new ScriptInvoker();

	protected string m_sBack = "Back";
	protected string m_sTitle = "Title";

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		Event_OnMenuOpen.Invoke();

		Widget backBtn = GetRootWidget().FindAnyWidget(m_sBack);
		if (backBtn) {
			SCR_NavigationButtonComponent nav = SCR_NavigationButtonComponent.Cast(backBtn.FindHandler(SCR_NavigationButtonComponent));
			nav.m_OnActivated.Insert(OnMenuBack);
		}

		KOTH_GameModeBase gameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		if (!gameMode) {
			Print("KOTH super menu could not find KOTH_GameModeBase", LogLevel.ERROR);
			return;
		}

		super.OnMenuOpen();

		UpdateTabs();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpened()
	{
		super.OnMenuOpened();
	}

	//------------------------------------------------------------------------------------------------
	void OnMenuBack()
	{
		if (s_bIsShown)
			Close();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		GetRootWidget().SetEnabled(true);
		Event_OnMenuClose.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		if (!s_bIsShown)
			Event_OnMenuShow.Invoke();
		s_bIsShown = true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();
		s_bIsShown = false;
		Event_OnMenuHide.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();
		GetRootWidget().SetEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		super.OnMenuFocusLost();
		GetRootWidget().SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateTabs()
	{
		int selectedTab = m_TabViewComponent.GetShownTab();

		// enable individual submenu tabs based on gamemode settings:
		m_TabViewComponent.EnableTab(KOTHMenuScreenType.VEHICLES, true);
		
		int nextTab = m_TabViewComponent.GetNextValidItem(false);
		if (m_TabViewComponent.IsTabEnabled(nextTab))
			selectedTab = nextTab;

		// switch to the first enabled tab
		if (!m_TabViewComponent.IsTabEnabled(selectedTab))
			selectedTab = m_TabViewComponent.GetNextValidItem(false);

		if (selectedTab > -1)
			m_TabViewComponent.ShowTab(selectedTab, true, false);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateCurrentTab()
	{
		m_TabViewComponent.ShowTab(m_TabViewComponent.GetShownTab());
	}

	//------------------------------------------------------------------------------------------------
	void SetMenuTitle(string text)
	{
		TextWidget w = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sTitle));
		if (w)
			w.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	static KOTH_KOTHSuperMenu GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	void KOTH_KOTHSuperMenu()
	{
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	void ~KOTH_KOTHSuperMenu()
	{
		s_Instance = null;
	}
};
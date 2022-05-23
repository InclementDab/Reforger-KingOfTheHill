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
		if (backBtn)
		{
			SCR_NavigationButtonComponent nav = SCR_NavigationButtonComponent.Cast(backBtn.FindHandler(SCR_NavigationButtonComponent));
			nav.m_OnActivated.Insert(OnMenuBack);
		}

		KOTH_GameModeBase gameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		if (!gameMode)
		{
			Print("Respawn menu could not find any KOTH_GameModeBase", LogLevel.ERROR);
			return;
		}

		Widget loading = GetRootWidget().FindAnyWidget("Loading");
		if (loading)
		{
			m_Loading = SCR_LoadingOverlay.Cast(loading.FindHandler(SCR_LoadingOverlay));
		}

		super.OnMenuOpen();
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.PauseMenu);

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
		// Mute sounds
		// If menu is opened before loading screen is closed, wait for closing
		if (ArmaReforgerLoadingAnim.IsOpen())
			ArmaReforgerLoadingAnim.m_onExitLoadingScreen.Insert(MuteSounds);
		else
			MuteSounds();

		//GetGame().GetCallqueue().CallLater(MuteSounds, 100);
	}

	//------------------------------------------------------------------------------------------------
	void MuteSounds(bool mute = true)
	{
		if (!IsOpen())
			return;

		AudioSystem.SetMasterVolume(AudioSystem.SFX, !mute);
		AudioSystem.SetMasterVolume(AudioSystem.VoiceChat, !mute);
		AudioSystem.SetMasterVolume(AudioSystem.Dialog, !mute);

		ArmaReforgerLoadingAnim.m_onExitLoadingScreen.Remove(MuteSounds);
	}

	//------------------------------------------------------------------------------------------------
	void OnMenuBack()
	{
		if (s_bIsShown)
			GetGame().OpenPauseMenu(false, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		MuteSounds(false);
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
		MuteSounds(false);
		s_bIsShown = false;
		Event_OnMenuHide.Invoke();
		//GetGame().GetInputManager().RemoveActionListener("ShowScoreboard", EActionTrigger.DOWN, ShowPlayerList);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();
		GetRootWidget().SetEnabled(true);
		// Auto-open the editor (when enabled) on top of respawn menu for Game Masters
		/*SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.AutoInit();

		GetGame().GetInputManager().AddActionListener("ShowScoreboard", EActionTrigger.DOWN, ShowPlayerList);*/
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		super.OnMenuFocusLost();
		GetRootWidget().SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	/*void ShowPlayerList()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PlayerListMenu, 0, true, false);
	}*/

	//------------------------------------------------------------------------------------------------
	void UpdateTabs()
	{
		int selectedTab = m_TabViewComponent.GetShownTab();

		// enable individual submenu tabs based on gamemode settings:
		m_TabViewComponent.EnableTab(KOTHMenuScreenType.VEHICLES, true);
		//m_TabViewComponent.ShowTab(KOTHMenuScreenType.VEHICLES);

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
	void HandleOnVehicleAssigned()
	{

	}

	//------------------------------------------------------------------------------------------------
	void SetMenuTitle(string text)
	{
		TextWidget w = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sTitle));
		if (w)
			w.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	void SetLoadingVisible(bool visible)
	{
		m_Loading.SetShown(visible);
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
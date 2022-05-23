class KOTH_SubMenuBase : SCR_SubMenuBase
{
	protected KOTH_VehicleSelectionTileSelection m_TileSelection;
	protected InputManager m_InputManager;
	protected FactionManager m_FactionManager;
	protected KOTH_GameModeBase m_GameMode;
	protected int m_iPlayerId;

	protected SCR_NavigationButtonComponent m_ConfirmButton;
	protected SCR_NavigationButtonComponent m_QuickDeployButton;
	protected string m_sConfirmButtonText;

	[Attribute("{1F0A6C9C19E131C6}UI/Textures/Icons/icons_wrapperUI.imageset")]
	protected ResourceName m_sIcons;

	[Attribute("#AR-ButtonSelectDeploy")]
	protected LocalizedString m_sButtonTextSelectVehicle;

	[Attribute("#AR-Button_Confirm-UC")]
	protected LocalizedString m_sConfirm;

	//------------------------------------------------------------------------------------------------
	protected void GetWidgets()
	{
	}

	//------------------------------------------------------------------------------------------------
	protected bool ConfirmSelection()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		GetWidgets();

		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager)
			return;

		m_FactionManager = GetGame().GetFactionManager();
		if (!m_FactionManager)
			return;

		m_GameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		if (!m_GameMode)
			return;

		m_iPlayerId = SCR_PlayerController.GetLocalPlayerId();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuHide(parentMenu);
		GetGame().GetWorkspace().SetFocusedWidget(null);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(SCR_SuperMenuBase parentMenu, float tDelta)
	{
		super.OnMenuUpdate(parentMenu, tDelta);
	}

	//------------------------------------------------------------------------------------------------
	void KOTH_SubMenuBase()
	{

	}

	//------------------------------------------------------------------------------------------------
	void ~KOTH_SubMenuBase()
	{

	}
};
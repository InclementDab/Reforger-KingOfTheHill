class KOTH_VehicleSelectionSubMenu : KOTH_SubMenuBase
{
	protected static KOTH_VehicleSelectionSubMenu s_Instance;

	[Attribute("Tiles")]
	protected string m_sTileContainer;

	override void GetWidgets()
	{
		super.GetWidgets();
		Widget tileSelection = GetRootWidget().FindAnyWidget("TileSelection");
		if (tileSelection)
			m_TileSelection = KOTH_VehicleSelectionTileSelection.Cast(tileSelection.FindHandler(KOTH_VehicleSelectionTileSelection));
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected override bool ConfirmSelection()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	static KOTH_VehicleSelectionSubMenu GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	void KOTH_VehicleSelectionSubMenu()
	{
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	void ~KOTH_VehicleSelectionSubMenu()
	{
		s_Instance = null;
	}
};
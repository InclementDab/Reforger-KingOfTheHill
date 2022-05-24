class KOTH_VehicleSelectionvehicleEntry: ScriptedWidgetComponent
{
	protected Widget m_Root;
	protected ImageWidget m_Image;
	
	/*void KOTH_VehicleSelectionvehicleEntry(Widget parent)
	{
		
	}*/
}

class KOTH_VehicleSelectionSubMenu : KOTH_SubMenuBase
{
	protected static KOTH_VehicleSelectionSubMenu s_Instance;	
	protected KOTH_GameModeBase m_KOTHGameMode;

	//------------------------------------------------------------------------------------------------
	override void GetWidgets()
	{
		super.GetWidgets();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		
		CreateConfirmButton();
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
	protected void CreateConfirmButton()
	{
		m_ConfirmButton = CreateNavigationButton("MenuSelect", m_sConfirm, true);
		if (m_ConfirmButton)
		{
			m_ConfirmButton.m_OnActivated.Insert(HandleOnConfirm);
			m_ConfirmButton.GetRootWidget().SetZOrder(-1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HandleOnConfirm()
	{
		
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
		
		m_KOTHGameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
	}
		
	//------------------------------------------------------------------------------------------------
	void ~KOTH_VehicleSelectionSubMenu()
	{
		s_Instance = null;
	}
};
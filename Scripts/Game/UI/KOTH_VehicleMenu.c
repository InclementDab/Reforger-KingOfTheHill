enum KOTHDeployScreenType
{
	VEHICLES = 0
};


//------------------------------------------------------------------------------------------------
class SCR_KOTHSubMenuBase : SCR_SubMenuBase
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
	void SCR_KOTHSubMenuBase()
	{

	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_KOTHSubMenuBase()
	{

	}
};

//------------------------------------------------------------------------------------------------
class KOTH_VehicleSelectionSubMenu : SCR_KOTHSubMenuBase
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

class KOTH_VehicleSelectionTileSelection : ScriptedWidgetComponent
{
	[Attribute()]
	protected ResourceName m_sTilePrefab;

	[Attribute("Tiles")]
	protected string m_sTileContainer;

	protected Widget m_wTileContainer;

	protected KOTH_VehicleSelectionTile m_FocusedTile;
	protected ref array<KOTH_VehicleSelectionTile> m_aTiles = {};

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wTileContainer = w.FindAnyWidget(m_sTileContainer);
	}

	//------------------------------------------------------------------------------------------------
	void AddTile(KOTH_VehicleSelectionTile tile)
	{
		m_aTiles.Insert(tile);
	}

	//------------------------------------------------------------------------------------------------
	void Init()
	{
		int count = m_aTiles.Count();
	}

	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		if (m_aTiles.IsEmpty())
			return;

		Widget child = m_wTileContainer.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			child.RemoveFromHierarchy();
			child = sibling;
		}

		m_aTiles.Clear();
	}

	//------------------------------------------------------------------------------------------------
	KOTH_VehicleSelectionTile GetFocusedTile()
	{
		return m_FocusedTile;
	}

	//------------------------------------------------------------------------------------------------
	void FocusTile(KOTH_VehicleSelectionTile tile)
	{
		m_FocusedTile = tile;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetTileResource()
	{
		return m_sTilePrefab;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetTileContainer()
	{
		return m_wTileContainer;
	}

	//------------------------------------------------------------------------------------------------
	void SetTilesEnabled(bool enabled)
	{
		foreach (KOTH_VehicleSelectionTile tile : m_aTiles)
		{
			tile.SetEnabled(enabled);
		}
	}
};

//------------------------------------------------------------------------------------------------
class KOTH_VehicleSelectionTileBase : SCR_ButtonImageComponent
{
};

//------------------------------------------------------------------------------------------------
class KOTH_VehicleSelectionTile : KOTH_VehicleSelectionTileBase
{
	protected TextWidget m_wText;
	protected KOTH_VehicleSelectionTileSelection m_Parent;
	protected bool m_bClickEnabled = false;

	//------------------------------------------------------------------------------------------------
	protected void EnableButton()
	{
		m_bClickEnabled = true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_bClickEnabled)
			return false;

		super.OnClick(w, x, y, button);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wText = TextWidget.Cast(w.FindAnyWidget("Text"));
		GetGame().GetCallqueue().CallLater(EnableButton, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	void ShowTile(bool show)
	{
		GetRootWidget().SetVisible(show);
		GetRootWidget().SetEnabled(show);
	}

	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		if (m_wText)
			m_wText.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		if (m_Parent)
			m_Parent.FocusTile(this);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		if (m_Parent)
			m_Parent.FocusTile(null);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void SetParent(KOTH_VehicleSelectionTileSelection parent)
	{
		m_Parent = parent;
	}
};
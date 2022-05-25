class KOTH_VehicleSelectionSubMenu : SCR_SubMenuBase
{
	[Attribute("Tiles")]
	protected string m_sTileContainer;
	[Attribute("#AR-Button_Confirm-UC")]
	protected LocalizedString m_sButtonTextSelectVehicle;
	[Attribute("#AR-Button_Confirm-UC")]
	protected LocalizedString m_sConfirm;
	
	protected static KOTH_VehicleSelectionSubMenu s_Instance;
	protected KOTH_GameModeBase m_KOTHGameMode;
	protected KOTH_VehicleAssetInfo m_SelectedVehicle;
	protected ref map<KOTH_VehicleMenuTile, KOTH_VehicleAssetInfo> m_mAvailableVehicles = new ref map<KOTH_VehicleMenuTile, KOTH_VehicleAssetInfo>();
	protected KOTH_VehicleMenuTileSelection m_TileSelection;
	protected bool m_bVehicleRequestSent = false;
	protected bool m_bButtonsUnlocked = true;
	protected SCR_NavigationButtonComponent m_ConfirmButton;
	protected string m_sConfirmButtonText;
	//[Attribute("NoLoadoutSaved")]
	//protected string m_sNoArsenalLoadoutMessageID;

	void GetWidgets()
	{
		Widget tileSelection = GetRootWidget().FindAnyWidget("TileSelection");
		if (tileSelection)
			m_TileSelection = KOTH_VehicleMenuTileSelection.Cast(tileSelection.FindHandler(KOTH_VehicleMenuTileSelection));
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);

		//m_bIsLastAvailableTab = !GetRespawnMenuHandler().GetAllowSpawnPointSelection();
		CreateConfirmButton();
		//CreateQuickDeployButton();

		//PlayerManager pm = GetGame().GetPlayerManager();
		//SCR_RespawnComponent rc = SCR_RespawnComponent.Cast(pm.GetPlayerRespawnComponent(m_iPlayerId));
		//rc.GetLoadoutLockInvoker().Insert(LockLoadoutTiles);

		//m_LoadoutManager = GetGame().GetLoadoutManager();
		m_sConfirmButtonText = m_sButtonTextSelectVehicle;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);

		UpdateLoadouts();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateLoadouts()
	{
		//Faction faction = m_RespawnSystemComponent.GetPlayerFaction(m_iPlayerId);
		array<ref KOTH_VehicleAssetInfo> vehicles = new array<ref KOTH_VehicleAssetInfo>;
		Widget gallery = GetRootWidget().FindAnyWidget(m_sTileContainer);
		SCR_GalleryComponent gallery_component = SCR_GalleryComponent.Cast( gallery.GetHandler(0));
		gallery_component.ClearAll();	

		int vehiclesCnt;
		/*if (m_LoadoutManager)
		{
			if (faction)
			{
				loadoutCnt = m_LoadoutManager.GetPlayerLoadoutsByFaction(faction, loadouts);
			}
			else
			{
				loadouts = m_LoadoutManager.GetPlayerLoadouts();
				loadoutCnt = loadouts.Count();
			}
		}*/
		
		m_KOTHGameMode.GetVehicleAssetList(vehicles);
		vehiclesCnt = vehicles.Count();

		for (int i = 0; i < vehiclesCnt; ++i)
		{
			KOTH_VehicleMenuTile tile = KOTH_VehicleMenuTile.InitializeTile(m_TileSelection, vehicles[i]);			
			m_mAvailableVehicles.Set(tile, vehicles[i]);
			tile.m_OnClicked.Insert(HandleOnConfirm);
		}

		m_TileSelection.Init();
	}

	//------------------------------------------------------------------------------------------------
	protected int GetWeaponSlots(IEntitySource prefab, out array<IEntityComponentSource> slots)
	{
		if (!prefab)
			return -1;

		int componentsCount = prefab.GetComponentCount();
		for (int i = 0; i < componentsCount; ++i)
		{
			IEntityComponentSource compSrc = prefab.GetComponent(i);
			if (compSrc.GetClassName() == "CharacterWeaponSlotComponent")
				slots.Insert(compSrc);
		}

		return slots.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected KOTH_VehicleAssetInfo GetSelectedVehicle()
	{
		if (m_TileSelection)
		{
			KOTH_VehicleMenuTile tile = KOTH_VehicleMenuTile.Cast(m_TileSelection.GetFocusedTile());
			m_SelectedVehicle = m_mAvailableVehicles.Get(tile);
		}

		return m_SelectedVehicle;
	}

	//------------------------------------------------------------------------------------------------
	protected bool ConfirmSelection()
	{
		if (GetSelectedVehicle())
		{
			//! TODO!!
		}
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void LockVehicleTiles(bool locked)
	{
		m_bButtonsUnlocked = !locked;
		m_TileSelection.SetTilesEnabled(!locked);
	}

	//------------------------------------------------------------------------------------------------
	void HandleOnVehicleAssigned(int playerId, KOTH_VehicleAssetInfo vehicle)
	{
	}
	
	protected void CreateConfirmButton()
	{
		m_ConfirmButton = CreateNavigationButton("MenuSelect", m_sConfirm, true);
		if (m_ConfirmButton)
		{
			m_ConfirmButton.m_OnActivated.Insert(HandleOnConfirm);
			m_ConfirmButton.GetRootWidget().SetZOrder(-1);
		}
	}

	protected void HandleOnConfirm()
	{
		bool assignResult = ConfirmSelection();
		m_bVehicleRequestSent = false;
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
		//s_Instance = null;
	}
};
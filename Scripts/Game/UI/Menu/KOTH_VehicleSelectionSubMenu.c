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
	protected SCR_NavigationButtonComponent m_ConfirmButton;
	protected string m_sConfirmButtonText;
	protected ref array<ref KOTH_VehicleAssetInfo> m_Vehicles = new array<ref KOTH_VehicleAssetInfo>;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		Print(ToString() + "::OnMenuOpen - Start");
		
		super.OnMenuOpen(parentMenu);
		
		Widget tileSelection = GetRootWidget().FindAnyWidget("TileSelection");
		if (tileSelection) 
			m_TileSelection = KOTH_VehicleMenuTileSelection.Cast(tileSelection.FindHandler(KOTH_VehicleMenuTileSelection));
		
		CreateConfirmButton();
		m_sConfirmButtonText = m_sButtonTextSelectVehicle;
		
		Print(ToString() + "::OnMenuOpen - End");
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);

		UpdateVehicles();
	}

	//! Update vehicle ui list from game mode vehicle prefab config
	//------------------------------------------------------------------------------------------------
	protected void UpdateVehicles()
	{
		Widget gallery = GetRootWidget().FindAnyWidget(m_sTileContainer);
		SCR_GalleryComponent gallery_component = SCR_GalleryComponent.Cast( gallery.GetHandler(0));
		gallery_component.ClearAll();	

		m_KOTHGameMode.GetVehicleAssetList(m_Vehicles);
		for (int i = 0; i < m_Vehicles.Count(); ++i) {
			Resource res = Resource.Load(m_Vehicles[i].GetPrefab());
			BaseResourceObject baseObj = res.GetResource();
			IEntityComponentSource source = SCR_BaseContainerTools.FindComponentSource(res, "SCR_EditableVehicleComponent");
			BaseContainer container = source.GetObject("m_UIInfo");
			SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
			
			ResourceName path;
			//! Get the vehicle image
			container.Get("m_Image", path);
			
			KOTH_VehicleMenuTile tile = KOTH_VehicleMenuTile.InitializeTile(m_TileSelection, m_Vehicles[i]);			
			m_mAvailableVehicles.Set(tile, m_Vehicles[i]);
			//tile.m_OnClicked.Insert(HandleOnConfirm);
		}

		m_TileSelection.Init();
	}

	//------------------------------------------------------------------------------------------------
	protected KOTH_VehicleAssetInfo GetSelectedVehicle()
	{
		if (m_TileSelection) {
			KOTH_VehicleMenuTile tile = KOTH_VehicleMenuTile.Cast(m_TileSelection.GetFocusedTile());
			m_SelectedVehicle = m_mAvailableVehicles.Get(tile);
		}

		return m_SelectedVehicle;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on button click
	protected bool ConfirmSelection()
	{
		Print(ToString() + "::ConfirmSelection - Start");
		
		if (GetSelectedVehicle()) {
			Print(ToString() + "::ConfirmSelection - Vehicle selected!");
			PlayerController playerController = GetGame().GetPlayerController();
			if (!playerController)
			{
				Print(ToString() + "::ConfirmSelection - F 1");
				return false;
			}
			
			KOTH_DeliveryPoint spawnPointUSA = KOTH_DeliveryPoint.Cast(m_KOTHGameMode.FindEntityByName("VehicleRequestSpawnUSA"));
			if (!spawnPointUSA)
			{
				Print(ToString() + "::ConfirmSelection - F 2");
			}
			
			int prefabID;
			for (int i = 0; i < m_Vehicles.Count(); ++i) {
				if (m_Vehicles[i] == GetSelectedVehicle())
					prefabID = i;
			}
			
			// Find campaign network component to send RPC to server
			KOTH_NetworkComponent kothNetworkComponent = KOTH_NetworkComponent.Cast(playerController.FindComponent(KOTH_NetworkComponent));
			if (kothNetworkComponent)
			{
				Print(ToString() + "::ConfirmSelection - Try to spawn: " + prefabID);
				kothNetworkComponent.SpawnVehicle(spawnPointUSA, prefabID);
				return true;
			}
		}
		
		Print(ToString() + "::ConfirmSelection - End and return: FALSE");
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on button click
	void HandleOnVehicleAssigned(int playerId, KOTH_VehicleAssetInfo vehicle)
	{
		Print(ToString() + "::HandleOnVehicleAssigned - Start");
		
		Print(ToString() + "::HandleOnVehicleAssigned - End");
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void CreateConfirmButton()
	{
		m_ConfirmButton = CreateNavigationButton("MenuSelect", m_sConfirm, true);
		if (m_ConfirmButton) {
			m_ConfirmButton.m_OnActivated.Insert(HandleOnConfirm);
			m_ConfirmButton.GetRootWidget().SetZOrder(-1);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleOnConfirm()
	{
		Print(ToString() + "::HandleOnConfirm - Start");
		
		ConfirmSelection();
		
		Print(ToString() + "::HandleOnConfirm - End");
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
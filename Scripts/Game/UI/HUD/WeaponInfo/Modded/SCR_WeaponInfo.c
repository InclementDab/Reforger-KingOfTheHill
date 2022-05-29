modded class SCR_WeaponInfo : SCR_InfoDisplayExtended
{
	protected const ResourceName DEFAULT_WEAPON_INFO_LAYOUT = "{805AEE9643457962}UI/layouts/HUD/WeaponInfo/WeaponInfo.layout";

	protected RichTextWidget m_AmmoCount;
		
	void SetAmmoCount(int count)
	{
		GetWidgets().GetAmmoCountWidget().SetText(count.ToString());
		AnimateWidget_ColorFlash(GetWidgets().GetAmmoCountWidget(), EWeaponFeature.AMMOCOUNT);	
		AnimateWidget_TextPopUp(GetWidgets().GetAmmoCountWidget(), 36, 54, EWeaponFeature.AMMOCOUNT);
	}
	
	override void OnMagazineChanged(BaseWeaponComponent weapon, BaseMagazineComponent magazine, BaseMagazineComponent prevMagazine)
	{
		super.OnMagazineChanged(weapon, magazine, prevMagazine);
		
		if (magazine) SetAmmoCount(magazine.GetAmmoCount());
	}
	
	override void OnAmmoCountChanged_init(BaseWeaponComponent weapon, BaseMagazineComponent magazine)
	{
		super.OnAmmoCountChanged_init(weapon, magazine);
		
		if (magazine) SetAmmoCount(magazine.GetAmmoCount());
	}
	
	override void OnAmmoCountChanged(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle, BaseMagazineComponent magazine, int ammoCount, bool isBarrelChambered)
	{
		super.OnAmmoCountChanged(weapon, muzzle, magazine, ammoCount, isBarrelChambered);
		
		if (magazine) SetAmmoCount(ammoCount);
	}

	override static InventoryStorageManagerComponent GetInventoryManager()
	{		
		PlayerController pc = GetGame().GetPlayerController();
			
		if (!pc)
			return null;
		
		IEntity playerEntity = pc.GetControlledEntity();
		
		if (!playerEntity)
			return null;
		
		InventoryStorageManagerComponent invManager = InventoryStorageManagerComponent.Cast(playerEntity.FindComponent(InventoryStorageManagerComponent));
		
		CompartmentAccessComponent compAccess = CompartmentAccessComponent.Cast(playerEntity.FindComponent(CompartmentAccessComponent));
		
		if (!compAccess)
			return invManager;
		
		BaseCompartmentSlot compSlot = compAccess.GetCompartment();
		
		if (!compSlot)
			return invManager;

		IEntity vehEntity = compSlot.GetVehicle();
		InventoryStorageManagerComponent vehInvMgr = InventoryStorageManagerComponent.Cast(
			vehEntity.FindComponent(InventoryStorageManagerComponent));
		
		if (!vehInvMgr)
		{
			vehEntity = compSlot.GetOwner();
			vehInvMgr = InventoryStorageManagerComponent.Cast(vehEntity.FindComponent(InventoryStorageManagerComponent));		
		}
		
		if (!vehInvMgr)
			return null;
		
		return vehInvMgr;
	}
	
	SCR_WeaponInfoWidgets GetWidgets()
	{
		return widgets;
	}
}

modded class SCR_WeaponInfoWidgets
{
	static const ResourceName m_LayoutRoot = "{805AEE9643457962}UI/layouts/HUD/WeaponInfo/WeaponInfo.layout";
	
	protected RichTextWidget m_MagAmmoCountText;
	
	override ResourceName GetLayout() 
	{
		return m_LayoutRoot; 
	}

	override bool Init(Widget root)
	{
		super.Init(root);
		
		m_MagAmmoCountText = RichTextWidget.Cast(root.FindAnyWidget("m_MagazineAmmoCount"));
		
		return true;
	}
	
	RichTextWidget GetAmmoCountWidget()
	{
		return m_MagAmmoCountText;
	}
};

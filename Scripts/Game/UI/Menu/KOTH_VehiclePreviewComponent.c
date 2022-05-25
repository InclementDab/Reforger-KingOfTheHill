class KOTH_VehiclePreviewComponent : ScriptedWidgetComponent
{
	[Attribute("{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et")]
	protected ResourceName m_sPreviewManager;
	protected ItemPreviewManagerEntity m_PreviewManager;
	protected ItemPreviewWidget m_wPreview;

	protected IEntity m_PreviewedVehicle;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wPreview = ItemPreviewWidget.Cast(w.FindAnyWidget("Preview"));
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_PreviewedVehicle)
			delete m_PreviewedVehicle;
	}

	//------------------------------------------------------------------------------------------------
	IEntity SetPreviewedVehicle(notnull KOTH_VehicleAssetInfo vehicle)
	{
		m_PreviewManager = GetGame().GetItemPreviewManager();

		if (!m_PreviewManager)
		{
			Resource res = Resource.Load(m_sPreviewManager);
			if (res.IsValid())
				GetGame().SpawnEntityPrefabLocal(res);
			m_PreviewManager = GetGame().GetItemPreviewManager();
			if (!m_PreviewManager)
			{
				return null;
			}
		}

		ResourceName resName = vehicle.GetPrefab();
		Resource res = Resource.Load(resName);

		if (m_PreviewedVehicle)
			delete m_PreviewedVehicle;

		m_PreviewedVehicle = GetGame().SpawnEntityPrefabLocal(res);

		if (m_PreviewedVehicle && m_wPreview)
		{
			m_PreviewManager.SetPreviewItem(m_wPreview, m_PreviewedVehicle);
			m_PreviewedVehicle.ClearFlags(EntityFlags.ACTIVE | EntityFlags.TRACEABLE, true);
			m_PreviewedVehicle.SetFlags(EntityFlags.NO_LINK, true);
		}

		return m_PreviewedVehicle;
	}
};
class KOTH_VehicleMenuTileSelection : ScriptedWidgetComponent
{
	[Attribute()]
	protected ResourceName m_sTilePrefab;

	[Attribute("Tiles")]
	protected string m_sTileContainer;

	protected Widget m_wTileContainer;

	protected KOTH_VehicleMenuTile m_FocusedTile;
	protected ref array<KOTH_VehicleMenuTile> m_aTiles = {};

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wTileContainer = w.FindAnyWidget(m_sTileContainer);
	}

	//------------------------------------------------------------------------------------------------
	void AddTile(KOTH_VehicleMenuTile tile)
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
		while (child) {
			Widget sibling = child.GetSibling();
			child.RemoveFromHierarchy();
			child = sibling;
		}

		m_aTiles.Clear();
	}

	//------------------------------------------------------------------------------------------------
	KOTH_VehicleMenuTile GetFocusedTile()
	{
		return m_FocusedTile;
	}

	//------------------------------------------------------------------------------------------------
	void FocusTile(KOTH_VehicleMenuTile tile)
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
		foreach (KOTH_VehicleMenuTile tile : m_aTiles) {
			tile.SetEnabled(enabled);
		}
	}
};
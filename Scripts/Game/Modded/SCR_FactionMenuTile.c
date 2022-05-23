modded class SCR_FactionMenuTile
{
	static override SCR_FactionMenuTile InitializeTile(SCR_DeployMenuTileSelection parent, SCR_Faction faction)
	{
		Widget tile = GetGame().GetWorkspace().CreateWidgets("{5968FE6DF3F3853B}UI/layouts/Menus/RoleSelection/DeployMenuTile.layout");
		SCR_FactionMenuTile handler = SCR_FactionMenuTile.Cast(tile.FindHandler(SCR_FactionMenuTile));
		SCR_GalleryComponent gallery_handler = SCR_GalleryComponent.Cast(parent.GetTileContainer().GetHandler(0));
		if (!handler) {
			return null;
		}

		handler.SetParent(parent);
		handler.SetImage(faction.GetFactionFlag());
		handler.SetText(faction.GetFactionName());
		handler.SetFactionBackgroundColor(faction.GetFactionColor());
		gallery_handler.AddItem(tile);
		return handler;
	}

	void SetButtonState(bool state, bool animate = true)
	{
		m_bClickEnabled = state;
		if (!m_bClickEnabled) {
			OnDisabled(animate);
			OnMouseLeave(m_wRoot, null, 0, 0);
		} else {
			OnEnabled(animate);
		}

		m_wRoot.SetEnabled(m_bClickEnabled);
	}

	override bool OnFocus(Widget w, int x, int y)
	{
		if (!m_bClickEnabled) {
			return false;
		}

		return super.OnFocus(w, x, y);
	}
}

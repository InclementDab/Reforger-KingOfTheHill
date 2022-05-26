modded class SCR_FactionMenuTile
{
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

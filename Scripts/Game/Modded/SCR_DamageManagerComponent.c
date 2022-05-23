// hack
modded class SCR_DamageManagerComponent// hack
{// hack
	// hack
	protected bool m_GodMode;// hack
	// hack
	void SetGodMode(bool god_mode)// hack
	{// hack
		m_GodMode = god_mode;// hack
	}// hack
	// hack
	override void OnDamageStateChanged(EDamageState state)// hack
	{// hack
		super.OnDamageStateChanged(state);// hack
		// hack
		if (m_GodMode) {// hack
			FullHeal();// hack
		}// hack
	}// hack
}// hack
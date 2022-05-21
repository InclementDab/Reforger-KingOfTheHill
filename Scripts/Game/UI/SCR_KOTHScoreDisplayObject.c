//------------------------------------------------------------------------------------------------
/*!
	This object serves as a wrapper for individual HUD team score widgets.
*/
class KOTH_TeamScoreDisplayObject
{
	//! Topmost frame of this object
	protected Widget m_Root;

	//! Reference to text widget that displays player count
	protected TextWidget m_PlayerCountText;
	
	//! Reference to fillable bar widget
	protected ImageWidget m_FactionImage;
	
	//! Reference to fillable bar widget
	protected ImageWidget m_BackgroundImage;	

	//! Reference to text widget that displays score
	protected TextWidget m_ScoreText;

	//! Faction this object represents
	protected Faction m_Faction;

	void KOTH_TeamScoreDisplayObject(notnull Widget root, notnull Faction faction)
	{
		m_Root = root;
		m_Faction = faction;
		
		m_FactionImage = ImageWidget.Cast(m_Root.FindAnyWidget("Icon_Faction"));
		m_BackgroundImage = ImageWidget.Cast(m_Root.FindAnyWidget("Icon_Background"));
		m_ScoreText = TextWidget.Cast(m_Root.FindAnyWidget("ScoreBar_Text"));
		m_PlayerCountText = TextWidget.Cast(m_Root.FindAnyWidget("ScoreBar_PlayerCount")); 

		ResourceName icon_resource;
		SCR_Faction script_faction = SCR_Faction.Cast(faction);
		if (script_faction) {
			icon_resource = script_faction.GetFactionFlag();
		} else {
			// Set icon directly
			UIInfo faction_info = faction.GetUIInfo();
			if (faction_info) {
				icon_resource = faction_info.GetIconPath();
			}
		}

		if (!icon_resource.IsEmpty()) {
			m_FactionImage.SetColor(Color.White);
			m_FactionImage.LoadImageTexture(0, icon_resource);
			m_FactionImage.SetImage(0);
		} else {
			m_FactionImage.SetColor(faction.GetFactionColor());
		}

		// Color elements
		m_Root.SetColor(faction.GetFactionColor());
	}
		
	void DoBlink(float speed)
	{
		Color color;
		if (speed > 0) {
			color = Color.FromRGBA(46, 204, 113, 140);
			m_BackgroundImage.SetColor(color);
		}
		else if (speed <= 0) {
			color = Color.FromRGBA(0, 0, 0, 0);
			m_BackgroundImage.SetColor(color);
		}
		
		WidgetAnimator.PlayAnimation(new WidgetAnimationOpacity(m_BackgroundImage, speed, 0, true));
	}
	
	void UpdateScore(int score)
	{
		m_ScoreText.SetText(score.ToString());
	}
	
	void UpdatePlayerCount(int count)
	{
		m_PlayerCountText.SetText(count.ToString());
	}

	Faction GetFaction()
	{
		return m_Faction;
	}
}

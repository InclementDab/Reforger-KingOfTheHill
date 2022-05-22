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
	protected KOTH_Faction m_Faction;

	void KOTH_TeamScoreDisplayObject(notnull Widget root, notnull KOTH_Faction faction)
	{
		m_Root = root;
		m_Faction = faction;
		
		m_FactionImage = ImageWidget.Cast(m_Root.FindAnyWidget("Icon_Faction"));
		m_BackgroundImage = ImageWidget.Cast(m_Root.FindAnyWidget("Icon_Background"));
		m_ScoreText = TextWidget.Cast(m_Root.FindAnyWidget("ScoreBar_Text"));
		m_PlayerCountText = TextWidget.Cast(m_Root.FindAnyWidget("ScoreBar_PlayerCount")); 

		ResourceName icon_resource = faction.GetFactionFlag();
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
		
	void DoBlink(float speed, bool tie)
	{
		Color color;
		
		if (!tie) color = Color.FromRGBA(46, 204, 113, 240);
		else color = Color.FromRGBA(231, 76, 113, 240);
		m_BackgroundImage.SetColor(color);
		WidgetAnimator.PlayAnimation(new WidgetAnimationOpacity(m_BackgroundImage, speed, 0, true));
	}
	
	void StopBlink()
	{
		m_BackgroundImage.SetColor(Color.FromRGBA(0, 0, 0, 0));
		WidgetAnimator.StopAllAnimations(m_BackgroundImage);
	}
	
	void UpdateScore(int score)
	{
		m_ScoreText.SetText(score.ToString());
	}
	
	void UpdatePlayerCount(int count)
	{
		m_PlayerCountText.SetText(count.ToString());
	}

	KOTH_Faction GetFaction()
	{
		return m_Faction;
	}
}

class KOTH_ObjectiveDisplayObject
{
	//! Topmost frame of this object
	protected Widget m_Root;
	
	//! Reference to text widget that displays distance
	protected TextWidget m_DistanceText;
	
	//! Reference to outline image widget
	protected ImageWidget m_OutlinesImage;
	
	//! Reference to outline image widget
	protected ImageWidget m_ControlledImage;
	
	void KOTH_ObjectiveDisplayObject(notnull Widget root)
	{
		m_Root = root;
				
		m_DistanceText = TextWidget.Cast(m_Root.FindAnyWidget("Distance"));
		m_OutlinesImage = ImageWidget.Cast(m_Root.FindAnyWidget("Icon_Outlines"));
		m_ControlledImage = ImageWidget.Cast(m_Root.FindAnyWidget("Icon_Controlled"));
	}
}
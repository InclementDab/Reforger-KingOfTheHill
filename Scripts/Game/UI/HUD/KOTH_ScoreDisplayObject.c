/*!
	This object serves as a wrapper for individual HUD team score widgets.
*/

enum ScoreDiplayObjectBlinkState
{
	OFF,
	TIE,
	OWNED
}

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
	
	//! Reference to the player icon next to player count
	protected ImageWidget m_PlayerImage;

	//! Reference to text widget that displays score
	protected TextWidget m_ScoreText;

	//! Faction this object represents
	protected KOTH_Faction m_Faction;
	
	protected ScoreDiplayObjectBlinkState m_BlinkState;

	void KOTH_TeamScoreDisplayObject(notnull Widget root, notnull KOTH_Faction faction)
	{
		m_Root = root;
		m_Faction = faction;
		
		m_FactionImage = ImageWidget.Cast(m_Root.FindAnyWidget("Icon_Faction"));
		m_BackgroundImage = ImageWidget.Cast(m_Root.FindAnyWidget("Icon_Background"));
		m_ScoreText = TextWidget.Cast(m_Root.FindAnyWidget("ScoreBar_Text"));
		m_PlayerCountText = TextWidget.Cast(m_Root.FindAnyWidget("ScoreBar_PlayerCount")); 
		m_PlayerImage = ImageWidget.Cast(m_Root.FindAnyWidget("ScoreBar_PlayerImage")); 

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
		
	void SetBlinkState(ScoreDiplayObjectBlinkState blink_state)
	{
		m_BlinkState = blink_state;
		
		switch (m_BlinkState) {
			case ScoreDiplayObjectBlinkState.OFF: {
				m_BackgroundImage.SetVisible(false);
				//m_BackgroundImage.SetColor(Color.FromRGBA(0, 0, 0, 255));
				WidgetAnimator.StopAnimation(m_BackgroundImage, WidgetAnimationType.Opacity);
				break;
			}
			
			case ScoreDiplayObjectBlinkState.TIE: {
				WidgetAnimator.StopAllAnimations(m_BackgroundImage);
				m_BackgroundImage.SetVisible(true);
				m_BackgroundImage.SetColor(Color.FromRGBA(231, 76, 113, 255));
				//WidgetAnimator.PlayAnimation(new WidgetAnimationOpacity(m_BackgroundImage, speed, 0, true, false));
				WidgetAnimator.PlayAnimation(m_BackgroundImage, WidgetAnimationType.Opacity, 1, 0, true);
				break;
			}
			
			case ScoreDiplayObjectBlinkState.OWNED: {
				WidgetAnimator.StopAllAnimations(m_BackgroundImage);
				m_BackgroundImage.SetVisible(true);
				m_BackgroundImage.SetColor(Color.FromRGBA(46, 204, 113, 255));
				WidgetAnimator.PlayAnimation(m_BackgroundImage, WidgetAnimationType.Opacity, 1, 0, true);
			}
		}
	}
		
	void UpdateScore(int score)
	{
		m_ScoreText.SetText(score.ToString());
	}
	
	void UpdatePlayerCount(int count, bool in_zone)
	{
		m_PlayerCountText.SetText(count.ToString());
		m_PlayerImage.SetColor(Color.FromRGBA(255 * !in_zone, 255, 255 * !in_zone, 255));
	}

	KOTH_Faction GetFaction()
	{
		return m_Faction;
	}
};
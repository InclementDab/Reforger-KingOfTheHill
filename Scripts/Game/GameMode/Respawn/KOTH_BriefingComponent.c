[ComponentEditorProps(category: "GameScripted/GameMode/KOTH", description: "Briefing screen shown in respawn menu.")]
class KOTH_BriefingComponentClass : SCR_RespawnBriefingComponentClass
{
}

//------------------------------------------------------------------------------------------------
/*!
	This component provides Capture & Hold specific data for the respawn menu briefing screen.
*/
class KOTH_BriefingComponent : SCR_RespawnBriefingComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Image to be shown on the briefing screen.", "edds")]
	protected ResourceName m_sBriefingImage;

	[Attribute("0", UIWidgets.CheckBox, "If enabled and mission header image is available, it will be used instead.", "edds")]
	protected bool m_bUseMissionHeaderImage;

	//------------------------------------------------------------------------------------------------
	/*!
		Returns image resource that should be shown in the briefing screen.
	*/
	ResourceName GetBriefingImagePath()
	{
		if (m_bUseMissionHeaderImage)
		{
			SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
			if (!missionHeader)
				return ResourceName.Empty;

			return missionHeader.m_sLoadingScreen;
		}

		return m_sBriefingImage;
	}
}

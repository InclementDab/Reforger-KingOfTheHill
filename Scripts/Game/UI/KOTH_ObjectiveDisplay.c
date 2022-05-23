class KOTH_ZoneObjectiveDisplayObject
{
	//! Topmost widget of this object
	protected Widget m_wRoot;
	//! Text that displays objective distance
	protected TextWidget m_wDistanceText;
	//! Text that displays name or symbol of the objective
	protected TextWidget m_wNameText;

	//! The objective image
	protected ImageWidget m_wIcon;
	//! The objective is under controll symbol image
	protected ImageWidget m_wControlledIcon;
	//! The major objective symbol image
	protected ImageWidget m_wMajorIcon;
	//! The backdrop image of this element
	protected ImageWidget m_wBackdropIcon;

	protected VerticalLayoutWidget m_wLayout;

	//! Blend progress of when within the area
	protected float m_fBlendScreenPosition;
	//! Color blending rate
	protected const float COLOR_BLEND_SPEED = 5.5;
	//! Position blending rate
	protected const float POSITION_BLEND_SPEED = 2.5;

	// Initial values
	protected float m_fOriginalXSize;
	protected float m_fOriginalYSize;
	protected float m_fOriginalTitleSize;
	protected float m_fOriginalTextSize;

	// Loaded texture indices
	protected const int ATLAS_INDEX_NEUTRAL = 0;
	protected const int ATLAS_INDEX_BLUFOR = 1;
	protected const int ATLAS_INDEX_OPFOR = 2;

	//! The area this object represents
	protected KOTH_ZoneTriggerEntity m_AffiliatedZone;
	protected KOTH_ZoneManager m_ZoneManager;

	//------------------------------------------------------------------------------------------------
	/*!
		Returns the area this element represents.
	*/
	KOTH_ZoneTriggerEntity GetZone()
	{
		return m_AffiliatedZone;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Returns root widget of this element.
	*/
	Widget GetRootWidget()
	{
		return m_wRoot;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Create new wrapper for objective display.
	*/
	void KOTH_ZoneObjectiveDisplayObject(notnull Widget root, notnull KOTH_ZoneTriggerEntity area, notnull KOTH_ZoneManager zoneManager)
	{
		m_wRoot = root;
		m_AffiliatedZone = area;
		m_ZoneManager = zoneManager;
		
		m_wNameText = TextWidget.Cast(m_wRoot.FindAnyWidget("Name"));
		m_fOriginalTitleSize = 15;

		m_wDistanceText = TextWidget.Cast(m_wRoot.FindAnyWidget("Distance"));
		m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon_Faction"));
		m_wBackdropIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon_Backdrop"));

		m_wControlledIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon_Controlled"));
		m_wLayout = VerticalLayoutWidget.Cast(m_wRoot.FindAnyWidget("VerticalLayout"));
		m_fOriginalXSize = FrameSlot.GetSizeX(m_wLayout);
		m_fOriginalYSize = FrameSlot.GetSizeY(m_wLayout);

		m_wMajorIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon_MajorMark"));

		m_fOriginalTextSize= 14;
		/*const string atlasImageset = "{225B7CAD5CEC4AE3}UI/Imagesets/CaptureAndHold/CaptureAndHoldAtlas.imageset";
		m_wIcon.LoadImageFromSet(ATLAS_INDEX_NEUTRAL, atlasImageset, "CAH_NEUTRAL_LARGE");
		m_wIcon.LoadImageFromSet(ATLAS_INDEX_BLUFOR, atlasImageset, "CAH_BLUFOR_LARGE");
		m_wIcon.LoadImageFromSet(ATLAS_INDEX_OPFOR, atlasImageset, "CAH_OPFOR_LARGE");

		// BACKDROPS
		m_wBackdropIcon.LoadImageFromSet(ATLAS_INDEX_NEUTRAL, atlasImageset, "CAH_NEUTRAL_BACKDROP");
		m_wBackdropIcon.LoadImageFromSet(ATLAS_INDEX_BLUFOR, atlasImageset, "CAH_BLUFOR_BACKDROP");
		m_wBackdropIcon.LoadImageFromSet(ATLAS_INDEX_OPFOR, atlasImageset, "CAH_OPFOR_BACKDROP");*/
	}

	//------------------------------------------------------------------------------------------------
	void ~KOTH_ZoneObjectiveDisplayObject()
	{
	}

	//------------------------------------------------------------------------------------------------
	/*
		Update image state according to provided area state
	*/
	protected void UpdateImageState(Widget root, float timeSlice)
	{
		//Faction owningFaction = m_AffiliatedZone.GetOwningFaction();
		KOTH_Faction owningFaction = m_ZoneManager.GetZoneOwner();

		Color targetColor;
		if (!owningFaction)
			targetColor = Color.FromRGBA(249, 210, 103, 255);
		else
			targetColor = owningFaction.GetFactionColor();

		Color currentColor = m_wRoot.GetColor();
		if (currentColor != targetColor)
			currentColor.Lerp(targetColor, timeSlice * COLOR_BLEND_SPEED);

		// Only show major symbol when area is marked as one
		//m_wMajorIcon.SetVisible(m_AffiliatedZone.IsMajor());

		// Area is completely neutral
		if (!owningFaction)
		{
			m_wIcon.SetImage(ATLAS_INDEX_NEUTRAL);
			m_wBackdropIcon.SetImage(ATLAS_INDEX_NEUTRAL);
			m_wRoot.SetColor(currentColor);
			// With no owning faction, there is no contesting faction
			m_wControlledIcon.SetVisible(false);
			return;
		}

		// TODO: Improvement desirable
		// This is far from ideal and is not sandbox enough.
		FactionKey factionKey = owningFaction.GetFactionKey();
		if (factionKey == "US" || factionKey == "FIA")
		{
			m_wIcon.SetImage(ATLAS_INDEX_BLUFOR);
			m_wBackdropIcon.SetImage(ATLAS_INDEX_BLUFOR);
			m_wControlledIcon.SetRotation(0);
		}
		else if (factionKey == "USSR")
		{
			m_wIcon.SetImage(ATLAS_INDEX_OPFOR);
			m_wBackdropIcon.SetImage(ATLAS_INDEX_OPFOR);
			m_wControlledIcon.SetRotation(45);
		}

		m_wRoot.SetColor(currentColor);

		/*Faction contestingFaction = m_AffiliatedZone.GetContestingFaction();
		if (contestingFaction)
		{
			m_wControlledIcon.SetVisible(true);
			// Pulsing animation
			//Color col = new Color(1.0, 1.0, 1.0, 1.0);
			Color col = Color.FromRGBA(249, 210, 103, 255);
			float val01 = Math.Sin( m_AffiliatedZone.GetWorld().GetWorldTime() * 0.01 ) * 0.5 + 0.5;
			col.Lerp(contestingFaction.GetFactionColor(), val01);
			m_wControlledIcon.SetColor(col);

			//Color selfCol = new Color(1.0, 1.0, 1.0, 1.0);
			Color selfCol = Color.FromRGBA(249, 210, 103, 255);
			selfCol.Lerp(owningFaction.GetFactionColor(), val01);
			m_wRoot.SetColor(selfCol);
		}
		else
		{
			m_wControlledIcon.SetVisible(false);
		}*/
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Update this widget as static UI element, ie. not reprojecting it automatically.
	*/
	void UpdateStatic(float timeSlice)
	{
		m_wDistanceText.SetVisible(false);
		UpdateImageState(m_wRoot, timeSlice);
		// Set objective symbol
		m_wNameText.SetText("KOTH");
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Update this widget as dynamic UI element projected to screen space
	*/
	void UpdateDynamic(IEntity playerEntity, float timeSlice)
	{
		vector objectiveWorldPosition = m_AffiliatedZone.GetWorldZoneCenter();
		vector projectedScreenPosition = m_wRoot.GetWorkspace().ProjWorldToScreen(objectiveWorldPosition, m_AffiliatedZone.GetWorld());

		vector cameraMatrix[4];
		m_AffiliatedZone.GetWorld().GetCurrentCamera(cameraMatrix);

		float alpha = 1.0;
		bool visible = true;

		// This case should rarely happen as we do not expect spawn protection area
		// to overlay a capture area, but regardless..
		bool isPopupDrawn;
		SCR_PopUpNotification popupNotifications = SCR_PopUpNotification.GetInstance();
		if (popupNotifications && popupNotifications.IsShowing())
			isPopupDrawn = true;

		// Center to screen and stay fixed
		if (playerEntity && m_AffiliatedZone.IsCharacterInside(SCR_ChimeraCharacter.Cast(playerEntity)) && !isPopupDrawn)
		{
			// Update screen space blend
			m_fBlendScreenPosition += (timeSlice * POSITION_BLEND_SPEED);
			visible = true; // Always draw inside
		}
		else
		{
			// Worldprojection
			vector dirToCamera = (objectiveWorldPosition - cameraMatrix[3]).Normalized();
			float dot = vector.Dot(dirToCamera, cameraMatrix[2]);
			if (dot < 0.6666 && m_fBlendScreenPosition <= 0) // Force animation
				visible = false;

			float alphaScale = Math.InverseLerp(0.667, 1.0, dot);
			alphaScale = Math.Clamp(alphaScale, 0.0, 1.0);

			if (m_fBlendScreenPosition <= 0.0)
				alpha = Math.Lerp(0.5, 1.0, alphaScale);
			else
				alpha = 1.0;

			// Blend the point out significantly in optics, because the projection is misaligned
			if (ArmaReforgerScripted.IsScreenPointInPIPSights(projectedScreenPosition, ArmaReforgerScripted.GetCurrentPIPSights()))
				alpha = Math.Min(alpha, 0.65);

			m_fBlendScreenPosition -= (timeSlice * POSITION_BLEND_SPEED);
		}

		// Clamp the blend so we don't overshoot
		m_fBlendScreenPosition = Math.Clamp(m_fBlendScreenPosition, 0.0, 1.0);

		// If not visible, do not draw and that's it
		if (!visible)
		{
			m_wRoot.SetVisible(false);
			return;
		}

		// Otherwise update widget
		m_wDistanceText.SetVisible(true);

		// Distance text
		float distance = vector.Distance(cameraMatrix[3], objectiveWorldPosition);
		distance = Math.Round(distance);
		m_wDistanceText.SetTextFormat("#CAH-Objective_Distance", distance);

		// Update image state
		UpdateImageState(m_wRoot, timeSlice);

		// Opacity
		m_wRoot.SetOpacity(alpha);

		float x, y;
		float scale = 1.0;
		float textScale = 1.0;
		// Interpolate position
		if (m_fBlendScreenPosition > 0.0)
		{
			WorkspaceWidget ww = m_wRoot.GetWorkspace();
			int w = ww.GetWidth();
			int h = ww.GetHeight();
			float fixedX = ww.DPIUnscale(0.5 * w);
			float fixedY = ww.DPIUnscale(0.15 * h);

			SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
			if (editorManager && editorManager.IsOpened())
				fixedY += 40;

			x = Math.Lerp(projectedScreenPosition[0], fixedX, m_fBlendScreenPosition);
			y = Math.Lerp(projectedScreenPosition[1], fixedY, m_fBlendScreenPosition);
			scale = m_fBlendScreenPosition * 2.25;
			textScale = m_fBlendScreenPosition * 1.85;
		}
		else
		{
			x = projectedScreenPosition[0];
			y = projectedScreenPosition[1];
		}

		float xScale = m_fOriginalXSize * scale;
		float yScale = m_fOriginalYSize * scale;
		// Apply
		FrameSlot.SetPos(m_wRoot, x - 0.5 * xScale, y - 0.5 * yScale);

		// Scale
		FrameSlot.SetSize(m_wLayout, xScale, yScale);

		m_wNameText.SetExactFontSize(m_fOriginalTitleSize * textScale);
		m_wDistanceText.SetExactFontSize(m_fOriginalTextSize * textScale);

		// Set objective symbol
		m_wNameText.SetText("KOTH");

		// And make widget visible
		m_wRoot.SetVisible(true);
	}
}

class KOTH_ZoneObjectiveDisplay : SCR_InfoDisplayExtended
{
	[Attribute("{A1EA87A89C5215AC}UI/layouts/HUD/KOTH/KOTHObjectiveLayout.layout", params: "layout")]
	protected ResourceName m_rObjectiveHUDLayout;

	//! Individual element wrappers
	protected ref KOTH_ZoneObjectiveDisplayObject m_aObjectiveElement;

	//! Area manager provides us with necessary API
	protected KOTH_ZoneManager m_ZoneManager;
	//! Game mode instance.
	protected KOTH_GameModeBase m_GameMode;

	//------------------------------------------------------------------------------------------------
	/*!
		Checks whether prerequisites for this InfoDisplay are met.
	*/
	override bool DisplayStartDrawInit(IEntity owner)
	{
		// No ui can be drawn without necessary items
		m_GameMode = KOTH_GameModeBase.Cast(GetGame().GetGameMode());
		if (!m_GameMode)
			return false;

		m_ZoneManager = KOTH_ZoneManager.Cast(m_GameMode.FindComponent(KOTH_ZoneManager));
		if (!m_ZoneManager)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*
		Creates HUD elements for individual zones.
	*/
	override void DisplayStartDraw(IEntity owner)
	{
		// Create objective icons
		KOTH_ZoneTriggerEntity zone = m_ZoneManager.GetZone();
		CreateObjectiveDisplays(zone);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Destroys HUD elements of individual areas.
	*/
	override void DisplayStopDraw(IEntity owner)
	{
		// Clear ALL hud objects
		ClearObjectiveDisplays();
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Updates and manages individual HUD elements.
	*/
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		// area markers
		KOTH_ZoneTriggerEntity zone = m_ZoneManager.GetZone();
		CreateObjectiveDisplays(zone);
		World world = GetGame().GetWorld();
		IEntity playerEntity = GetGame().GetPlayerController().GetControlledEntity();

		// Redraw all elements
		ClearObjectiveDisplays();
		CreateObjectiveDisplays(zone);

		m_aObjectiveElement.UpdateDynamic(playerEntity, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Creates and fills the objective collection using provided areas.
	*/
	protected void CreateObjectiveDisplays(KOTH_ZoneTriggerEntity zone)
	{
		Widget objective = GetRootWidget().GetWorkspace().CreateWidgets(m_rObjectiveHUDLayout, GetRootWidget());
		if (!objective)
			return;

		KOTH_ZoneObjectiveDisplayObject displayObject = new KOTH_ZoneObjectiveDisplayObject(objective, zone, m_ZoneManager);
		m_aObjectiveElement = displayObject;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Removes objective displays from the collection.
		\param emptyOnly If true, empty (not linking to area) objects are removed. If false, all objects are flushed immediately.
	*/
	protected void ClearObjectiveDisplays()
	{
		// Clear previous elements
		Widget w = m_aObjectiveElement.GetRootWidget();
		if (w)
		{
			w.RemoveFromHierarchy();
			delete w;
		}

		delete m_aObjectiveElement;
	}
}
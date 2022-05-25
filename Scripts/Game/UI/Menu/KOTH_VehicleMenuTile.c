class KOTH_VehicleMenuTileBase : SCR_ButtonImageComponent
{
};

//------------------------------------------------------------------------------------------------
class KOTH_VehicleMenuTile : KOTH_VehicleMenuTileBase
{
	[Attribute("VehicleMessage", desc: "Widget name of simple message component")]
	protected string m_sSimpleMessageName;

	protected TextWidget m_wText;
	protected KOTH_VehicleMenuTileSelection m_Parent;
	protected bool m_bClickEnabled = false;
	
	//~ Disabled when vehicles page is loaded
	protected bool m_bForceDisabled;

	protected ImageWidget m_wIcon;
	protected ImageWidget m_wFactionBackground;
	protected KOTH_VehiclePreviewComponent m_Preview;;

	//------------------------------------------------------------------------------------------------
	static KOTH_VehicleMenuTile InitializeTile(KOTH_VehicleMenuTileSelection parent, KOTH_VehicleAssetInfo vehicles)
	{
		Widget tile = GetGame().GetWorkspace().CreateWidgets(parent.GetTileResource());
		SCR_GalleryComponent gallery_handler = SCR_GalleryComponent.Cast(parent.GetTileContainer().GetHandler(0));

		KOTH_VehicleMenuTile handler = KOTH_VehicleMenuTile.Cast(tile.FindHandler(KOTH_VehicleMenuTile));
		if (!handler)
			return null;

		handler.SetParent(parent);
		handler.SetPreviewedVehicle(vehicles);
		handler.SetText(vehicles.GetDisplayName());
		gallery_handler.AddItem(tile);

		Resource res = Resource.Load(vehicles.GetPrefab());
		IEntityComponentSource source = SCR_BaseContainerTools.FindComponentSource(res, "SCR_EditableVehicleComponent");
		BaseContainer container = source.GetObject("m_UIInfo");
		SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(container));

		ResourceName path;
		// get the vehicles img
		container.Get("m_Image", path);
		handler.SetImage(path);

		// get the vehicles icon
		handler.SetIcon(info);

		return handler;
	}

	/*!
	Disable the layout button and show a message over it
	\param messageID ID in simple message component to set message to
	*/
	void DisableAndShowMessage(string messageID)
	{
		m_bForceDisabled = true;
		SetEnabled(false);
		ShowSimpleMessage(true, messageID);
	}

	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);

		//~ Only sets the tile as selected if it was not forced disabled. Else it will set itself as null
		if (m_Parent && !m_bForceDisabled)
			m_Parent.FocusTile(this);
		else if (m_bForceDisabled)
			m_Parent.FocusTile(null);

		return false;
	}

	/*!
	Show (or hide) message using the message ID
	\param show if false the message will be hidden and if true it will be shown
	\param messageId the id of the message that should be shown
	*/
	void ShowSimpleMessage(bool show, string messageId = string.Empty)
	{
		Widget simpleMessageWidget = GetRootWidget().FindAnyWidget(m_sSimpleMessageName);

		if (!simpleMessageWidget)
			return;

		if (!show || messageId.IsEmpty())
		{
			simpleMessageWidget.SetVisible(false);
			return;
		}

		SCR_SimpleMessageComponent simpleMessage = SCR_SimpleMessageComponent.Cast(simpleMessageWidget.FindHandler(SCR_SimpleMessageComponent));
		if (!simpleMessage)
			return;

		simpleMessage.SetContentFromPreset(messageId);
		simpleMessageWidget.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wIcon = ImageWidget.Cast(w.FindAnyWidget("Icon"));
		m_wFactionBackground = ImageWidget.Cast(w.FindAnyWidget("FactionBckg"));
		Widget widget = w.FindAnyWidget("VehiclePreview");
		m_Preview = KOTH_VehiclePreviewComponent.Cast(widget.FindHandler(KOTH_VehiclePreviewComponent));
	}

	//------------------------------------------------------------------------------------------------
	void SetPreviewedVehicle(KOTH_VehicleAssetInfo vehicles)
	{
		if (m_Preview)
		{
			IEntity ent = m_Preview.SetPreviewedVehicle(vehicles);
			if (!ent)
				return;

			/*FactionAffiliationComponent affiliation = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
			if (affiliation)
			{
				Faction faction = affiliation.GetAffiliatedFaction();
				if (faction)
					m_wFactionBackground.SetColor(faction.GetFactionColor());
			}*/
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetIcon(SCR_EditableEntityUIInfo info)
	{
		if (!info)
			return;
		info.SetIconTo(m_wIcon);
		m_wIcon.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParent(KOTH_VehicleMenuTileSelection parent)
	{
		m_Parent = parent;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		if (m_wText)
			m_wText.SetText(text);
	}
};
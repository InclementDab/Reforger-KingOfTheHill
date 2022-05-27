[BaseContainerProps(), SCR_BaseContainerCustomTitleObject("m_Struct", "Struct: %1")]
class KOTH_DSSessionCallback: DSSessionCallback
{
	protected KOTH_PlayerStorage m_Struct;
	
	void KOTH_DSSessionCallback(KOTH_PlayerStorage struct)
	{
		m_Struct = struct;
	}
	
	void SaveSession(string file_name)
	{
		SessionStorage storage = GetGame().GetBackendApi().GetStorage();
		storage.AssignFileIDCallback(file_name, this);
		
		if (Replication.IsRunning()) {
			//--- MP
			storage.RequestSave(file_name);
			PrintFormat("RequestSave: $profile:.backend\\%1.json", file_name);
		} else {
			//--- SP
			storage.LocalSave(file_name);
			PrintFormat("LocalSave: $profile:.backend\\%1.json", file_name);
		}
	}
	
	void LoadSession(string file_name)
	{
		SessionStorage storage = GetGame().GetBackendApi().GetStorage();
		storage.AssignFileIDCallback(file_name, this);
		
		// TODO: Solve for Listen servers
		if (RplSession.Mode() == RplMode.Dedicated) {
			//--- MP
			storage.RequestLoad(file_name);
			PrintFormat("RequestLoad: $profile:.backed\\%1.json", file_name);
		} else if (RplSession.Mode() == RplMode.None) {
			//--- SP
			storage.LocalLoad(file_name);
			PrintFormat("LocalLoad: $profile:.backed\\%1.json", file_name);
		}
	}

	void LogSession()
	{
		m_Struct.Log();
	}
	
	protected bool SaveStruct()
	{
		return m_Struct && m_Struct.Serialize();
	}
	
	protected bool LoadStruct()
	{
		return m_Struct && m_Struct.Deserialize();
	}
	
	override void OnSaving(string fileName)
	{
		if (SaveStruct()) {
			GetGame().GetBackendApi().GetStorage().ProcessSave(m_Struct, fileName);
		}
	}
	
	override void OnLoaded(string fileName)
	{
		GetGame().GetBackendApi().GetStorage().ProcessLoad(m_Struct, fileName);
		LoadStruct();
	}
	
	override void OnConnect()
	{
		Print("OnConnect", LogLevel.DEBUG);
	}
	
	override void OnDisconnect()
	{
		Print("OnDisconnect", LogLevel.DEBUG);
	}
	
	override void OnSetup(string fileName)
	{
		Print("OnSetup", LogLevel.DEBUG);
	}
	
	override void OnLoadFailed(string fileName)
	{
		Print("OnLoadFailed", LogLevel.DEBUG);
	}
	
	override void OnSaveFailed(string fileName)
	{
		Print("OnSaveFailed", LogLevel.DEBUG);
	}
	
	override void OnSaveSuccess(string filename)
	{
		Print("OnSaveSuccess", LogLevel.DEBUG);
	}
	
	override void OnInitialize()
	{
		Print("OnInitialize", LogLevel.DEBUG);
	}
	
	override void OnNew()
	{
		Print("OnNew", LogLevel.DEBUG);
	}
	
	override void OnReady()
	{
		Print("OnReady", LogLevel.DEBUG);
	}
}

enum KOTH_PlayerClasses
{
	MONEY,
	AMMO,
	GRENADE,
	SPRINT,
}


// automatically adds every variable to RegV
class KOTH_AutoJsonApiStruct: SCR_JsonApiStruct
{
	void KOTH_AutoJsonApiStruct()
	{
		// register all variables to be saved
		for (int i = 0; i < Type().GetVariableCount(); i++) {
			RegV(Type().GetVariableName(i));
		}
	}
}

class KOTH_GameStorage: KOTH_AutoJsonApiStruct
{
					// uid
	protected ref map<string, ref KOTH_PlayerStorage> m_PlayerStorage = new map<string, ref KOTH_PlayerStorage>();
}

class KOTH_PlayerStorage: KOTH_AutoJsonApiStruct
{
	string Uid;
	int Currency;
	int Experience;
	KOTH_PlayerClasses Classes; // bitmask of classes
}


[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class KOTH_SaveLoadComponentClass: SCR_BaseGameModeComponentClass
{
}

/*!
Manager for saving and loading game mode data.

ToDo:
* Load when starting the world if it's requested (i.e., 'Continue' save)
* Refactor Conflict save to use this system
*/
class KOTH_SaveLoadComponent: SCR_BaseGameModeComponent
{
	[Attribute(desc: "Struct object which manages saved data. Must be defined, without it saving won't work.")]
	protected ref KOTH_GameStorage m_Struct;
	
	[Attribute(defvalue: "1", desc: "When enabled, save the state when exiting the world.")]
	protected bool m_SaveOnExit;
	
	[Attribute(defvalue: "0", desc: "0 = disabled. 60 seconds is the lowest accepted value otherwise.")]
	protected int m_iAutosavePeriod;
	
	protected ref KOTH_DSSessionCallback m_Callback;
	protected string m_sFileName;
	protected float m_fTimer;
	
	protected static string m_sFileNameToLoad;
	
	static const int MINIMUM_AUTOSAVE_PERIOD = 60;
	
	/*!
	Check if given mission has a save file
	\param mission_header Mission header
	\return True if the mission has a save file
	*/
	static bool HasSaveFile(KOTH_MissionHeader mission_header)
	{
		return GetGame().GetBackendApi().GetStorage().CheckFileID(mission_header.GetSaveFileName());
	}
	
	/*!
	Mark the mission to be loaded from a save file upon start.
	\param mission_header Header of the misison that should be loaded on start. Use null to reset the value.
	*/
	static void LoadOnStart(KOTH_MissionHeader mission_header = null)
	{
		if (mission_header) {
			m_sFileNameToLoad = mission_header.GetSaveFileName();
		} else {
			m_sFileNameToLoad = string.Empty;
		}
	}
	
	/*!
	Check if the mission should be loaded from a save file upon start.
	\return mission_header Queried mission
	*/
	static bool IsLoadOnStart(KOTH_MissionHeader mission_header)
	{
		// TODO: Solve for Listen servers
		if (RplSession.Mode() == RplMode.Dedicated) {
			return !System.IsCLIParam("backendFreshSession");
		} else { 
			return mission_header && mission_header.GetSaveFileName() == m_sFileNameToLoad;
		}
	}

	/*!
	\return Local instance of the possession manager
	*/
	static SCR_SaveLoadComponent GetInstance()
	{
		BaseGameMode game_mode = GetGame().GetGameMode();
		if (!game_mode) {
			return null;
		}
		
		return SCR_SaveLoadComponent.Cast(game_mode.FindComponent(SCR_SaveLoadComponent));
	}
	
	/*!
	Save all configured structs.
	*/
	void Save()
	{
		if (m_Callback != null) {
			m_Callback.SaveSession(m_sFileName);
		}
	}
	/*!
	Restart the world and load saved state afterwards.
	*/
	void RestartAndLoad()
	{
		KOTH_MissionHeader mission_header = KOTH_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (mission_header) {
			LoadOnStart(mission_header);
			GetGame().PlayMission(mission_header);
		} else {
			Print("Cannot load mission, no MissionHeader defined. Happens for example when playing the world from World Editor.", LogLevel.WARNING);
		}
	}
	
	/*!
	\return True if the world should be saved on exit.
	*/
	bool CanSaveOnExit()
	{
		return m_SaveOnExit;
	}
	
	/*!
	Log the most recently saved structs.
	*/
	void Log()
	{
		if (m_Callback != null) {
			m_Callback.LogSession();
		}
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (GetGame().IsDev() && !System.IsConsoleApp())
		{
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_SAVE))
			{
				Save();
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVELOAD_SAVE, false);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOAD))
			{
				RestartAndLoad();
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOAD, false);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOG))
			{
				Log();
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOG, false);
			}
		}
		
		// Autosave
		if (m_iAutosavePeriod > 0)
		{
			m_fTimer += timeSlice;
			
			if (m_fTimer >= m_iAutosavePeriod)
			{
				m_fTimer = 0;
				Save();
			}
		}
	}
	
	/* //--- Disabled, not reliable. Some variables are already cleared at this moment.
	override void OnGameEnd()
	{
		if (m_SaveOnExit)
			Save();
	}
	*/
	override void OnWorldPostProcess(World world)
	{
		if (!Replication.IsServer()) {
			return;
		} 
		
		SCR_MissionHeader mission_header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());		
		if (mission_header)
		{
			m_sFileName = mission_header.GetSaveFileName();
		
			//--- Saving is disabled, terminate
			if (!mission_header.IsSavingEnabled()) {
				return;
			}
		}
#ifdef WORKBENCH
		else {
			m_sFileName = "WB_" + FilePath.StripPath(FilePath.StripExtension(GetGame().GetWorldFile()));
		}
#endif
		
		if (m_sFileName) {			
			m_Callback = new KOTH_DSSessionCallback(m_Struct);
			if (IsLoadOnStart(mission_header)) {
				m_Callback.LoadSession(m_sFileName);
			}
		}
		
	}
	override void OnPostInit(IEntity owner)
	{
		if (Replication.IsServer()) {
			owner.SetFlags(EntityFlags.ACTIVE, false);
			SetEventMask(owner, EntityEvent.FRAME);
			
			if (m_iAutosavePeriod > 0 && m_iAutosavePeriod < MINIMUM_AUTOSAVE_PERIOD) {
				Print("SCR_SaveLoadComponent: Autosave period set too low (" + m_iAutosavePeriod + "), setting to " + MINIMUM_AUTOSAVE_PERIOD, LogLevel.WARNING);
				m_iAutosavePeriod = MINIMUM_AUTOSAVE_PERIOD;
			}
		}
		
		if (GetGame().IsDev() && Replication.IsServer() && !System.IsConsoleApp())
		{
			DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_SAVELOAD, "Save/Load", "Game");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_SAVE, "", "Save Session", "Save/Load");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOAD, "", "Restart and Load Session", "Save/Load");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVELOAD_LOG, "", "Log Session Save", "Save/Load");
		}
	}
};
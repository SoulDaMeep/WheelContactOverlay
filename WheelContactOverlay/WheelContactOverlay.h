#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

#include "Settings.h"
#include <fstream>

class WheelContactOverlay: public BakkesMod::Plugin::BakkesModPlugin
	,public SettingsWindowBase // Uncomment if you wanna render your own tab in the settings menu
	,public PluginWindowBase // Uncomment if you want to render your own plugin window
{

	void onLoad() override;

	bool IsValidEnv();

	void RenderWheelNumericData(CanvasWrapper canvas, Vector2F& Offset, WheelContactData wheel, int i);

	void NumericDataAtPos(CanvasWrapper canvas, Vector2F& Offset, WheelContactData wheel, int i);

	void NumericDataAtWheels(CanvasWrapper canvas, WheelContactData wheel);

	void GetWheelData(CarWrapper car);


public:

	struct SetGamePausedParams {
		bool bPaused;
	};

public:
	// std::vector is slow
	//std::vector<WheelContactData> Wheels;
	std::array<WheelContactData, 4> Wheels;
	
	bool CanShowWindow = true;

public:
	void LoadSettings();
	void WriteSettings();
	void DefaultSettings();

	std::filesystem::path saveFile = gameWrapper->GetDataFolder() / "WheelContactOverlay" / "settings.json";
	Settings settings;

public:
	void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	void RenderWindow() override; // Uncomment if you want to render your own plugin window
};

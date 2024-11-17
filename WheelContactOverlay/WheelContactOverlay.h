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
	,public SettingsWindowBase
	,public PluginWindowBase
{

	void onLoad() override;

public:
	std::array<WheelContactData, 4> Wheels;
	bool CanShowWindow = true;

public:
	bool IsValidEnv();
	void RenderWheelNumericData(CanvasWrapper canvas, Vector2F& Offset, WheelContactData wheel, int i);
	void NumericDataAtPos(CanvasWrapper canvas, Vector2F& Offset, WheelContactData wheel, int i);
	void NumericDataAtWheels(CanvasWrapper canvas, WheelContactData wheel);
	void GetWheelData(CarWrapper car);
public:
	void LoadSettings();
	void WriteSettings();
	void DefaultSettings();

	std::filesystem::path saveFile = gameWrapper->GetDataFolder() / "WheelContactOverlay" / "settings.json";
	Settings settings;

public:
	void RenderSettings() override;
	void RenderWindow() override;
};

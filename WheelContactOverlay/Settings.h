#pragma once
#include "pch.h"
#include <nlohmann/json.hpp>


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LinearColor, R, G, B, A)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ImVec4, x, y, z, w)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector2F, X, Y)
struct NumericWheelData {

	LinearColor Color;

	bool Enabled;
	bool StaticPosition;
	bool Wheels;

	Vector2F StaticPos;
	float Scale;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(NumericWheelData, Color, Enabled, StaticPosition, Wheels, StaticPos, Scale)
};

struct PhysicalWheelOverlay {

	LinearColor VectorCol;

	bool Enabled;

	bool ShowNormal;
	bool ShowGrid;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(PhysicalWheelOverlay, VectorCol, Enabled, ShowNormal, ShowGrid)
};

struct WheelOverlay {
	ImVec4 OutlineCol;
	ImVec4 NoContactCol;
	ImVec4 HasContactCol;

	bool Enabled;
	bool ShowWindow;
	bool Transparent;

	float WheelBorderThickness;
	float WheelRoundness;
	float Scale;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(WheelOverlay, OutlineCol, NoContactCol, HasContactCol, Enabled, ShowWindow, Transparent, WheelBorderThickness, WheelRoundness, Scale)
};
struct Settings {
	bool Enabled;

	WheelOverlay WheelOverlay;
	PhysicalWheelOverlay PhysicalWheelOverlay;
	NumericWheelData NumericWheelData;


	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Settings, Enabled, WheelOverlay, PhysicalWheelOverlay, NumericWheelData);

};
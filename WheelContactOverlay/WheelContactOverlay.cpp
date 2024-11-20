#include "pch.h"
// CinderBlocc RenderingTools
#define LINMATH_H
#include "WheelContactOverlay.h"
#include "RenderingTools/Objects/Grid.h"
#include "RenderingTools/Objects/Matrix3.h"
#include "RenderingTools/Objects/Frustum.h"
#include "RenderingTools/Objects/Line.h"
#include "RenderingTools/Objects/Sphere.h"
#include "RenderingTools/Extra/WrapperStructsExtensions.h"
#include "RenderingTools/Extra/RenderingMath.h"
using namespace RT;

BAKKESMOD_PLUGIN(WheelContactOverlay, "WheelContactOverlay: Displays the contact of each wheel on the local car.", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
void WheelContactOverlay::onUnload() {
	if (settings.WheelOverlay.ShowWindow) {
		cvarManager->executeCommand("togglemenu WheelcontactOverlay");
	}
}
void WheelContactOverlay::onLoad()
{
	_globalCvarManager = cvarManager;

	if (!std::filesystem::exists(saveFile)) {
		std::filesystem::create_directories(saveFile.parent_path());
		DefaultSettings();
		LOG("Added Default Settings");
	}
	
	LoadSettings();

	gameWrapper->SetTimeout([this](GameWrapper* gw) {
		if (settings.WheelOverlay.ShowWindow) {
			// Bakkesmod doesnt load the gui before the main class (i think)(could be clickbait idk)
			cvarManager->executeCommand("togglemenu WheelContactOverlay");
			LOG("Loaded");
		}
	}, 1.0f);

	gameWrapper->HookEvent("Function Engine.Interaction.Tick", [this](std::string eventName) {
		if (!settings.Enabled) return;

		CanShowWindow = true;

		if (IsValidEnv() == false || gameWrapper->IsPaused()) {
			CanShowWindow = false;
			return;
		}

		if (gameWrapper->IsInFreeplay() || gameWrapper->IsInCustomTraining()) {
			// Dont want to get the caller CarWrapper because I want to keep it to the local car.
			GetWheelData(gameWrapper->GetLocalCar());
		}

		else if (gameWrapper->IsInReplay()) {
			ServerWrapper server = gameWrapper->GetGameEventAsReplay();
			if (!server) return;

			CameraWrapper camera = gameWrapper->GetCamera();
			if (!camera) return;

			ViewTarget target = camera.GetViewTarget();

			for (CarWrapper car : server.GetCars()) {
				// if the memory address matches the target that the camera is spectating
				if (reinterpret_cast<void*>(car.memory_address) == target.Target) {
					GetWheelData(car);
				}
			}
		}
	});

	gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) {
		if (!settings.Enabled) return;
		if (!CanShowWindow) return;
		if (IsValidEnv() == false) return;
		
		CameraWrapper camera = gameWrapper->GetCamera();
		Frustum frust{ canvas, camera };
		Vector2F Offset = settings.NumericWheelData.StaticPos;

		for (size_t i = 0; i < Wheels.size(); i++) {
			if (Wheels[i].bHasContact) {
				if (settings.NumericWheelData.Enabled) {

					canvas.SetColor(settings.NumericWheelData.Color * 255);

					RenderWheelNumericData(canvas, Offset, Wheels[i], i);
				}
				
				canvas.SetColor(settings.PhysicalWheelOverlay.VectorCol * 255);
		
				if (settings.PhysicalWheelOverlay.ShowGrid) {
					// Thank you CinderBlocc
					RT::Matrix3 mat3 = RT::Matrix3(Wheels[i].Normal, Wheels[i].LongDirection, Wheels[i].LatDirection);
					Quat rot = mat3.ToQuat();
					RT::Grid(Wheels[i].Location, rot, 35, 20, 5, 3).Draw(canvas, frust);
				} 

				if (settings.PhysicalWheelOverlay.ShowNormal) {
					RT::Line(Wheels[i].Location, Wheels[i].Location + (Wheels[i].Normal * 25), 2.0f).Draw(canvas);
				}
			}
		}
	});
}

////////////////// HELPER /////////////////////////////////

bool WheelContactOverlay::IsValidEnv() {
	bool isValidEnv = gameWrapper->IsInFreeplay()
		|| gameWrapper->IsInReplay()
		|| gameWrapper->IsInCustomTraining();

	return isValidEnv;
}

/////////////////////////////// NUMERIC WHEEL RENDERING /////////////////////////////////////////

void WheelContactOverlay::RenderWheelNumericData(CanvasWrapper canvas, Vector2F& Offset, WheelContactData wheel, int i) {
	if (settings.NumericWheelData.StaticPosition) {
		NumericDataAtPos(canvas, Offset, wheel, i);
	}

	if (settings.NumericWheelData.Wheels) {
		NumericDataAtWheels(canvas, wheel);
	}
}

void WheelContactOverlay::NumericDataAtPos(CanvasWrapper canvas, Vector2F& Offset, WheelContactData wheel, int i) {

	float scale = settings.NumericWheelData.Scale;

	canvas.SetPosition(Offset);
	canvas.DrawString(std::format("Wheel {}", i + 1), scale, scale);
	Offset += Vector2{ 0, int(12 * scale) };

	canvas.SetPosition(Offset);
	canvas.DrawString(std::format("X: {:.3f}", wheel.Location.X), scale, scale);
	Offset += Vector2{ 0, int(12 * scale) };

	canvas.SetPosition(Offset);
	canvas.DrawString(std::format("Y: {:.3f}", wheel.Location.Y), scale, scale);
	Offset += Vector2{ 0, int(12 * scale) };

	canvas.SetPosition(Offset);
	canvas.DrawString(std::format("Z: {:.3f}", wheel.Location.Z), scale, scale);
	Offset += Vector2{ 0, int(12 * scale) };

}

void WheelContactOverlay::NumericDataAtWheels(CanvasWrapper canvas, WheelContactData wheel) {
	Vector2F p = canvas.ProjectF(wheel.Location);

	canvas.SetPosition(Vector2F{ p.X, p.Y - (-2 * 10) });
	canvas.DrawString(std::format("Normal: ({:.3f} {:.3f} {:.3f})", wheel.Normal.X, wheel.Normal.Y, wheel.Normal.Z));
	canvas.SetPosition(Vector2F{ p.X, p.Y - (1 * 10) });
	canvas.DrawString(std::format("X: {:.3f}", wheel.Location.X), 1, 1);
	canvas.SetPosition(Vector2F{ p.X, p.Y - (0 * 10) });
	canvas.DrawString(std::format("Y: {:.3f}", wheel.Location.Y), 1, 1);
	canvas.SetPosition(Vector2F{ p.X, p.Y - (-1 * 10) });
	canvas.DrawString(std::format("Z: {:.3f}", wheel.Location.Z), 1, 1);
}
/////////////////////////////////////////////////////////////////////////////////////////////

void WheelContactOverlay::GetWheelData(CarWrapper car) {

	if (!car) return;

	VehicleSimWrapper vehiclesim = car.GetVehicleSim();
	if (!vehiclesim) return;

	ArrayWrapper<WheelWrapper> wheels = vehiclesim.GetWheels();

	for (size_t i = 0; i < wheels.Count(); i++) {
		Wheels[i] = wheels.Get(i).GetContact();
	}
}

///////////////////////// Settings Editing with Nlohmann /////////////////////////////////////
void WheelContactOverlay::DefaultSettings() {
	auto file = std::ofstream(saveFile);

	settings.Enabled = true;

	settings.WheelOverlay.Enabled = true;
	settings.WheelOverlay.Scale = 1.0f;
	settings.WheelOverlay.ShowWindow = true;
	settings.WheelOverlay.Transparent = false;
	settings.WheelOverlay.HasContactCol = ImVec4{ 0, 1, 0, 1 };
	settings.WheelOverlay.NoContactCol = ImVec4{ 1, 0, 0, 1 };
	settings.WheelOverlay.OutlineCol = ImVec4{ 0, 0, 0, 1 };
	settings.WheelOverlay.WheelBorderThickness = 9.0f;
	settings.WheelOverlay.WheelRoundness = 5.0f;

	settings.PhysicalWheelOverlay.ShowNormal = false;
	settings.PhysicalWheelOverlay.ShowGrid = false;
	settings.PhysicalWheelOverlay.VectorCol = LinearColor{ 0, 1, 0 , 1 };

	settings.NumericWheelData.StaticPosition = true;
	settings.NumericWheelData.Wheels = false;
	settings.NumericWheelData.Enabled = false;
	settings.NumericWheelData.Color = LinearColor{ 1, 1, 1, 1 };
	settings.NumericWheelData.StaticPos = Vector2F{ 0, 0 };
	settings.NumericWheelData.Scale = 1.0f;

	if (file.is_open()) {
		WriteSettings();
	}
	file.close();
}

void WheelContactOverlay::LoadSettings() {
	auto file = std::ifstream(saveFile);
	nlohmann::json json;
	if (file.is_open()) {
		file >> json;
	}
	file.close();
	settings = json.get<Settings>();
}

void WheelContactOverlay::WriteSettings() {
	auto file = std::ofstream(saveFile);
	nlohmann::json json = settings;
	if (file.is_open()) {
		file << json.dump(4);
	}
	file.close();
}
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
			// bakkesmod doesnt load the gui before the main class (i think)(could be clickbait idk)

			cvarManager->executeCommand("togglemenu WheelContactOverlay");
			LOG("Loaded");
		}
	}, 1.0f);

	gameWrapper->HookEvent("Function Engine.Interaction.Tick", [this](std::string eventName) {
		if (!settings.Enabled) return;

		CanShowWindow = !gameWrapper->IsPaused();
		// if its not allowed to show, clear the list.
		if (!CanShowWindow) {
			Wheels.fill(WheelContactData{});
		}

		// set conditions to not show overlay
		if (!gameWrapper->IsInFreeplay() && !gameWrapper->IsInReplay() && !gameWrapper->IsInCustomTraining()) {
			if (gameWrapper->GetCurrentMap() == "menu_main_p") CanShowWindow = false;
			else if (gameWrapper->IsInOnlineGame()) CanShowWindow = false;
			return;
		}
		
		if (gameWrapper->IsInFreeplay() || gameWrapper->IsInCustomTraining()) {

			// Dont want to get the caller CarWrapper because I want to keep it to the local car.
			CarWrapper car = gameWrapper->GetLocalCar();
			if (!car) return;

			VehicleSimWrapper vehiclesim = car.GetVehicleSim();
			if (!vehiclesim) return;

			ArrayWrapper<WheelWrapper> wheels = vehiclesim.GetWheels();

			for (size_t i = 0; i < wheels.Count(); i++) {
				Wheels[i] = wheels.Get(i).GetContact();
			}
		}
		else if (gameWrapper->IsInReplay()) {
			ServerWrapper server = gameWrapper->GetGameEventAsReplay();
			if (!server) return;

			CameraWrapper camera = gameWrapper->GetCamera();
			if (!camera) return;
			ViewTarget target = camera.GetViewTarget();


			for (CarWrapper car : server.GetCars()) {
				if (reinterpret_cast<void*>(car.memory_address) == target.Target) {

					VehicleSimWrapper vehiclesim = car.GetVehicleSim();
					if (!vehiclesim) return;

					ArrayWrapper<WheelWrapper> wheels = vehiclesim.GetWheels();

					for (size_t i = 0; i < wheels.Count(); i++) {
						Wheels[i] = wheels.Get(i).GetContact();
					}
				}
			}
		}
	});

	gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) {
		if (!settings.Enabled) return;
		if (!gameWrapper->IsInFreeplay() && !gameWrapper->IsInReplay() && !gameWrapper->IsInCustomTraining()) return;
		if (!CanShowWindow) return;
		CameraWrapper camera = gameWrapper->GetCamera();
		Frustum frust{ canvas, camera };

		for (size_t i = 0; i < Wheels.size(); i++) {



			if (Wheels[i].bHasContact) {


				if (settings.NumericWheelData.Enabled) {
					canvas.SetColor
					(LinearColor
						{
							settings.NumericWheelData.Color.R * 255,
							settings.NumericWheelData.Color.G * 255,
							settings.NumericWheelData.Color.B * 255,
							settings.NumericWheelData.Color.A * 255
						}
					);

					if (settings.NumericWheelData.StaticPosition) {
						Vector2F Offset = settings.NumericWheelData.StaticPos;
						float scale = settings.NumericWheelData.Scale;
						for (int i = 0; i < Wheels.size(); i++) {
							if (Wheels[i].bHasContact) {
								canvas.SetPosition(Offset);
								canvas.DrawString(std::format("Wheel {}", i+1), scale, scale);
								Offset += Vector2{ 0, int(12 * scale) };
								canvas.SetPosition(Offset);
								canvas.DrawString(std::format("X: {:.3f}", Wheels[i].Location.X), scale, scale);
								Offset += Vector2{ 0, int(12 * scale)};

								canvas.SetPosition(Offset);
								canvas.DrawString(std::format("Y: {:.3f}", Wheels[i].Location.Y), scale, scale);
								Offset += Vector2{ 0, int(12 * scale)};

								canvas.SetPosition(Offset);
								canvas.DrawString(std::format("Z: {:.3f}", Wheels[i].Location.Z), scale, scale);
								Offset += Vector2{ 0, int(12 * scale)};
							}
							
						}

					}

					if (settings.NumericWheelData.Wheels) {

						Vector2F p = canvas.ProjectF(Wheels[i].Location);
				

						canvas.SetPosition(Vector2F{ p.X, p.Y - (-2 * 10) });
						canvas.DrawString(std::format("Normal: ({:.3f} {:.3f} {:.3f})", Wheels[i].Normal.X, Wheels[i].Normal.Y, Wheels[i].Normal.Z));
						canvas.SetPosition(Vector2F{ p.X, p.Y - (1 * 10) });
						canvas.DrawString(std::format("X: {:.3f}", Wheels[i].Location.X), 1, 1);
						canvas.SetPosition(Vector2F{ p.X, p.Y - (0 * 10) });
						canvas.DrawString(std::format("Y: {:.3f}", Wheels[i].Location.Y), 1, 1);
						canvas.SetPosition(Vector2F{ p.X, p.Y - (-1 * 10) });
						canvas.DrawString(std::format("Z: {:.3f}", Wheels[i].Location.Z), 1,1);
					}
				}
				
				canvas.SetColor
				(LinearColor
					{
						settings.PhysicalWheelOverlay.VectorCol.R * 255,
						settings.PhysicalWheelOverlay.VectorCol.G * 255,
						settings.PhysicalWheelOverlay.VectorCol.B * 255,
						settings.PhysicalWheelOverlay.VectorCol.A * 255
					}
				);

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
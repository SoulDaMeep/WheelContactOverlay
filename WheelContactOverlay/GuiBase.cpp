#include "pch.h"
#include "GuiBase.h"
#include "WheelContactOverlay.h"

std::string SettingsWindowBase::GetPluginName()
{
	return "WheelContactOverlay";
}

void SettingsWindowBase::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

std::string PluginWindowBase::GetMenuName()
{
	return "WheelContactOverlay";
}

std::string PluginWindowBase::GetMenuTitle()
{
	return menuTitle_;
}

void PluginWindowBase::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

bool PluginWindowBase::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

bool PluginWindowBase::IsActiveOverlay()
{
	return false;
}

void PluginWindowBase::OnOpen()
{
	isWindowOpen_ = true;
}

void PluginWindowBase::OnClose()
{
	isWindowOpen_ = false;
}

void WheelContactOverlay::RenderWindow() {
	if (!CanShowWindow) return;
	if (!settings.WheelOverlay.Enabled) return;
	// window size scale
	ImGui::SetNextWindowSize(ImVec2{ 200 * settings.WheelOverlay.Scale, 300 * settings.WheelOverlay.Scale });
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration;
	if (settings.WheelOverlay.Transparent) flags |= ImGuiWindowFlags_NoBackground;

	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, flags))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 WP = ImGui::GetWindowPos();

    // Apply scale to margin and wheel size
    ImVec2 Margin = { 25 * settings.WheelOverlay.Scale, 25 * settings.WheelOverlay.Scale };
    ImVec2 WheelSize = { 50 * settings.WheelOverlay.Scale, 100 * settings.WheelOverlay.Scale };

    ImVec2 pos1 = { WP.x + Margin.x, WP.y + Margin.y };
    ImVec2 pos2 = ImVec2(pos1.x + WheelSize.x, pos1.y + WheelSize.y);

    ImVec2 Points[4] = {
        {WP.x + Margin.x, WP.y + Margin.y},
        {WP.x + (Margin.x * 3 + WheelSize.x), WP.y + Margin.y},
        {WP.x + Margin.x, WP.y + (Margin.y * 3 + WheelSize.y)},
        {WP.x + (Margin.x * 3 + WheelSize.x), WP.y + (Margin.y * 3 + WheelSize.y)}
    };

    for (int i = 0; i < 4; i++) {
        ImVec2 Point2 = ImVec2{ Points[i].x + WheelSize.x, Points[i].y + WheelSize.y };
		
        dl->AddRect(Points[i], Point2, ImGui::GetColorU32(settings.WheelOverlay.OutlineCol), settings.WheelOverlay.WheelRoundness, ImDrawCornerFlags_All, settings.WheelOverlay.WheelBorderThickness);
		dl->AddRectFilled(Points[i], Point2, ImGui::GetColorU32(ImVec4{0, 0, 0, 255}), settings.WheelOverlay.WheelRoundness, ImDrawCornerFlags_All);
        if (Wheels[i].bHasContact) 
            dl->AddRectFilled(Points[i], Point2, ImGui::GetColorU32(settings.WheelOverlay.HasContactCol), settings.WheelOverlay.WheelRoundness, ImDrawCornerFlags_All);
		else 
			dl->AddRectFilled(Points[i], Point2, ImGui::GetColorU32(settings.WheelOverlay.NoContactCol), settings.WheelOverlay.WheelRoundness, ImDrawCornerFlags_All);
    }

	ImGui::End();
}
void WheelContactOverlay::RenderSettings() {
	if (ImGui::Checkbox("Enable", &settings.Enabled)) WriteSettings();
	ImGui::Separator();
	ImGui::Text("Window Overlay");
	if (ImGui::Checkbox("Show Window", &settings.WheelOverlay.ShowWindow)) {
		gameWrapper->Execute([this](GameWrapper* gw) {
			cvarManager->executeCommand("togglemenu " + GetMenuTitle());
			});
		WriteSettings();
	}


	if (settings.WheelOverlay.ShowWindow) {
		ImGui::Indent(18);

		float OutLineColor[4] = { 
			settings.WheelOverlay.OutlineCol.x, 
			settings.WheelOverlay.OutlineCol.y, 
			settings.WheelOverlay.OutlineCol.z, 
			settings.WheelOverlay.OutlineCol.w 
		};
		float HasContactColor[4] = { 
			settings.WheelOverlay.HasContactCol.x, 
			settings.WheelOverlay.HasContactCol.y,
			settings.WheelOverlay.HasContactCol.z,
			settings.WheelOverlay.HasContactCol.w
		};
		float NoContactColor[4] = { 
			settings.WheelOverlay.NoContactCol.x, 
			settings.WheelOverlay.NoContactCol.y, 
			settings.WheelOverlay.NoContactCol.z, 
			settings.WheelOverlay.NoContactCol.w 
		};

		if(ImGui::Checkbox("Transparent?", &settings.WheelOverlay.Transparent))     WriteSettings();
		if(ImGui::SliderFloat("Scale", &settings.WheelOverlay.Scale, 0.1f, 2.0f)) WriteSettings();
		if (ImGui::SliderFloat("Wheel Outline Thickness", &settings.WheelOverlay.WheelBorderThickness, 0.0f, 25.0f)) WriteSettings();
		if (ImGui::SliderFloat("Wheel Roundness", &settings.WheelOverlay.WheelRoundness, 0.0f, 10.0f)) WriteSettings();
		if(ImGui::ColorEdit4("Wheel Outline Color", OutLineColor)) {
			settings.WheelOverlay.OutlineCol.x = OutLineColor[0];
			settings.WheelOverlay.OutlineCol.y = OutLineColor[1];
			settings.WheelOverlay.OutlineCol.z = OutLineColor[2];
			settings.WheelOverlay.OutlineCol.w = OutLineColor[3];
			WriteSettings();
		}
		if(ImGui::ColorEdit4("Wheel Has Contact Color", HasContactColor)) {
			settings.WheelOverlay.HasContactCol.x = HasContactColor[0];
			settings.WheelOverlay.HasContactCol.y = HasContactColor[1];
			settings.WheelOverlay.HasContactCol.z = HasContactColor[2];
			settings.WheelOverlay.HasContactCol.w = HasContactColor[3];
			WriteSettings();
		}
		if (ImGui::ColorEdit4("Wheel No Contact Color", NoContactColor)) {
			settings.WheelOverlay.NoContactCol.x = NoContactColor[0];
			settings.WheelOverlay.NoContactCol.y = NoContactColor[1];
			settings.WheelOverlay.NoContactCol.z = NoContactColor[2];
			settings.WheelOverlay.NoContactCol.w = NoContactColor[3];
			WriteSettings();
		}
		ImGui::Indent(-18);
	}

	float ContactCol[4] = {
		settings.PhysicalWheelOverlay.VectorCol.R,
		settings.PhysicalWheelOverlay.VectorCol.G, 
		settings.PhysicalWheelOverlay.VectorCol.B , 
		settings.PhysicalWheelOverlay.VectorCol.A 
	};
	ImGui::Separator();
	ImGui::Text("Show Location|Normal of the Wheels");
	if (ImGui::Checkbox("Show Numeric Data of Wheels", &settings.NumericWheelData.Enabled)) WriteSettings();
	if (settings.NumericWheelData.Enabled) {
		ImGui::Indent(18);
		if (ImGui::RadioButton("Show at Wheels", settings.NumericWheelData.Wheels)) {
			settings.NumericWheelData.Wheels = true;
			settings.NumericWheelData.StaticPosition = false;
			WriteSettings();
		}
		if (ImGui::RadioButton("Show at Static Position", settings.NumericWheelData.StaticPosition)) {
			settings.NumericWheelData.Wheels = false;
			settings.NumericWheelData.StaticPosition = true;
			WriteSettings();
		}

		if (settings.NumericWheelData.StaticPosition) {
			if (ImGui::SliderFloat("StaticPos X", &settings.NumericWheelData.StaticPos.X, 0.0f, 2160.0f)) {
				WriteSettings();
			}
			if (ImGui::SliderFloat("StaticPos Y", &settings.NumericWheelData.StaticPos.Y, 0.0f, 1160.0f)) {
				WriteSettings();
			}
			if (ImGui::SliderFloat("StaticPos Scale", &settings.NumericWheelData.Scale, 0.1f, 10.0f)) {
				WriteSettings();
			}
		}



		ImGui::Indent(-18);
	}
	ImGui::Separator();
	ImGui::Text("Show Physical Overlay at Wheel");

	if (ImGui::Checkbox("Show Physical Wheel Contact Normal", &settings.PhysicalWheelOverlay.ShowNormal)) WriteSettings();
	if (ImGui::Checkbox("Show Physical Wheel Contact Normal Mesh", &settings.PhysicalWheelOverlay.ShowGrid)) WriteSettings();
	if (ImGui::ColorEdit4("Physical Wheel Contact Color", ContactCol)) {
		settings.PhysicalWheelOverlay.VectorCol.R = ContactCol[0];
		settings.PhysicalWheelOverlay.VectorCol.G = ContactCol[1];
		settings.PhysicalWheelOverlay.VectorCol.B = ContactCol[2];
		settings.PhysicalWheelOverlay.VectorCol.A = ContactCol[3];
		WriteSettings();
	}


}

void PluginWindowBase::Render()
{
	RenderWindow();

	if (!isWindowOpen_)
	{
		_globalCvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

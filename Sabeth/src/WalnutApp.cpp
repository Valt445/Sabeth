#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "Renderer.h"
#include "camera.h"
#include "glm/gtc/type_ptr.hpp"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:

	ExampleLayer()
		: m_camera(45.0f, 0.1f, 100.0f) 
	{
		Material& pinkSphere = m_scene.Materials.emplace_back();
		pinkSphere.Albedo = { 1.0f, 0.2f, 0.7f };
		pinkSphere.Roughness = 0.0f;

		Material& blueSphere = m_scene.Materials.emplace_back();
		blueSphere.Albedo = { 0.2f, 0.2f, 1.0f };
		blueSphere.Roughness = 0.1f;
		{
			Sphere sphere;
			sphere.Position = { 0.0f, 0.0f, 0.0f };
			sphere.Radius = 1.0f;
			sphere.materialIndex = 0; // pinkSphere
			m_scene.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = { 0.0f, -101.0f, 0.0f };
			sphere.Radius = 100.0f;
			sphere.materialIndex = 1; // blueSphere
			m_scene.Spheres.push_back(sphere);

		}
	}
	

	virtual void OnUpdate(float ts) override
	{
		if (m_camera.OnUpdate(ts))
		{
			m_Renderer.ResetFrameIndex();
		}
	}
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render time: %.3fms", m_LastRenderTime);

		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);

		if (ImGui::Button("Reset Renderer"))
		{
			m_Renderer.ResetFrameIndex();
		}
		ImGui::End();
		
		ImGui::Begin("Scene");
		for (size_t i = 0; i < m_scene.Spheres.size(); i++)
		{
			ImGui::PushID((int)i);
			ImGui::Text("Sphere %d", (int)i);
			ImGui::DragFloat3("Position", glm::value_ptr(m_scene.Spheres[i].Position), 0.1f);
			ImGui::DragFloat("Radius", &m_scene.Spheres[i].Radius, 0.1f);
			ImGui::DragInt("Material Index", &m_scene.Spheres[i].materialIndex, 1.0f, 0, (int)m_scene.Materials.size() - 1);

			ImGui::PopID();
		}
		for (size_t i = 0; i < m_scene.Materials.size(); i++)
		{
			ImGui::PushID((int)i);
			ImGui::Text("Material %d", (int)i);
			Material& mat = m_scene.Materials[i];
			ImGui::ColorEdit3("Color", glm::value_ptr(mat.Albedo));
			ImGui::DragFloat("Roughness", &mat.Roughness, 0.05f, 0.0f, 1.0f);
			ImGui::DragFloat("Metallic", &mat.Metallic, 0.05f, 0.0f, 1.0f);
			ImGui::Separator();
			ImGui::PopID();
		}
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		ImGui::Begin("Viewport");

		m_viewportWidth = ImGui::GetContentRegionAvail().x;
		m_viewportHeight = ImGui::GetContentRegionAvail().y;
		auto image = m_Renderer.GetFinalImage();
		if (image)
		{
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() }, ImVec2(0,1), ImVec2(1,0));
		}
		ImGui::End();
		ImGui::PopStyleVar();

		Render(sphereColor, light_dir);
	}

	void Render(glm::vec3 sphereColor, glm::vec3 lightDir)
	{
		Timer timer;

		m_Renderer.Resize(m_viewportWidth, m_viewportHeight);
		m_camera.OnResize(m_viewportWidth, m_viewportHeight);
		m_Renderer.render(sphereColor, light_dir, m_camera, m_scene);

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer m_Renderer;
	glm::vec3 sphereColor = glm::vec3(1.0f, 1.0f, 1.0f);
	Camera m_camera;
	Scene m_scene;
	glm::vec3 light_dir = glm::vec3(-1.0f, -1.0f, -1.0f);
	uint32_t m_viewportWidth = 0, m_viewportHeight = 0;
	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Sabeth";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}
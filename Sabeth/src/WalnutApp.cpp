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
		{
			Sphere sphere;
			sphere.Position = { 0.0f, 0.0f, -2.0f };
			sphere.Radius = 0.5f;
			sphere.Albedo = sphereColor;
			m_scene.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = { 1.0f, 0.0f, -5.0f };
			sphere.Radius = 1.5f;
			sphere.Albedo = {0.2f, 0.2f, 1.0f};
			m_scene.Spheres.push_back(sphere);

		}
	}
	

	virtual void OnUpdate(float ts) override
	{
		m_camera.OnUpdate(ts);
	}
	virtual void OnUIRender() override
	{
		ImGui::Begin("Hello");
		ImGui::Text("Last render time: %.3fms", m_LastRenderTime);
		ImGui::End();
		
		ImGui::Begin("Scene");
		for (size_t i = 0; i < m_scene.Spheres.size(); i++)
		{
			ImGui::PushID((int)i);
			ImGui::Text("Sphere %d", (int)i);
			ImGui::DragFloat3("Position", glm::value_ptr(m_scene.Spheres[i].Position), 0.1f);
			ImGui::DragFloat("Radius", &m_scene.Spheres[i].Radius, 0.1f);
			ImGui::ColorEdit3("Color", glm::value_ptr(m_scene.Spheres[i].Albedo));
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
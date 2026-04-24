#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

class Camera
{
public:
	Camera(float VerticalFOV, float nearClip, float farClip);

	bool OnUpdate(float ts);
	void OnResize(uint32_t width, uint32_t height);

	const glm::mat4& GetProjection() const { return m_Projection; }
	const glm::mat4& GetView() const { return m_View; }
	const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
	const glm::mat4& GetInverseView() const { return m_InverseView; }

	const glm::vec3& GetPosition() const { return m_Position; }
	const glm::vec3& GetDirection() const { return m_Direction; }

	const std::vector<glm::vec3>& GetRayDirections() const { return m_RayDirections; }

	float GetRotationSpeed();
private:
	void RecalculateView();
	void RecalculateProjection();
	void RecalculateRayDirections();

private:
	glm::mat4 m_Projection;
	glm::mat4 m_View;
	glm::mat4 m_InverseProjection;
	glm::mat4 m_InverseView;

	glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 m_Direction = { 0.0f, 0.0f, 0.0f };

	float m_VerticalFOV;
	float m_NearClip;
	float m_FarClip;

	glm::vec3 m_ForwardDirection{ 0.0f, 0.0f, 0.0f };


	std::vector<glm::vec3> m_RayDirections;

	glm::vec2 m_LastMousePosition{ 0.0f, 0.0f };

	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
};


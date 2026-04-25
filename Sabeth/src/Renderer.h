#pragma once
#include "Walnut/Image.h"
#include <memory>
#include <Walnut/Random.h>
#include <Walnut/Timer.h>
#include <glm/glm.hpp>
#include "camera.h"
#include "ray.h"
#include "scene.h"

class Renderer
{
public:
	struct Settings
	{
		bool Accumulate = true;
		
	};

public:
	Renderer() = default;

	void render(glm::vec3 sphere_color, glm::vec3 light_dir, const Camera& camera, const Scene& scene);
	std::shared_ptr<Walnut::Image> GetFinalImage() { return m_FinalImage; }
	void Resize(uint32_t width, uint32_t height);

	void ResetFrameIndex() { m_FrameIndex = 1; }
	Settings& GetSettings() { return m_Settings; }
private:

	struct HitPayload
	{
		float HitDistance;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;

		int objectIndex;
		
	};

	glm::vec4 perPixel(uint32_t x, uint32_t y); // rayGeneration shader

	HitPayload ClosestHit(const Ray& ra, float hitDistance, int objectIndex); //closest hit shader
	HitPayload TraceRay(const Ray& ray);
	HitPayload Miss(const Ray& ray); // miss shader

private:

	Settings m_Settings;
	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;
	std::shared_ptr<Walnut::Image> m_FinalImage;

	std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVerticalIter;

	uint32_t* m_ImageData = nullptr;
	glm::vec4* m_AccumulationData = nullptr;

	uint32_t m_FrameIndex = 1;
};


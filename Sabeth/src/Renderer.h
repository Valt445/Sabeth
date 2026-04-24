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
	Renderer() = default;

	void render(glm::vec3 sphere_color, glm::vec3 light_dir, const Camera& camera, const Scene& scene);
	std::shared_ptr<Walnut::Image> GetFinalImage() { return m_FinalImage; }
	void Resize(uint32_t width, uint32_t height);

private:
	glm::vec4 TraceRay(const Scene& scene, const Ray& ray, glm::vec3 sphere_color, glm::vec3 light_dir);

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

};


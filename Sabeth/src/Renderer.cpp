#include "Renderer.h"
#include "Walnut/Application.h"
#include <execution>

namespace Utils
{
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);
		return (a << 24) | (b << 16) | (g << 8) | r;
	}
}

void Renderer::render(glm::vec3 sphere_color, glm::vec3 light_dir, const Camera& camera, const Scene& scene)
{
	m_ActiveCamera = &camera;
	m_ActiveScene = &scene;

	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));
#define MT 1
#if MT
	std::for_each(std::execution::par,m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(), [this](uint32_t y)
		{
			for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
			{
				//perPixel(x, y);
				glm::vec4 color = perPixel(x, y);
				m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

				glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
				accumulatedColor /= (float)m_FrameIndex; // average the accumulated color by the number of frames

				accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
				m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
			}
		});
#else
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			//perPixel(x, y);
			glm::vec4 color = perPixel(x, y);
			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

			glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
			accumulatedColor /= (float)m_FrameIndex; // average the accumulated color by the number of frames

			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor); 
		}
	}
#endif

	m_FinalImage->SetData(m_ImageData);

	if (m_Settings.Accumulate)
		m_FrameIndex++;
	else
		m_FrameIndex = 1;

}

void Renderer::Resize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;
		m_FinalImage->Resize(width, height);

	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];

	m_ImageHorizontalIter.resize(width);
	m_ImageVerticalIter.resize(height);
	for (uint32_t i = 0; i < width; i++)
		m_ImageHorizontalIter[i] = i;
	for (uint32_t i = 0; i < height; i++)
		m_ImageVerticalIter[i] = i;

}



Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{
	int closestSphere = -1;
	float hitDistance = std::numeric_limits<float>::max();

	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
	{
		const auto& sphere = m_ActiveScene->Spheres[i];
		glm::vec3 origin = ray.Origin - sphere.Position;

		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;
		//quadratic formula discriminant
		//b^2 - 4ac

		// (-b +- sqrt(discriminant)) / 2a

		float discriminant = b * b - 4.0f * a * c;

		if (discriminant < 0.0f)
		{
			continue;
		}

		//float t0 = (-b + sqrt(discriminant)) / (2.0f * a);
		float closestT = (-b - sqrt(discriminant)) / (2.0f * a);
		if (closestT > 0.0f && closestT < hitDistance)
		{
			hitDistance = closestT;
			closestSphere = int(i);
		}
		// we will only render the first sphere in the scene for now
	}

	if (closestSphere < 0)
	{
		return Miss(ray);
	}

	return ClosestHit(ray, hitDistance, closestSphere);

	
}

glm::vec4 Renderer::perPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 color(0.0f);

	int bounces = 5; // number of bounces for the ray
	float multiplier = 1.0f; // multiplier for the color contribution of each bounce
	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitPayload payload = TraceRay(ray);
		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
			color += skyColor * multiplier;
			break;
		}

		glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1)); // light direction
		// dot product == cosine of the angle between the normal and the light direction

		//and we take the max with 0 to avoid negative lighting (when the light is behind the surface)
		float d = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f); // cosine of the angle between the normal and the light direction

		const Sphere& sphere = m_ActiveScene->Spheres[payload.objectIndex];
		const Material& material = m_ActiveScene->Materials[sphere.materialIndex];
		glm::vec3 sphere_color = material.Albedo; // Apply the sphere's albedo to its color
		sphere_color *= d; // Transform normal from [-1, 1] to [0, 1]
		color += sphere_color * multiplier;
		multiplier *= 0.7f; // reduce the contribution of each subsequent bounce

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f; // offset the ray origin to avoid self-intersection
		ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5)); // reflect the ray direction around the normal
	}

	return glm::vec4(color, 1.0f);

}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];

	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.objectIndex = objectIndex;
	//THE EQUATION it calculates if the ray intersect the circle and the variable which tells us is t
	// (bx^2 + by^2) * t^2 + (2*ax*bx + 2*ay*by) * t + (ax^2 + ay^2 - r^2) = 0
	// a is the ray origin and b is the ray direction
	// r is the radius of the circle
	// t is the hit distance from the ray origin to the hit point

	// the a and b below are coefficents not the ray origin and direction

	glm::vec3 origin = ray.Origin - closestSphere.Position;
	//glm::vec3 h0 = origin + ray.Direction * t0;
	payload.WorldPosition = ray.Origin + ray.Direction * hitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition - closestSphere.Position); // normal of the sphere at the hit point



	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}

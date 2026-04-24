#include "Renderer.h"
#include "Walnut/Application.h"


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
	
	Ray ray;
	ray.Origin = camera.GetPosition();

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];

			glm::vec4 color = TraceRay(scene,ray, sphere_color, light_dir);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color); 
		}
	}
	

	m_FinalImage->SetData(m_ImageData);

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

}

glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray, glm::vec3 sphere_color, glm::vec3 light_dir)
{
	
	

	if (scene.Spheres.size() == 0)
	{
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	const Sphere* closestSphere = nullptr;
	float hitDistance = std::numeric_limits<float>::max();

	for (const auto& sphere : scene.Spheres)
	{
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
		if (closestT < hitDistance)
		{
			hitDistance = closestT;
			closestSphere = &sphere;
		}
		// we will only render the first sphere in the scene for now
	}

	if (closestSphere == nullptr)
	{
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	//THE EQUATION it calculates if the ray intersect the circle and the variable which tells us is t
	// (bx^2 + by^2) * t^2 + (2*ax*bx + 2*ay*by) * t + (ax^2 + ay^2 - r^2) = 0
	// a is the ray origin and b is the ray direction
	// r is the radius of the circle
	// t is the hit distance from the ray origin to the hit point

	// the a and b below are coefficents not the ray origin and direction
	
	glm::vec3 origin = ray.Origin - closestSphere->Position;
	//glm::vec3 h0 = origin + ray.Direction * t0;
	glm::vec3 hitpoint = origin + ray.Direction * hitDistance;
	glm::vec3 normal = glm::normalize(hitpoint - glm::vec3(0.0f)); // normal of the sphere at the hit point

	glm::vec3 lightDir = glm::normalize(light_dir); // light direction
	// dot product == cosine of the angle between the normal and the light direction

	//and we take the max with 0 to avoid negative lighting (when the light is behind the surface)
	float d = glm::max(glm::dot(normal, -lightDir), 0.0f); // cosine of the angle between the normal and the light direction

	sphere_color *= closestSphere->Albedo; // Apply the sphere's albedo to its color
	sphere_color *= d; // Transform normal from [-1, 1] to [0, 1]
	
	return glm::vec4(sphere_color, 1.0f);
}

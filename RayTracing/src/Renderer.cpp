#include "Renderer.h"

#include "Walnut/Random.h"

namespace Utils {

	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
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

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	const glm::vec3& rayOrigin = camera.GetPosition();

	Ray ray;
	ray.Origin = rayOrigin;

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
			glm::vec4 color = TraceRay(scene, ray);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray)
{
	glm::vec4 color = scene.SkyColor;

	if (scene.Spheres.size() == 0)
		return color;

	const Sphere* closestSphere = nullptr;
	float hitDistance = std::numeric_limits<float>::max();
	for (const Sphere& sphere : scene.Spheres)
	{
		// b.bt^2 + (2(a.b) - 2(b.c))t + a.a -2(a.c) + c.c - r^2 = 0
		// where:
		// a = ray origin
		// b = ray direction
		// c = sphere origin
		// r = sphere radius
		// t = distance to intersection

		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(ray.Origin, ray.Direction) - 2.0f * glm::dot(ray.Direction, sphere.Position);
		float c = glm::dot(ray.Origin, ray.Origin) - 2.0f * glm::dot(ray.Origin, sphere.Position) - sphere.Radius * sphere.Radius;

		// quadratic formula discriminant = b^2 - 4ac
		float discriminant = b * b - 4.0f * a * c;


		if (discriminant < 0)
			continue;

		// quadratic formula
		// t = (-b +- sqrt(discriminant))/2a
		// where t is the distance between the rayOrigin and
		// the point the ray hits the sphere.
		float closestT = -b - (glm::sqrt(discriminant) / (2.0f * a));
		if (closestT < hitDistance)
		{
			hitDistance = closestT;
			closestSphere = &sphere;
		}
	}

	if (closestSphere == nullptr)
		return color;

	// hit position
	// using the vector formula
	// hitPosition = rayOrigin + rayDirection * length
	glm::vec3 hitPosition = ray.Origin + ray.Direction * hitDistance;

	// normal
	// normal = hitPosition - sphere.Position
	glm::vec3 normal = hitPosition - closestSphere->Position;
	normal = glm::normalize(normal);

	float intensity = glm::dot(normal, -scene.Light.Position);
	intensity = intensity * 0.5f + 0.5f;

	//color = glm::vec4((scene.Light.Color * intensity) * closestSphere->Albedo), 1.0f);
	color = glm::vec4(scene.Light.Color * intensity * closestSphere->Albedo, 1.0f);

	return color;
}

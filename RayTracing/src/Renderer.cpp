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

void Renderer::Render(const Camera& camera)
{
	const glm::vec3& rayOrigin = camera.GetPosition();

	Ray ray;
	ray.Origin = rayOrigin;

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
			glm::vec4 color = TraceRay(ray);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::TraceRay(const Ray& ray)
{
	//rayDirection = glm::normalize(rayDirection);
	glm::vec3 sphereOrigin(0.0f, 0.0f, -2.0f);
	float radius = 0.5f;
	glm::vec4 sphereColor(1.0f, 1.0f, 0.0f, 1.0f); // RGBA yellow
	glm::vec3 lightSource(-1.0f, -1.0f, 1.0f);
	lightSource = glm::normalize(lightSource);
	glm::vec4 lightSourceColor(0.0f, 1.0f, 1.0f, 1.0f); // RGBA teal

	// b.bt^2 + (2(a.b) - 2(b.c))t + a.a -2(a.c) + c.c - r^2 = 0
	// where:
	// a = ray origin
	// b = ray direction
	// c = sphere origin
	// r = sphere radius
	// t = distance to intersection

	float a = glm::dot(ray.Direction, ray.Direction);
	float b = 2.0f * glm::dot(ray.Origin, ray.Direction) - 2.0f * glm::dot(ray.Direction, sphereOrigin);
	float c = glm::dot(ray.Origin, ray.Origin) - 2.0f * glm::dot(ray.Origin, sphereOrigin) - radius * radius;

	// quadratic formula discriminant = b^2 - 4ac
	float discriminant = b * b - 4.0f * a * c;

	glm::vec4 color = glm::vec4(0, 0, 0, 1);

	if (discriminant < 0)
		return color;

	// quadratic formula
	// t = (-b +- sqrt(discriminant))/2a
	// where t is the distance between the rayOrigin and
	// the point the ray hits the sphere.
	float t[] = {
		(-b - glm::sqrt(discriminant)) / (2.0f * a),
		(-b + glm::sqrt(discriminant)) / (2.0f * a)
	};

	// shade only the nearer hit points for now
	for (int i = 0; i < 1; i++)
	{
		// hit position
		// using the vector formula
		// hitPosition = rayOrigin + rayDirection * length
		glm::vec3 hitPosition = ray.Origin + ray.Direction * t[i];

		// normal
		// normal = hitPosition - sphereOrigin
		glm::vec3 normal = hitPosition - sphereOrigin;
		normal = glm::normalize(normal);

		float intensity = glm::dot(normal, -lightSource);
		intensity = intensity * 0.5f + 0.5f;

		color = (lightSourceColor * intensity) * sphereColor;
		color.a = 1.0f;
	}


	return color;
}

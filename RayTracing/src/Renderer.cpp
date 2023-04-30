#include "Renderer.h"
#include "Renderer.h"

#include "Walnut/Random.h"

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

void Renderer::Render()
{
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };
			coord = coord * 2.0f - 1.0f; // (-1,-1) to (1,1) coordinate space
			float aspectRatio = (float)m_FinalImage->GetWidth() / (float)m_FinalImage->GetHeight();
			coord.x = coord.x * aspectRatio;
			m_ImageData[x + y * m_FinalImage->GetWidth()] = PerPixel(coord);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	glm::vec3 rayOrigin(0.0f, 0.0f, 2.0f);
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	//rayDirection = glm::normalize(rayDirection);
	glm::vec3 sphereOrigin(0.0f, 0.0f, -2.0f);
	float radius = 0.5f;
	glm::vec3 lightSource(-1.0f, -1.0f, 1.0f);
	lightSource = glm::normalize(lightSource);
	uint32_t lightSourceColor = 0xFFFFFF00; //teal

	// b.bt^2 + (2(a.b) - 2(b.c))t + a.a -2(a.c) + c.c - r^2 = 0
	// where:
	// a = ray origin
	// b = ray direction
	// c = sphere origin
	// r = sphere radius
	// t = distance to intersection

	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection) - 2.0f * glm::dot(rayDirection, sphereOrigin);
	float c = glm::dot(rayOrigin, rayOrigin) - 2.0f * glm::dot(rayOrigin, sphereOrigin) - radius * radius;

	// quadratic formula discriminant = b^2 - 4ac
	float discriminant = b * b - 4.0f * a * c;

	uint32_t color = 0xff000000;

	if (discriminant >= 0)
	{
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
			glm::vec3 hitPosition = rayOrigin + rayDirection * t[i];

			// normal
			// normal = hitPosition - sphereOrigin
			glm::vec3 normal = hitPosition - sphereOrigin;
			normal = glm::normalize(normal);

			float shade = glm::dot(normal, -lightSource);

			uint8_t r = (uint8_t)((shade * 0.5f + 0.5f) * (lightSourceColor & 0xff));
			uint8_t g = (uint8_t)((shade * 0.5f + 0.5f) * ((lightSourceColor >> 8) & 0xff));
			uint8_t b = (uint8_t)((shade * 0.5f + 0.5f) * ((lightSourceColor >> 16) & 0xff));

			color = 0xff000000 + (b << 16) + (g << 8) + r;
		}
	}

	return color;
}

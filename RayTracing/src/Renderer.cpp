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
	glm::vec3 circleOrigin(0.0f, 0.0f, -2.0f);
	float radius = 0.5f;

	// b.bt^2 + (2(a.b) - 2(b.c))t + a.a -2(a.c) + c.c - r^2 = 0
	// where:
	// a = ray origin
	// b = ray direction
	// c = circle origin
	// r = circle radius
	// t = distance to intersection

	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection) - 2.0f * glm::dot(rayDirection, circleOrigin);
	float c = glm::dot(rayOrigin, rayOrigin) - 2.0f * glm::dot(rayOrigin, circleOrigin) - radius * radius;

	// quadratic formula discriminant = b^2 - 4ac
	float discriminant = b * b - 4.0f * a * c;

	// quadratic formula
	// t = (-b +- sqr(discriminant))/2a

	

	uint32_t color = 0xff000000;
	if (discriminant >= 0)
		color = 0xffff00ff;

	return color;
}

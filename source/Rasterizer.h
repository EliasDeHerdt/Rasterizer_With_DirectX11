#pragma once
#include <vector>
#include "Structs.h"

namespace Rasterizer {
	
	inline void VertexTransformationFunction(const std::vector<InputVertex>& originalVertices,
		std::vector<OutputVertex>& transformedVertices, const Elite::FPoint3& cameraPos, const Elite::FMatrix4& cameraToWorld, const Elite::FMatrix4& world,
		const Elite::FMatrix4& ProjectionMatrix, float screenWidth, float screenHeight, float near, float far, float FOV) {

		Elite::FMatrix4 WorldViewProjectionMatrix = ProjectionMatrix * cameraToWorld * world;

		transformedVertices.clear();
		for (const InputVertex& currentVertex : originalVertices) {

			//ViewDirection
			Elite::FVector3 direction = Elite::GetNormalized(cameraPos - currentVertex.Position);

			//Creating the OutputVertex
			transformedVertices.push_back(OutputVertex{ Elite::FPoint4{ currentVertex.Position, 1.f }, currentVertex.UV, currentVertex.Normal, currentVertex.Tangent, currentVertex.Color, direction });
			Elite::FPoint4& position = transformedVertices.back().Position;
			Elite::FVector3& normal = transformedVertices.back().Normal;
			Elite::FVector3& tangent = transformedVertices.back().Tangent;

			//ViewSpace
			position = WorldViewProjectionMatrix * position;
			normal = (Elite::FMatrix3)world * Elite::GetNormalized(normal);
			tangent = (Elite::FMatrix3)world * Elite::GetNormalized(tangent);

			position.x /= position.w;
			position.y /= position.w;
			position.z /= position.w;

			//ProjectionSpace
			/*reference.x = (reference.x / -reference.z) / (aspectRatio * FOV);
			reference.y = (reference.y / -reference.z) / FOV;
			reference.z = -reference.z;*/
			
			//ScreenSpace
			position.x = ((position.x + 1) / 2) * screenWidth;
			position.y = ((1 - position.y) / 2) * screenHeight;
		}
	}

	inline std::pair<Elite::FPoint2, Elite::FPoint2> CreateBoundingBox(const std::vector<OutputVertex>& vertices, uint32_t width, uint32_t height) {
		
		std::pair<Elite::FPoint2, Elite::FPoint2> minMax;
		auto minmaxX = std::minmax_element(vertices.begin(), vertices.end(), [](const OutputVertex& lhs, const OutputVertex& rhs) {
			return lhs.Position.x < rhs.Position.x;
			});
		auto minmaxY = std::minmax_element(vertices.begin(), vertices.end(), [](const OutputVertex& lhs, const OutputVertex& rhs) {
			return lhs.Position.y < rhs.Position.y;
			});

		//min
		minMax.first.x = Elite::Clamp((minmaxX.first->Position.x - 1), 0.f, float(width - 1));
		minMax.first.y = Elite::Clamp((minmaxY.first->Position.y - 1), 0.f, float(height - 1));

		//max
		minMax.second.x = Elite::Clamp((minmaxX.second->Position.x + 1), 0.f, float(width - 1));
		minMax.second.y = Elite::Clamp((minmaxY.second->Position.y + 1), 0.f, float(height - 1));

		return minMax;
	}

	inline std::pair<Elite::FPoint2, Elite::FPoint2> CreateBoundingBox(const Elite::FPoint4& v0, const Elite::FPoint4& v1, const Elite::FPoint4& v2, uint32_t width, uint32_t height) {

		std::pair<Elite::FPoint2, Elite::FPoint2> minMax;
		std::vector<Elite::FPoint4> vertices = { v0, v1, v2 };
		auto minmaxX = std::minmax_element(vertices.begin(), vertices.end(), [](const Elite::FPoint4& lhs, const Elite::FPoint4& rhs) {
			return lhs.x < rhs.x;
			});
		auto minmaxY = std::minmax_element(vertices.begin(), vertices.end(), [](const Elite::FPoint4& lhs, const Elite::FPoint4& rhs) {
			return lhs.y < rhs.y;
			});

		//min
		minMax.first.x = Elite::Clamp((minmaxX.first->x - 1), 0.f, float(width - 1));
		minMax.first.y = Elite::Clamp((minmaxY.first->y - 1), 0.f, float(height - 1));

		//max
		minMax.second.x = Elite::Clamp((minmaxX.second->x + 1), 0.f, float(width - 1));
		minMax.second.y = Elite::Clamp((minmaxY.second->y + 1), 0.f, float(height - 1));

		return minMax;
	}

	inline Elite::RGBColor PixelShading(const OutputVertex& v, Elite::RGBColor normalMapSample, Elite::RGBColor SpecularMapSample, float GlossyMapSample, float Shininess, float DirLightIntensity) {

		Elite::FVector3 binormal = Elite::GetNormalized(Elite::Cross(v.Tangent, v.Normal));
		Elite::FMatrix3 tangentSpaceAxis = Elite::FMatrix3(v.Tangent, binormal, v.Normal);

		//We already return a color / 255.f in our Texture Sample(). So we don't need to do it here!
		Elite::FVector3 sampledNormal = { normalMapSample.r, normalMapSample.g, normalMapSample.b };
		sampledNormal = 2.f * sampledNormal - Elite::FVector3{ 1.f, 1.f, 1.f };

		Elite::FVector3 newNormal = tangentSpaceAxis * sampledNormal;
		Elite::FVector3 lightDirection = { 0.577f, -0.577f, -0.577f };
		float observedArea = Elite::Dot(-newNormal, lightDirection);

		//Lambert calculation
		Elite::RGBColor lightColor = { 1.f, 1.f, 1.f };
		Elite::RGBColor Lambert = (SpecularMapSample * v.Color) / (float)M_PI;
		Lambert *= lightColor * DirLightIntensity * observedArea;

		//Phong calculations
		//observedArea is the same calculation as LambertCosignLaw
		//Specular- and glossy map are greyscales, so every value of RGB is the same value!
		Elite::RGBColor Phong{0.f, 0.f, 0.f};
		if (observedArea >= 0) {

			Elite::FVector3 reflect = lightDirection - 2 * (observedArea * -newNormal);
			float angle = Elite::Clamp(Elite::Dot(reflect, v.ViewDirection), 0.f, 1.f);
			Phong = SpecularMapSample * (float)pow(angle, int(GlossyMapSample * Shininess));
		}

		
		return Lambert + Phong;
	}
}
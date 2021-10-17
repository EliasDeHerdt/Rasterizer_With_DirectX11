#include "pch.h"
#include "ObjParser.h"
#include "Structs.h"
#include <fstream>
#include <iostream>
#include <regex>

ObjParser* ObjParser::m_Instance = nullptr;
ObjParser::ObjParser()
	: m_InputVertexBuffer{}
	, m_IndexBuffer{}
	, m_VertexBuffer{}
	, m_NormalBuffer{}
	, m_UvBuffer{}
	, m_FaceBuffer{}
{
}

std::pair<const std::vector<InputVertex>&, const std::vector<uint32_t>&> ObjParser::ReadObjFile(std::string path)
{
	int index{};
	std::ifstream input(path);
	std::string currentLine{};
	m_InputVertexBuffer.clear();
	m_IndexBuffer.clear();
	m_VertexBuffer.clear();
	m_NormalBuffer.clear();
	m_UvBuffer.clear();
	m_FaceBuffer.clear();
	if (!input)
		ThrowFileError(path);

	while (std::getline(input, currentLine)) {

		++index;
		std::smatch match{};
		std::regex lineType{ "^([#A-Za-z]+)\\s+(.*)" };
		if (std::regex_match(currentLine.cbegin(), currentLine.cend(), match, lineType)) {

			if (match[1] == "v") {

				std::string dataToCheck{ match[2].str() };
				std::regex dataSearch{ "^([\\-\\.0-9e]+) ([\\-\\.0-9e]+) ([\\-\\.0-9e]+)" };
				if (std::regex_match(dataToCheck.cbegin(), dataToCheck.cend(), match, dataSearch)) {

					m_VertexBuffer.push_back(Elite::FPoint3{std::stof(match[1].str()) , std::stof(match[2].str()), std::stof(match[3].str())} );
				}
				else {

					ThrowIndexError("Vertex", index);
				}
			}
			else if (match[1] == "vn") {

				std::string dataToCheck{ match[2].str() };
				std::regex dataSearch{ "^([\\-\\.0-9e]+) ([\\-\\.0-9e]+) ([\\-\\.0-9e]+)\\s*" };
				if (std::regex_match(dataToCheck.cbegin(), dataToCheck.cend(), match, dataSearch)) {

					m_NormalBuffer.push_back(Elite::FVector3{ std::stof(match[1].str()) , std::stof(match[2].str()), std::stof(match[3].str()) });
				}
				else {

					ThrowIndexError("Normal", index);
				}
			}
			else if (match[1] == "vt") {

				std::string dataToCheck{ match[2].str() };
				std::regex dataSearch{ "^([\\-\\.0-9e]+) ([\\-\\.0-9e]+) [\\-\\.0-9e]+\\s*" };
				if (std::regex_match(dataToCheck.cbegin(), dataToCheck.cend(), match, dataSearch)) {

					m_UvBuffer.push_back(Elite::FVector2{ std::stof(match[1].str()) , 1 - std::stof(match[2].str()) });
				}
				else {

					ThrowIndexError("Texture Coords", index);
				}
			}
			else if (match[1] == "f") {

				std::string dataToCheck{ match[2].str() };
				std::regex dataSearch{ "^(\\d+)/(\\d+)/(\\d+)\\s(\\d+)/(\\d+)/(\\d+)\\s(\\d+)/(\\d+)/(\\d+)\\s*" };
				if (std::regex_match(dataToCheck.cbegin(), dataToCheck.cend(), match, dataSearch)) {

					m_FaceBuffer.push_back({ std::stoi(match[1].str()) - 1, std::stoi(match[2].str()) - 1, (std::stoi(match[3].str()) - 1) });
					m_FaceBuffer.push_back({ std::stoi(match[4].str()) - 1, std::stoi(match[5].str()) - 1, (std::stoi(match[6].str()) - 1) });
					m_FaceBuffer.push_back({ std::stoi(match[7].str()) - 1, std::stoi(match[8].str()) - 1, (std::stoi(match[9].str()) - 1) });
				}
				else {

					ThrowIndexError("Face", index);
				}
			}
		}
	}
	input.close();

	for (Elite::IPoint3 face : m_FaceBuffer) {
		InputVertex vertex = { m_VertexBuffer[face.x], m_UvBuffer[face.y], m_NormalBuffer[face.z] };

		int index{ 0 };
		bool found{ false };
		for (; index < m_InputVertexBuffer.size(); ++index)
			if (found = (vertex == m_InputVertexBuffer[index]))
				break;

		if (!found) {
			
			m_IndexBuffer.push_back(int(m_InputVertexBuffer.size()));
			m_InputVertexBuffer.push_back(vertex);
		}
		else {
			
			m_IndexBuffer.push_back(index);
		}
	}

	//Following code is heavily based on code found at the following link:
	//https://stackoverflow.com/questions/5255806/how-to-calculate-tangent-and-binormal
	for (uint32_t i = 0; i < m_IndexBuffer.size(); i += 3) {

		uint32_t index0 = m_IndexBuffer[i];
		uint32_t index1 = m_IndexBuffer[i + 1];
		uint32_t index2 = m_IndexBuffer[i + 2];

		const Elite::FPoint3& p0 = m_InputVertexBuffer[index0].Position;
		const Elite::FPoint3& p1 = m_InputVertexBuffer[index1].Position;
		const Elite::FPoint3& p2 = m_InputVertexBuffer[index2].Position;
		const Elite::FVector2& uv0 = m_InputVertexBuffer[index0].UV;
		const Elite::FVector2& uv1 = m_InputVertexBuffer[index1].UV;
		const Elite::FVector2& uv2 = m_InputVertexBuffer[index2].UV;

		const Elite::FVector3 edge0 = p1 - p0;
		const Elite::FVector3 edge1 = p2 - p0;
		const Elite::FVector2 diffX = Elite::FVector2(uv1.x - uv0.x, uv2.x - uv0.x);
		const Elite::FVector2 diffY = Elite::FVector2(uv1.y - uv0.y, uv2.y - uv0.y);
		float r = 1.f / Elite::Cross(diffX, diffY);

		Elite::FVector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
		m_InputVertexBuffer[index0].Tangent += tangent;
		m_InputVertexBuffer[index1].Tangent += tangent;
		m_InputVertexBuffer[index2].Tangent += tangent;
	}
	//Create the tangets (reject vector) + fix the tangents per vertex
	for (auto& v : m_InputVertexBuffer)
		v.Tangent = Elite::GetNormalized(Elite::Reject(v.Tangent, v.Normal));

	return std::pair<const std::vector<InputVertex>&, const std::vector<uint32_t>&>(m_InputVertexBuffer, m_IndexBuffer);
}


void ObjParser::ThrowFileError(const std::string& path)
{
	throw std::runtime_error("Could not open file '" + path + "', file not found.\n");
}

void ObjParser::ThrowIndexError(const std::string& dataType, int index)
{
	throw std::runtime_error("Could not read data of " + dataType + " at line " + std::to_string(index) + ".\n");
}
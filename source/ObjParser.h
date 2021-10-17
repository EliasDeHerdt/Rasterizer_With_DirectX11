#pragma once
#include <string>
#include <vector>

struct InputVertex;
class ObjParser final
{
public:
	static ObjParser* GetInstance() {
		if (m_Instance == nullptr) {
			m_Instance = new ObjParser();
		}
		return m_Instance;
	}
	~ObjParser() = default;
	ObjParser(const ObjParser& other) = delete;
	ObjParser& operator=(const ObjParser& other) = delete;
	ObjParser(ObjParser&& other) = delete;
	ObjParser& operator=(ObjParser&& other) = delete;

	std::pair<const std::vector<InputVertex>&, const std::vector<uint32_t>&> ReadObjFile(std::string path);
private:
	ObjParser();

	//Variables
	static ObjParser* m_Instance;
	std::vector<InputVertex> m_InputVertexBuffer;
	std::vector<uint32_t> m_IndexBuffer;
	std::vector<Elite::FPoint3> m_VertexBuffer;
	std::vector<Elite::FVector3> m_NormalBuffer;
	std::vector<Elite::FVector2> m_UvBuffer;
	std::vector<Elite::IPoint3> m_FaceBuffer;

	//Functions
	void ThrowFileError(const std::string& path);
	void ThrowIndexError(const std::string& dataType, int index);
};


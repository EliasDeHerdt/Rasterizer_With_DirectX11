#pragma once
#include <vector>
#include "BaseEffect.h"
#include "Texture.h"

struct InputVertex;
struct OutputVertex;
enum class RenderMode;
class Mesh final
{
public:
	enum class PrimitiveToplogy {

		TriangleList = 3,
		TriangleStrip = 1
	};
	Mesh(bool rotating, const Elite::FVector3& displacement, const std::string& texturePath, const std::string& normalMapPath, const std::string& specularMapPath, const std::string& glossinessMapPath, ID3D11Device* pDevice, const std::vector<InputVertex>& vertices, const std::vector<uint32_t>& indices, BaseEffect* effect, PrimitiveToplogy PrimitiveToplogy = PrimitiveToplogy::TriangleList);
	~Mesh();
	Mesh(const Mesh& other) = delete;
	Mesh& operator=(const Mesh& other) = delete;
	Mesh(Mesh&& other) = delete;
	Mesh& operator=(Mesh&& other) = delete;

	void Update(float deltaTime);
	void Rotate(float angle, const Elite::FVector3& axis, RenderMode mode, float delta);

	//DirectX
	void Render(ID3D11DeviceContext* pDeviceContext);
	const Elite::FMatrix4& GetWorldMatrix() const;
	const Texture& GetTexture() const;
	const Texture& GetNormalMap() const;
	const Texture& GetSpecularMap() const;
	const Texture& GetGlossinessMap() const;
	BaseEffect* GetEffect() const;

	//Rasterizer
	BaseEffect::Culling GetCullMode() const;
	const std::vector<InputVertex>& GetVertices() const;
	const PrimitiveToplogy GetPrimitveTopology() const;
	const int GetNrOfTriangles() const;
	const float GetLightIntensity() const;
	const float GetShininess() const;
	void GetTriangleIndices(int tIndex, int& i1, int& i2, int& i3) const;
	const Elite::RGBColor SampleTexture(const Elite::FVector2& uv) const;
	const Elite::RGBColor SampleNormalMap(const Elite::FVector2& uv) const;
	const Elite::RGBColor SampleSpecularMap(const Elite::FVector2& uv) const;
	const float SampleGlossinessMap(const Elite::FVector2& uv) const;
private:
	bool m_Rotating;
	Elite::FMatrix4 m_WorldMatrix;
	Texture m_Texture;
	Texture m_NormalMap;
	Texture m_SpecularMap;
	Texture m_GlossinessMap;

	//DirectX
	ID3D11Device* m_Device; //not the meshes job to release this
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;

	BaseEffect* m_pEffect; //not the meshes job to release this
	ID3D11InputLayout* m_pVertexLayout;
	uint32_t m_AmountIndices;

	//Rasterizer
	std::vector<InputVertex> m_VertexBuffer;
	std::vector<uint32_t> m_IndexBuffer;
	PrimitiveToplogy m_PrimitiveToplogy;

	std::vector<InputVertex> GetDirectXReadyVertices() const;
};


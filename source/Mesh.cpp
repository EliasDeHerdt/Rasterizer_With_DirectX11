#include "pch.h"
#include "Mesh.h"
#include "Structs.h"
#include "SceneGraph.h"

Mesh::Mesh(bool rotating, const Elite::FVector3& displacement, const std::string& texturePath, const std::string& normalMapPath, const std::string& specularMapPath, const std::string& glossinessMapPath, ID3D11Device* pDevice, const std::vector<InputVertex>& vertices, const std::vector<uint32_t>& indices, BaseEffect* effect, PrimitiveToplogy PrimitiveToplogy)
	: m_Rotating{ rotating }
	, m_WorldMatrix{ Elite::MakeTranslation(displacement) }
	, m_Texture{ texturePath, pDevice }
	, m_NormalMap{ normalMapPath, pDevice }
	, m_SpecularMap{ specularMapPath, pDevice }
	, m_GlossinessMap{ glossinessMapPath, pDevice }
	, m_Device{ pDevice }
	, m_pVertexBuffer{ }
	, m_pIndexBuffer{ }
	, m_pEffect{ effect }
	, m_pVertexLayout{ }
	, m_AmountIndices{ }
	, m_VertexBuffer{ vertices }
	, m_IndexBuffer{ indices }
	, m_PrimitiveToplogy{ PrimitiveToplogy }
{
	//Create Vertex Layout
	HRESULT result = S_OK;
	static const uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "TEXCOORD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 20;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "COLOR";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[4].AlignedByteOffset = 44;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//Create Vertex Buffer
	std::vector<InputVertex> flippedVertices = GetDirectXReadyVertices();

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(InputVertex) * (uint32_t)flippedVertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = flippedVertices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		return;

	//Create the input layout
	D3DX11_PASS_DESC passDesc;
	m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
	result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, 
		&m_pVertexLayout);
	if (FAILED(result))
		return;

	//Create index buffer
	m_AmountIndices = (int32_t)indices.size();
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(int32_t) * m_AmountIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
		return;
}

Mesh::~Mesh()
{
	if (m_pVertexBuffer)
		m_pVertexBuffer->Release();
	if (m_pIndexBuffer)
		m_pIndexBuffer->Release();
	if (m_pVertexLayout)
		m_pVertexLayout->Release();
}

void Mesh::Update(float deltaTime)
{
	if (m_Rotating)
		Rotate(-1.f, Elite::FVector3{ 0.f, 1.f, 0.f }, SceneGraph::GetInstance()->GetRenderMode(), deltaTime);
}

void Mesh::Rotate(float angle, const Elite::FVector3& axis, RenderMode mode, float delta)
{
	int renderMode = int(mode);
	float angleCorrection = angle * renderMode;
	Elite::FVector3 axisCorrection = {axis.x, axis.y, axis.z * renderMode };
	Elite::FMatrix3 rotationMatrix;

	if (delta == 0)
		rotationMatrix = Elite::MakeRotation(angleCorrection, axisCorrection);
	else
		rotationMatrix = Elite::MakeRotation(angleCorrection * delta, axisCorrection);

	m_WorldMatrix *= Elite::FMatrix4(rotationMatrix);
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext)
{
	UINT stride = sizeof(InputVertex);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//Set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set the input layour
	pDeviceContext->IASetInputLayout(m_pVertexLayout);

	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Render a triangle
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pEffect->GetTechnique()->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p) {

		m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
	}
}

const Elite::FMatrix4& Mesh::GetWorldMatrix() const
{
	return m_WorldMatrix;
}

const Texture& Mesh::GetTexture() const
{
	return m_Texture;
}

const Texture& Mesh::GetNormalMap() const
{
	return m_NormalMap;
}

const Texture& Mesh::GetSpecularMap() const
{
	return m_SpecularMap;
}

const Texture& Mesh::GetGlossinessMap() const
{
	return m_GlossinessMap;
}

BaseEffect* Mesh::GetEffect() const
{
	return m_pEffect;
}

BaseEffect::Culling Mesh::GetCullMode() const
{
	return m_pEffect->GetCullMode();
}

const std::vector<InputVertex>& Mesh::GetVertices() const
{
	return m_VertexBuffer;
}

const Mesh::PrimitiveToplogy Mesh::GetPrimitveTopology() const
{
	return m_PrimitiveToplogy;
}

//Return the amount of times we need to loop to get all triangles
const int Mesh::GetNrOfTriangles() const
{
	return (int)m_IndexBuffer.size() - 2;
}

const float Mesh::GetLightIntensity() const
{
	return m_pEffect->GetLightIntensity();
}

//Returns -1 if no shininess was used 
const float Mesh::GetShininess() const
{
	return m_pEffect->GetShininess();
}

//Check which topology we use and return the correct indices based
//on the the given TriangleIndex
void Mesh::GetTriangleIndices(int tIndex, int& i1, int& i2, int& i3) const
{
	switch (m_PrimitiveToplogy)
	{
	case Mesh::PrimitiveToplogy::TriangleList:
		i1 = m_IndexBuffer[tIndex];
		i2 = m_IndexBuffer[tIndex + 1];
		i3 = m_IndexBuffer[tIndex + 2];
		break;
	case Mesh::PrimitiveToplogy::TriangleStrip:
		//Check order of indices
		i1 = m_IndexBuffer[tIndex];
		i2 = m_IndexBuffer[tIndex + ((tIndex % 2) + 1)];
		i3 = m_IndexBuffer[tIndex + (2 - (tIndex % 2))];
		break;
	}
}

const Elite::RGBColor Mesh::SampleTexture(const Elite::FVector2& uv) const
{
	return m_Texture.Sample(uv);
}

const Elite::RGBColor Mesh::SampleNormalMap(const Elite::FVector2& uv) const
{
	return m_NormalMap.Sample(uv);
}

const Elite::RGBColor Mesh::SampleSpecularMap(const Elite::FVector2& uv) const
{
	return m_SpecularMap.Sample(uv);
}

const float Mesh::SampleGlossinessMap(const Elite::FVector2& uv) const
{
	return m_GlossinessMap.Sample(uv).r;
}

std::vector<InputVertex> Mesh::GetDirectXReadyVertices() const
{
	std::vector<InputVertex> flippedVertices;

	for (const InputVertex& currentVertex : m_VertexBuffer) {

		Elite::FPoint3 flippedPosition = { currentVertex.Position.x, currentVertex.Position.y, -currentVertex.Position.z };
		Elite::FVector3 flippedNormal = { currentVertex.Normal.x, currentVertex.Normal.y, -currentVertex.Normal.z };
		Elite::FVector3 flippedTangent = { currentVertex.Tangent.x, currentVertex.Tangent.y, -currentVertex.Tangent.z };

		InputVertex flippedVertex = { flippedPosition , currentVertex.UV, flippedNormal, flippedTangent, currentVertex.Color};
		flippedVertices.push_back(flippedVertex);
	}

	return flippedVertices;
}

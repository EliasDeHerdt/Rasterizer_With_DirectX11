#include "pch.h"
#include "SDL_surface.h"

//Project includes
#include "ERenderer.h"
#include "SceneGraph.h"
#include "CameraManager.h"
#include "EffectManager.h"
#include "Rasterizer.h"

Elite::Renderer::Renderer(SDL_Window * pWindow)
	: m_pWindow{ pWindow }
	, m_Width{}
	, m_Height{}
	, m_IsInitialized{ false }
{
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	int width, height = 0;
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	m_DepthBuffer = std::vector<float>(width * height);

	//Initialize DirectX pipeline
	if (InitializeDirectX() == 0) {

		m_IsInitialized = true;
		std::cout << "DirectX is ready\n";
	} else 
		std::cout << "DirectX failed\n";

	PrintEffectRenderingInformation();
	PrintDepthRenderingInformation();
}

Elite::Renderer::~Renderer()
{
	if (m_pRenderTargetView)
		m_pRenderTargetView->Release();
	if (m_pRenderTargetBuffer)
		m_pRenderTargetBuffer->Release();

	if (m_pDepthStencilView)
		m_pDepthStencilView->Release();
	if (m_pDepthStencilBuffer)
		m_pDepthStencilBuffer->Release();

	if (m_pSwapChain)
		m_pSwapChain->Release();
	if (m_pDXGIFactory)
		m_pDXGIFactory->Release();

	if (m_pDeviceContext) {

		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}
	if (m_pDevice)
		m_pDevice->Release();
}

void Elite::Renderer::Render()
{
	RGBColor clearColor = RGBColor(0.f, 0.f, 0.3f);
	const std::vector<Mesh*>& meshes = SceneGraph::GetInstance()->GetObjects();
	const Camera* activeCamera = CameraManager::GetInstance()->GetActiveCamera();

	switch (SceneGraph::GetInstance()->GetRenderMode()) {
	case RenderMode::DirectX:
		{
			if (!m_IsInitialized)
				return;

			//Clear Buffers
			m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
			m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

			//Render
			for (Mesh* currentMesh : meshes) {

				//Set temporary variables that get called more than once
				BaseEffect* currentEffect = currentMesh->GetEffect();

				currentEffect->SetWorldViewProjectionMatrix(activeCamera->GetViewMatrix(), activeCamera->GetProjectionMatrix(), currentMesh->GetWorldMatrix());
				currentEffect->SetDiffuseMap(currentMesh->GetTexture().GetTextureResourceView());
				//Set variables unique to each mesh
				switch (currentEffect->GetEffectType())
				{
				case BaseEffect::EffectType::Material:
				{
					MaterialEffect* materialEffect = static_cast<MaterialEffect*>(currentEffect);
					//Matrices
					materialEffect->SetWorldViewProjectionMatrix(activeCamera->GetViewMatrix(), activeCamera->GetProjectionMatrix(), currentMesh->GetWorldMatrix());
					materialEffect->SetWorldMatrix(currentMesh->GetWorldMatrix());
					materialEffect->SetViewInverseMatrix(activeCamera->GetInverseViewMatrix());

					//Textures
					materialEffect->SetNormalMap(currentMesh->GetNormalMap().GetTextureResourceView());
					materialEffect->SetSpecularMap(currentMesh->GetSpecularMap().GetTextureResourceView());
					materialEffect->SetGlossinessMap(currentMesh->GetGlossinessMap().GetTextureResourceView());
				}
				break;
				case BaseEffect::EffectType::Flat:
				{
					if (!m_RenderEffects)
						continue;
				}
				break;
				default:
					break;
				}

				//Render the currentMesh
				currentMesh->Render(m_pDeviceContext);
			}

			//Present
			m_pSwapChain->Present(0, 0);
		}
		break;
	case RenderMode::Rasterizer:
		{
			SDL_LockSurface(m_pBackBuffer);

			//Initialize variables
			std::vector<OutputVertex> vertices;
			std::for_each(m_DepthBuffer.begin(), m_DepthBuffer.end(), [](float& value) {
				value = FLT_MAX;
				});

			const float farPlane = activeCamera->GetFarPlane();
			const float nearPlane = activeCamera->GetNearPlane();
			const float FOV = activeCamera->GetFOV();
			const Elite::FPoint3& cameraLocation = activeCamera->GetLocation();
			const Elite::FMatrix4& lookAtMatrix = activeCamera->GetViewMatrix();


			//Set pixel to black
			for (uint32_t r = 0; r < m_Height; ++r) {
				for (uint32_t c = 0; c < m_Width; ++c) {

					m_pBackBufferPixels[c + (r * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(clearColor.r * 255),
						static_cast<uint8_t>(clearColor.g * 255),
						static_cast<uint8_t>(clearColor.b * 255));
				}
			}

			//Loop over all Objects
			for (Mesh* currentMesh : meshes) {
			
				if (!m_RenderEffects && currentMesh->GetEffect()->GetEffectType() != BaseEffect::EffectType::Material)
					continue;

				Rasterizer::VertexTransformationFunction(currentMesh->GetVertices(), vertices, cameraLocation, lookAtMatrix, currentMesh->GetWorldMatrix(), activeCamera->GetProjectionMatrix(), (float)m_Width, (float)m_Height, nearPlane, farPlane, FOV);

				//loop over all indices
				Mesh::PrimitiveToplogy topology = currentMesh->GetPrimitveTopology();
				for (int i = 0; i < currentMesh->GetNrOfTriangles(); i += (int)topology) {

					//Check which topology we use and implement it
					int index0{}, index1{}, index2{};
					currentMesh->GetTriangleIndices(i, index0, index1, index2);

					//If end of strip (surface triangle), continue
					if (index0 == index1 || index1 == index2 || index0 == index2)
						continue;

					const Elite::FPoint4& v0 = vertices[index0].Position;
					const Elite::FPoint4& v1 = vertices[index1].Position;
					const Elite::FPoint4& v2 = vertices[index2].Position;

					//Frustrum culling
					if (v0.z < 0 || v0.z > 1)
						continue;

					if (v1.z < 0 || v1.z > 1)
						continue;

					if (v2.z < 0 || v2.z > 1)
						continue;
					
					//Calculate total weight and create the bounding box for the current triangle
					float totalWeight = Elite::Cross(v0.xy - v1.xy, v0.xy - v2.xy);
					std::pair<Elite::FPoint2, Elite::FPoint2> boundingBox = Rasterizer::CreateBoundingBox(v0, v1, v2, m_Width, m_Height);

					//Loop over all the pixels
					for (uint32_t r = uint32_t(boundingBox.first.y); r < boundingBox.second.y; ++r)
					{
						for (uint32_t c = uint32_t(boundingBox.first.x); c < boundingBox.second.x; ++c)
						{

							//Create current pixel
							float weight0, weight1, weight2;
							if (!PixelInTri((float)c, (float)r, v0, v1, v2, weight0, weight1, weight2, currentMesh->GetCullMode()))
								continue;

							//Transform weights into ratio's
							weight0 /= totalWeight;
							weight1 /= totalWeight;
							weight2 /= totalWeight;

							//Check if the weights add up to 1
							if (std::round(weight0 + weight1 + weight2) != 1)
								continue;

							//Calculate the depth for a depth-check
							float depth{};
						
							CalculateDepthBuffer(depth, weight0, weight1, weight2, index0, index1, index2, vertices);
							if (depth > 0.f && depth < 1.f && depth < m_DepthBuffer[c + (r * m_Width)]) {

								//Set depth to found depth and recalculate it for calculations
								m_DepthBuffer[c + (r * m_Width)] = depth;
								CalculateDepthInterpolated(depth, weight0, weight1, weight2, index0, index1, index2, vertices);

								Elite::RGBColor finalColor{};
								if (!m_DepthRendering) {

									//Uv calculation
									Elite::FVector2 finalUV{};
									CalculateUV(finalUV, depth, weight0, weight1, weight2, index0, index1, index2, vertices);

									//Color calculation (either with uv or colors)
									finalColor = currentMesh->SampleTexture(finalUV);

									//Normal Calculation
									Elite::FVector3 finalNormal{};
									CalculateNormal(finalNormal, depth, weight0, weight1, weight2, index0, index1, index2, vertices);

									//Tangent Calculation
									Elite::FVector3 finalTangent{};
									CalculateTangent(finalTangent, depth, weight0, weight1, weight2, index0, index1, index2, vertices);

									//ViewDirection Calculation
									Elite::FVector3 finalViewDirection{};
									CalculateViewDirection(finalViewDirection, depth, weight0, weight1, weight2, index0, index1, index2, vertices);

									//Lighting Calculation
									if (currentMesh->GetNormalMap().IsValid() && currentMesh->GetSpecularMap().IsValid() && currentMesh->GetGlossinessMap().IsValid())
										finalColor = Rasterizer::PixelShading(OutputVertex{ {}, finalUV, finalNormal, finalTangent, finalColor, finalViewDirection }, currentMesh->SampleNormalMap(finalUV), currentMesh->SampleSpecularMap(finalUV), currentMesh->SampleGlossinessMap(finalUV), currentMesh->GetShininess(), currentMesh->GetLightIntensity());
								}
								else {
									float depthColor = Elite::Remap(m_DepthBuffer[c + (r * m_Width)], 0.985f, 1.f);
									finalColor = { depthColor, depthColor, depthColor };
								}
								finalColor.MaxToOne();
								finalColor.Clamp();

								//Color the pixels
								m_pBackBufferPixels[c + (r * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
									static_cast<uint8_t>(finalColor.r * 255.f),
									static_cast<uint8_t>(finalColor.g * 255.f),
									static_cast<uint8_t>(finalColor.b * 255.f));
							}
						}
					}
				}
			}
			SDL_UnlockSurface(m_pBackBuffer);
			SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
			SDL_UpdateWindowSurface(m_pWindow);
		}
		break;
	default:
		break;
	}
}

void Elite::Renderer::ToggleDepthRendering()
{
	m_DepthRendering = !m_DepthRendering;
	PrintDepthRenderingInformation();
}

void Elite::Renderer::PrintDepthRenderingInformation()
{
	std::cout << "Depth Rendering: ";
	if (m_DepthRendering)
		std::cout << "true\n";
	else
		std::cout << "false\n";
}

ID3D11Device* Elite::Renderer::GetDevice()
{
	return m_pDevice;
}

void Elite::Renderer::ToggleEffectRendering()
{
	m_RenderEffects = !m_RenderEffects;
	PrintEffectRenderingInformation();
}

void Elite::Renderer::PrintEffectRenderingInformation()
{
	std::cout << "Effect Rendering: ";
	if (m_RenderEffects)
		std::cout << "true\n";
	else
		std::cout << "false\n";
}

long Elite::Renderer::InitializeDirectX()
{
	//Create Device and Device context, using hardware acceleration
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32_t createDeviceFlags = 0;

	#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	HRESULT result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext);
	if (FAILED(result))
		return result;

	//Create DXGI Factory to create SwapChain based on hardware
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDXGIFactory));
	if (FAILED(result))
		return result;

	//Create SwapChain Descriptor
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1; // <--- this should be the default value, but throws error when not set to 1 (again)
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
		//swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		//swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		//swapChainDesc.SampleDesc.Quality = 0;
		//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		//swapChainDesc.Flags = 0;

	//Get the handle HWND from the SDL backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	//Create SwapChain and hook it into the handle of the SDL window
	result = m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	if (FAILED(result))
		return result;

	//Create the Depth/Stencil Buffer and View
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
		//depthStencilDesc.SampleDesc.Quality = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDepthStencilBuffer);
	if (FAILED(result))
		return result;

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
		return result;

	//Create the RenderTargetView
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result))
		return result;

	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, 0, &m_pRenderTargetView);
	if (FAILED(result))
		return result;

	//Bind the Views to the Output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//Set the Viewport
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(m_Width);
	viewPort.Height = static_cast<float>(m_Height);
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	viewPort. MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);

	return 0;
}

bool Elite::Renderer::PixelInTri(float col, float row, const Elite::FPoint4& v0, const Elite::FPoint4& v1, const Elite::FPoint4& v2, float& weight0, float& weight1, float& weight2, BaseEffect::Culling cullMode) const
{
	Elite::FVector2 currentEdge;
	Elite::FVector2 vectorToPoint;
	Elite::FPoint2 currentPixel{ col, row };

	currentEdge = v1.xy - v0.xy;
	vectorToPoint = currentPixel - v0.xy;
	weight2 = Elite::Cross(currentEdge, vectorToPoint);

	currentEdge = v2.xy - v1.xy;
	vectorToPoint = currentPixel - v1.xy;
	weight0 = Elite::Cross(currentEdge, vectorToPoint);

	currentEdge = v0.xy - v2.xy;
	vectorToPoint = currentPixel - v2.xy;
	weight1 = Elite::Cross(currentEdge, vectorToPoint);

	switch (cullMode)
	{
	case BaseEffect::Culling::Back:
		if (weight0 > 0 || weight1 > 0 || weight2 > 0)
			return false;
		break;
	case BaseEffect::Culling::Front:
		if (weight0 < 0 || weight1 < 0 || weight2 < 0)
			return false;
		break;
	case BaseEffect::Culling::None:
		if (!(weight0 > 0 && weight1 > 0 && weight2 > 0))
			if (!(weight0 < 0 && weight1 < 0 && weight2 < 0))
				return false;
		break;
	default:
		break;
	}
	

	return true;
}

void Elite::Renderer::CalculateDepthBuffer(float& depth, float w0, float w1, float w2, int i0, int i1, int i2, const std::vector<OutputVertex>& vertices) const
{
	depth = (1 / vertices[i0].Position.z) * w0;
	depth += (1 / vertices[i1].Position.z) * w1;
	depth += (1 / vertices[i2].Position.z) * w2;
	depth = 1 / depth;
}
void Elite::Renderer::CalculateDepthInterpolated(float& depth, float w0, float w1, float w2, int i0, int i1, int i2, const std::vector<OutputVertex>& vertices) const
{
	depth = (1 / vertices[i0].Position.w) * w0;
	depth += (1 / vertices[i1].Position.w) * w1;
	depth += (1 / vertices[i2].Position.w) * w2;
	depth = 1 / depth;
}

void Elite::Renderer::CalculateUV(Elite::FVector2& finalUV, float depth, float w0, float w1, float w2, int i0, int i1, int i2, const std::vector<OutputVertex>& vertices) const
{
	finalUV = (vertices[i0].UV / vertices[i0].Position.w) * w0;
	finalUV += (vertices[i1].UV / vertices[i1].Position.w) * w1;
	finalUV += (vertices[i2].UV / vertices[i2].Position.w) * w2;
	finalUV *= depth;
}

void Elite::Renderer::CalculateNormal(Elite::FVector3& finalNormal, float depth, float w0, float w1, float w2, int i0, int i1, int i2, const std::vector<OutputVertex>& vertices) const
{
	finalNormal = (vertices[i0].Normal / vertices[i0].Position.w) * w0;
	finalNormal += (vertices[i1].Normal / vertices[i1].Position.w) * w1;
	finalNormal += (vertices[i2].Normal / vertices[i2].Position.w) * w2;
	finalNormal *= depth;
	Elite::Normalize(finalNormal);
}

void Elite::Renderer::CalculateTangent(Elite::FVector3& finalTangent, float depth, float w0, float w1, float w2, int i0, int i1, int i2, const std::vector<OutputVertex>& vertices) const
{
	finalTangent = (vertices[i0].Tangent / vertices[i0].Position.w) * w0;
	finalTangent += (vertices[i1].Tangent / vertices[i1].Position.w) * w1;
	finalTangent += (vertices[i2].Tangent / vertices[i2].Position.w) * w2;
	finalTangent *= depth;
	Elite::Normalize(finalTangent);
}

void Elite::Renderer::CalculateViewDirection(Elite::FVector3& finalViewDirection, float depth, float w0, float w1, float w2, int i0, int i1, int i2, const std::vector<OutputVertex>& vertices) const
{
	finalViewDirection = (vertices[i0].ViewDirection / vertices[i0].Position.w) * w0;
	finalViewDirection += (vertices[i1].ViewDirection / vertices[i1].Position.w) * w1;
	finalViewDirection += (vertices[i2].ViewDirection / vertices[i2].Position.w) * w2;
	finalViewDirection *= depth;
}

void Elite::Renderer::CalculateColor(Elite::RGBColor& finalColor, float depth, float w0, float w1, float w2, int i0, int i1, int i2, const std::vector<OutputVertex>& vertices) const
{
	finalColor = (vertices[i0].Color / vertices[i0].Position.w) * w0;
	finalColor += (vertices[i1].Color / vertices[i1].Position.w) * w1;
	finalColor += (vertices[i2].Color / vertices[i2].Position.w) * w2;
	finalColor *= depth;
}
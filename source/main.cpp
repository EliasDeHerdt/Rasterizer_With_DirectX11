#include "pch.h"
//#undef main

//Standard includes
#include <iostream>

//Project includes
#include "ETimer.h"
#include "ERenderer.h"
#include "Structs.h"
#include "SceneGraph.h"
#include "CameraManager.h"
#include "EffectManager.h"
#include "ObjParser.h"

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

void CheckMouseInputs() {
	Elite::IPoint2 mousePos;
	const Uint32 mouseState = SDL_GetRelativeMouseState(&mousePos.x, &mousePos.y);
	if (mousePos != Elite::IPoint2{ 0, 0 }) {

		if (mouseState & SDL_BUTTON(1) && mouseState & SDL_BUTTON(3)) {

			CameraManager::GetInstance()->GetActiveCamera()->MoveCamera(Elite::FVector3{ 0.f, float(mousePos.y), 0.f });
		}
		else if (mouseState & SDL_BUTTON(1)) {

			CameraManager::GetInstance()->GetActiveCamera()->MoveCamera(Elite::FVector3{ -float(mousePos.x), 0.f, float(mousePos.y) });
		}
		else if (mouseState & SDL_BUTTON(3)) {

			CameraManager::GetInstance()->GetActiveCamera()->RotateCamera(Elite::ToRadians(float(mousePos.x)), Elite::ToRadians(float(mousePos.y)));
		}
	}
}

void PrintStartUpInformation() {

	std::cout << "-------Rasterizer and DirectX Combo-------\n";
	std::cout << "Framework by Matthieu Delaere\n";
	std::cout << "App by Elias De Herdt\n";
	std::cout << "Class 2DAE07 - Graphics Programming\n";
	std::cout << "Controls:\n";
	std::cout << "Left Mouse Button: Move the camera left, right, forwards or backwards\n";
	std::cout << "Right Mouse Button: Rotate the camera\n";
	std::cout << "Left & Right Mouse Button: Move the camera up or down\n";
	std::cout << "R: Swap render mode (DirectX or Rasterizer)\n";
	std::cout << "T: Toggle transparency of flames (DirectX only)\n";
	std::cout << "D: Toggle depth rendering (Rasterizer only)\n";
	std::cout << "F: Toggle sampling mode (Point, Linear, Anisotropic) (DirectX only)\n";
	std::cout << "X: Toggle rendering of effects (DirectX or Rasterizer)\n";
	std::cout << "C: Toggle cullmode (Back, Front, None)\n";
	std::cout << "-----------------------------------------\n";
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;
	SDL_Window* pWindow = SDL_CreateWindow(
		"DirectX - **Elias De Herdt**",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	PrintStartUpInformation();

	//Initialize "framework"
	auto pTimer{ std::make_unique<Elite::Timer>() };
	auto pRenderer{ std::make_unique<Elite::Renderer>(pWindow) };
	ID3D11Device* pDevice = pRenderer->GetDevice();

	try {
		bool rotate = true;

		//Add Camera
		CameraManager::GetInstance()->AddNewCamera(new Camera{ { 0.f, 5.f, 35.f }, {0.f, 0.f, 1.f}, width, height });

		//Create Effect
		EffectManager::GetInstance()->AddEffect("VehicleEffect", new MaterialEffect{pDevice, L"Resources/PosCol3D.fx" });
		EffectManager::GetInstance()->AddEffect("FlameEffect", new FlatEffect{pDevice, L"Resources/Transparency.fx" });

		//Add Objects to SceneGraph
		auto readFile1 = ObjParser::GetInstance()->ReadObjFile("Resources/vehicle.obj");
		SceneGraph::GetInstance()->AddObjectToGraph(new Mesh{ rotate, {}, "Resources/vehicle_diffuse.png", "Resources/vehicle_normal.png", "Resources/vehicle_specular.png", "Resources/vehicle_gloss.png", pDevice, readFile1.first, readFile1.second, EffectManager::GetInstance()->GetEffect("VehicleEffect")});

		auto readFile2 = ObjParser::GetInstance()->ReadObjFile("Resources/fireFX.obj");
		SceneGraph::GetInstance()->AddObjectToGraph(new Mesh{ rotate, {}, "Resources/fireFX_diffuse.png", "", "", "", pDevice, readFile2.first, readFile2.second, EffectManager::GetInstance()->GetEffect("FlameEffect") });

	}
	catch (std::runtime_error e) {
		std::cout << e.what();
	}
	
	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;

	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if (e.key.keysym.scancode == SDL_SCANCODE_R) {

					SceneGraph::GetInstance()->ToggleRenderMode();
					CameraManager::GetInstance()->RecalculateCameras();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_T)
					EffectManager::GetInstance()->ToggleObjectTransparency();
				if (e.key.keysym.scancode == SDL_SCANCODE_D)
					pRenderer->ToggleDepthRendering();
				if (e.key.keysym.scancode == SDL_SCANCODE_F)
					EffectManager::GetInstance()->ToggleObjectPixelShading();
				if (e.key.keysym.scancode == SDL_SCANCODE_X)
					pRenderer->ToggleEffectRendering();
				if (e.key.keysym.scancode == SDL_SCANCODE_C)
					EffectManager::GetInstance()->ToggleObjectCulling();
				break;
			}
		}

		CheckMouseInputs();

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "FPS: " << pTimer->GetFPS() << std::endl;
		}

		//Update
		SceneGraph::GetInstance()->Update(pTimer->GetElapsed());

	}
	pTimer->Stop();
	delete SceneGraph::GetInstance();
	delete CameraManager::GetInstance();
	delete EffectManager::GetInstance();
	delete ObjParser::GetInstance();

	//Shutdown "framework"
	ShutDown(pWindow);
	return 0;
}
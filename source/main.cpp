#include "pch.h"
//#undef main

#ifdef _DEBUG
#include <vld.h>
#endif

//Standard includes
#include <iostream>

//Project includes
#include "ETimer.h"
#include "ERenderer.h"

//My includes
#include "ECamera.h"

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

void DisplayControls()
{
	using std::cout;
	using std::endl;
	cout << "Controls:\n\tSwitch Renderer: E\n\tSwitch CullMode: C\n\tSwitch SampleFilter: F\n\tToggle Rotation: R\n\tToggle FireFX (DirectX only): T" << endl;
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
		"DualRasterizer - Vanderveken Arnaud",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize My Classes
	auto pCamera{ std::make_unique<Elite::Camera>((float)width, (float)height) };

	//Initialize "framework"
	auto pTimer{ std::make_unique<Elite::Timer>() };
	auto pRenderer{ std::make_unique<Elite::Renderer>(pWindow, pCamera.get()) };

	DisplayControls();

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
					switch (e.key.keysym.scancode)
					{
					case SDL_SCANCODE_E:
						pRenderer->SwitchRenderMode();
						break;
					case SDL_SCANCODE_C:
						pRenderer->SwitchCullMode();
						break;
					case SDL_SCANCODE_F:
						pRenderer->SwitchSampleFilter();
						break;
					case SDL_SCANCODE_R:
						pRenderer->ToggleRotating();
						break;
					case SDL_SCANCODE_T:
						pRenderer->ToggleFireFX();
						break;
					}
					break;
			}
		}

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "FPS: " << pTimer->GetFPS() << std::endl;
		}

		//--------- Update ---------
		pCamera->Update(pTimer->GetElapsed());
		pRenderer->Update(pTimer->GetElapsed());

		//--------- Render ---------
		pRenderer->Render();

	}
	pTimer->Stop();

	//Shutdown "framework"
	ShutDown(pWindow);
	return 0;
}
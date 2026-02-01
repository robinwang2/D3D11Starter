#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
// Adjust as necessary for your own folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
	: backgroundColor{ 0.4f, 0.6f, 0.75f, 1.0f },
	  showDemoWindow(true),
	  testSliderValue(0.5f),
	  testCounter(0),
	  testText{},
	  triangleMesh(nullptr),
	  squareMesh(nullptr),
	  hexagonMesh(nullptr),
	  trianglePosition(0.0f, 1.0f, 0.0f),    // Top center
	  squarePosition(-0.5f, -0.3f, 0.0f),   // Bottom left
	  hexagonPosition(0.5f, -0.3f, 0.0f)    // Bottom right
{
	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.

	LoadShaders();
	CreateGeometry();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
	}
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// Clean up our mesh objects
	delete triangleMesh;
	delete squareMesh;
	delete hexagonMesh;

	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 cyan = XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 magenta = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	// --- MESH 1: Triangle ---
	Vertex triangleVertices[] =
	{
		{ XMFLOAT3(trianglePosition.x, trianglePosition.y, trianglePosition.z), red },
		{ XMFLOAT3(trianglePosition.x + 0.5f, trianglePosition.y - 0.5f, trianglePosition.z), blue },
		{ XMFLOAT3(trianglePosition.x - 0.5f, trianglePosition.y - 0.5f, trianglePosition.z), green },
	};
	unsigned int triangleIndices[] = { 0, 1, 2 };

	triangleMesh = new Mesh(triangleVertices, 3, triangleIndices, 3);

	// --- MESH 2: Square (made of 2 triangles) ---
	Vertex squareVertices[] =
	{
		{ XMFLOAT3(squarePosition.x - 0.3f, squarePosition.y + 0.3f, squarePosition.z), red },    // Top-left
		{ XMFLOAT3(squarePosition.x + 0.3f, squarePosition.y + 0.3f, squarePosition.z), green },  // Top-right
		{ XMFLOAT3(squarePosition.x + 0.3f, squarePosition.y - 0.3f, squarePosition.z), blue },   // Bottom-right
		{ XMFLOAT3(squarePosition.x - 0.3f, squarePosition.y - 0.3f, squarePosition.z), yellow }, // Bottom-left
	};
	unsigned int squareIndices[] = { 
		0, 1, 2,  // First triangle (top-right half)
		0, 2, 3   // Second triangle (bottom-left half)
	};

	squareMesh = new Mesh(squareVertices, 4, squareIndices, 6);

	// --- MESH 3: Hexagon (made of 6 triangles radiating from center) ---
	Vertex hexagonVertices[] =
	{
		{ XMFLOAT3(hexagonPosition.x + 0.0f, hexagonPosition.y + 0.0f, hexagonPosition.z + 0.0f), white },   // Center (index 0)
		{ XMFLOAT3(hexagonPosition.x + 0.0f, hexagonPosition.y + 0.4f, hexagonPosition.z + 0.0f), black },     // Top (index 1)
		{ XMFLOAT3(hexagonPosition.x + 0.35f, hexagonPosition.y + 0.2f, hexagonPosition.z + 0.0f), black }, // Top-right (index 2)
		{ XMFLOAT3(hexagonPosition.x + 0.35f, hexagonPosition.y - 0.2f, hexagonPosition.z + 0.0f), black },  // Bottom-right (index 3)
		{ XMFLOAT3(hexagonPosition.x + 0.0f, hexagonPosition.y - 0.4f, hexagonPosition.z + 0.0f), black },    // Bottom (index 4)
		{ XMFLOAT3(hexagonPosition.x - 0.35f, hexagonPosition.y - 0.2f, hexagonPosition.z + 0.0f), black },   // Bottom-left (index 5)
		{ XMFLOAT3(hexagonPosition.x - 0.35f, hexagonPosition.y + 0.2f, hexagonPosition.z + 0.0f), black },// Top-left (index 6)
	};
	unsigned int hexagonIndices[] = {
		0, 1, 2,  // Triangle 1: center to top to top-right
		0, 2, 3,  // Triangle 2: center to top-right to bottom-right
		0, 3, 4,  // Triangle 3: center to bottom-right to bottom
		0, 4, 5,  // Triangle 4: center to bottom to bottom-left
		0, 5, 6,  // Triangle 5: center to bottom-left to top-left
		0, 6, 1   // Triangle 6: center to top-left to top
	};

	hexagonMesh = new Mesh(hexagonVertices, 7, hexagonIndices, 18);
}

// --------------------------------------------------------
// ImGui per-frame helper (call at the start of Update())
// --------------------------------------------------------
void Game::BeginImGuiFrame(float deltaTime)
{
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);

	// Show the demo window if enabled
	if (showDemoWindow)
	{
		ImGui::ShowDemoWindow();
	}
}

// --------------------------------------------------------
// Builds the custom ImGui UI
// --------------------------------------------------------
void Game::BuildUI()
{
	// Create a custom window
	ImGui::Begin("Game Settings");

	// Display framerate
	ImGui::Text("Framerate: %.1f FPS", ImGui::GetIO().Framerate);

	// Display window dimensions
	ImGui::Text("Window Size: %d x %d", Window::Width(), Window::Height());

	// Background color picker
	ImGui::ColorEdit4("Background Color", backgroundColor);

	// Button to toggle demo window
	if (ImGui::Button(showDemoWindow ? "Hide Demo Window" : "Show Demo Window"))
	{
		showDemoWindow = !showDemoWindow;
	}

	// Separator for visual organization
	ImGui::Separator();
	ImGui::Text("Mesh Information:");

	// Display mesh information
	if (triangleMesh)
		ImGui::Text("Triangle - Vertices: %d, Indices: %d", 
			triangleMesh->GetVertexCount(), triangleMesh->GetIndexCount());
	if (squareMesh)
		ImGui::Text("Square - Vertices: %d, Indices: %d", 
			squareMesh->GetVertexCount(), squareMesh->GetIndexCount());
	if (hexagonMesh)
		ImGui::Text("Hexagon - Vertices: %d, Indices: %d", 
			hexagonMesh->GetVertexCount(), hexagonMesh->GetIndexCount());

	// Separator for visual organization
	ImGui::Separator();
	ImGui::Text("Additional Test Elements:");

	// Test Element 1: Collapsing Header with content
	if (ImGui::CollapsingHeader("Collapsing Headers"))
	{
		ImGui::Text("This is in a collapsing header!");
		ImGui::BulletText("1");
		ImGui::BulletText("2");
	}

	// Test Element 2: Text Input
	ImGui::InputText("Text Input", testText, 256);

	// Test Element 3: Slider
	ImGui::SliderFloat("Test Slider", &testSliderValue, 0.0f, 1.0f);

	// Test Element 4: Counter with buttons
	ImGui::Text("Counter: %d", testCounter);
	if (ImGui::Button("Increment"))
		testCounter++;
	ImGui::SameLine();
	if (ImGui::Button("Decrement"))
		testCounter--;
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
		testCounter = 0;

	ImGui::End();
}

// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// BEGIN: ImGui new frame setup - MUST be first!
	BeginImGuiFrame(deltaTime);

	// Build the custom UI
	BuildUI();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		// Use the backgroundColor member variable instead of local const
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(), backgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry
	// - Each mesh's Draw() method handles setting buffers and drawing
	{
		triangleMesh->Draw();
		squareMesh->Draw();
		hexagonMesh->Draw();
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		ImGui::Render(); // Turns this frame's UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}

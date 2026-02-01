#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

class Mesh; // Forward declaration

class Game
{
public:
	// Basic OOP setup
	Game();
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateGeometry();

	void BeginImGuiFrame(float deltaTime);

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// Mesh objects
	Mesh* triangleMesh;
	Mesh* squareMesh;
	Mesh* hexagonMesh;

	// Mesh positions
	DirectX::XMFLOAT3 trianglePosition;
	DirectX::XMFLOAT3 squarePosition;
	DirectX::XMFLOAT3 hexagonPosition;

	// ImGui UI state variables
	float backgroundColor[4];  // Background color (RGBA)
	bool showDemoWindow;       // Toggle for demo window visibility
	
	// Test UI elements
	float testSliderValue;     // Slider value
	int testCounter;           // Counter value
	char testText[256];        // Text input buffer

	// Helper methods
	void BuildUI();            // Builds custom ImGui interface
};


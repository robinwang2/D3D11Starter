#pragma once

#include <d3d11.h>
#include <wrl/client.h>

class Mesh
{
private:
	// Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Counts
	int indexCount;
	int vertexCount;
public:
	// Constructor & Destructor
	Mesh(class Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount);
	~Mesh();

	// Getters
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetIndexCount();
	int GetVertexCount();

	// Drawing
	void Draw();
};
// Constant buffer for external data from C++ application
// - Must match the VertexShaderExternalData struct in BufferStructs.h
// - Register b0 means this is bound to slot 0
cbuffer ExternalData : register(b0)
{
	float4 colorTint;   // Color tint to multiply with vertex colors
	float3 offset;      // Offset to apply to vertex positions
	float padding;      // Padding for 16-byte alignment (not used in shader)
}

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{ 
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 localPosition	: POSITION;     // XYZ position
	float4 color			: COLOR;        // RGBA color
};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;	// XYZW position (System Value Position)
	float4 color			: COLOR;        // RGBA color
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// Apply the offset from the constant buffer to the vertex position
	// - This shifts all vertices by the same amount
	float3 offsetPosition = input.localPosition + offset;
	
	// Convert to float4 for output (homogeneous coordinates)
	// - To be considered within the bounds of the screen, the X and Y components 
	//   must be between -1 and 1.  
	// - The Z component must be between 0 and 1.  
	// - Each of these components is then automatically divided by the W component, 
	//   which we're leaving at 1.0 for now (this is more useful when dealing with 
	//   a perspective projection matrix, which we'll get to in the future).
	output.screenPosition = float4(offsetPosition, 1.0f);

	// Apply the color tint from the constant buffer to the vertex color
	// - Multiplying colors reduces their intensity (tints them)
	// - For example, a colorTint of (1.0, 0.8, 0.8, 1.0) will slightly reduce 
	//   blue and green, creating a reddish tint
	output.color = input.color * colorTint;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}
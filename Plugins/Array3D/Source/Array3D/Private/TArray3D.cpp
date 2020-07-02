#include "TArray3D.h"

namespace Array3D {
	int32 GetIndex(int32 DX, int32 DY, int32 DZ, int32 X, int32 Y, int32 Z)
	{
		ValidateDimensions(DX, DY, DZ);
		IndexWithinDimensions(DX, DY, DZ, X, Y, Z);
		
		return (DX * DY * Z) + (DX*Y) + (X);
	}
	int32 GetIndex(FIntVector Dimensions, int32 X, int32 Y, int32 Z)
	{
		return GetIndex(Dimensions.X, Dimensions.Y, Dimensions.Z, X, Y, Z);
	}
	int32 GetIndex(FIntVector Dimensions, FIntVector XYZ)
	{
		return GetIndex(Dimensions.X, Dimensions.Y, Dimensions.Z, XYZ.X, XYZ.Y, XYZ.Z);
	}

	FIntVector GetXYZ(int32 DX, int32 DY, int32 DZ, int32 I)
	{
		ValidateDimensions(DX, DY, DZ);
		IndexWithinDimensions(DX, DY, DZ, I);
	
		FIntVector XYZ;

		XYZ.Z = I / (DX * DY); // Find Z Index.
		I -= (DX * DY) * XYZ.Z; // Remove the Z dimension from the Index.

		XYZ.Y = I / DX; // Find Y Index.
		I -= DX * XYZ.Y; // Remove Y dimension from the Index.

		XYZ.X = I; // Remainder is X index.

		return XYZ;
	}
	FIntVector GetXYZ(FIntVector Dimensions, int32 I)
	{
		return GetXYZ(Dimensions.X, Dimensions.Y, Dimensions.Z, I);
	}

	int32 Size(int32 DX, int32 DY, int32 DZ)
	{
		return DX * DY * DZ; // Size is the dimensions multiplied together.
	}
	int32 Size(FIntVector Dimensions)
	{
		return Size(Dimensions.X, Dimensions.Y, Dimensions.Z);
	}

	void ValidateDimensions(int32 DX, int32 DY, int32 DZ)
	{
		checkf(DX > 0 && DY > 0 && DZ > 0, TEXT("Invalid dimensions, all dimensions need to be greater than 0. Dimensions:[%d, %d, %d]"), DX, DY, DZ);
	}

	void IndexWithinDimensions(int32 DX, int32 DY, int32 DZ, int32 X, int32 Y, int32 Z)
	{
		checkf(X >= 0 && X < DX && Y >= 0 && Y < DY && Z >= 0 && Z < DZ, TEXT("Index is not within dimensions. Dimensions:[%d, %d, %d] Index:[%d, %d, %d]"), DX, DY, DZ, X, Y, Z);
	}
	void IndexWithinDimensions(int32 DX, int32 DY, int32 DZ, int32 I)
	{
		checkf(I >= 0 && I < Size(DX, DY, DZ), TEXT("Index is not within the 1D bounds of the dimension. 1D bounds: 0 - %d Index: %d"), Size(DX,DY,DZ), I);
	}
}
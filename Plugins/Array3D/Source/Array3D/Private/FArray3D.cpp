#include "FArray3D.h"


int32 FArray3D::TranslateTo1D(int32 DX, int32 DY, int32 DZ, int32 X, int32 Y, int32 Z)
{
	verify(ValidateDimensions(DX, DY, DZ));
	verify(IndexWithinDimensions(DX, DY, DZ, X, Y, Z));

	return (DX * DY * Z) + (DX*Y) + (X);
}
int32 FArray3D::TranslateTo1D(FIntVector Dimensions, int32 X, int32 Y, int32 Z)
{
	return TranslateTo1D(Dimensions.X, Dimensions.Y, Dimensions.Z, X, Y, Z);
}
int32 FArray3D::TranslateTo1D(FIntVector Dimensions, FIntVector XYZ)
{
	return TranslateTo1D(Dimensions.X, Dimensions.Y, Dimensions.Z, XYZ.X, XYZ.Y, XYZ.Z);
}

FIntVector FArray3D::TranslateTo3D(int32 DX, int32 DY, int32 DZ, int32 I)
{
	verify(ValidateDimensions(DX, DY, DZ));
	verify(IndexWithinDimensions(DX, DY, DZ, I));
	
	FIntVector XYZ;

	XYZ.Z = I / (DX * DY); // Find Z Index.
	I -= (DX * DY) * XYZ.Z; // Remove the Z dimension from the Index.

	XYZ.Y = I / DX; // Find Y Index.
	I -= DX * XYZ.Y; // Remove Y dimension from the Index.

	XYZ.X = I; // Remainder is X index.

	return XYZ;
}
FIntVector FArray3D::TranslateTo3D(FIntVector Dimensions, int32 I)
{
	return TranslateTo3D(Dimensions.X, Dimensions.Y, Dimensions.Z, I);
}

int32 FArray3D::Size(int32 DX, int32 DY, int32 DZ)
{
	return DX * DY * DZ; // Size is the dimensions multiplied together.
}
int32 FArray3D::Size(FIntVector Dimensions)
{
	return Size(Dimensions.X, Dimensions.Y, Dimensions.Z);
}

bool FArray3D::ValidateDimensions(int32 DX, int32 DY, int32 DZ)
{
	return ensureAlwaysMsgf(DX > 0 && DY > 0 && DZ > 0, TEXT("Invalid dimensions, all dimensions need to be greater than 0. Dimensions:[%d, %d, %d]"), DX, DY, DZ);
}
bool FArray3D::ValidateDimensions(FIntVector Dimensions)
{
	return ValidateDimensions(Dimensions.X, Dimensions.Y, Dimensions.Z);
}

bool FArray3D::IndexWithinDimensions(int32 DX, int32 DY, int32 DZ, int32 X, int32 Y, int32 Z)
{
	return ensureAlwaysMsgf(X >= 0 && X < DX && Y >= 0 && Y < DY && Z >= 0 && Z < DZ, TEXT("Index is not within dimensions. Dimensions:[%d, %d, %d] Index:[%d, %d, %d]"), DX, DY, DZ, X, Y, Z);
}
bool FArray3D::IndexWithinDimensions(int32 DX, int32 DY, int32 DZ, int32 I)
{
	return ensureAlwaysMsgf(I >= 0 && I < Size(DX, DY, DZ), TEXT("Index is not within the 1D bounds of the dimension. 1D bounds: 0 - %d Index: %d"), Size(DX,DY,DZ), I);
}
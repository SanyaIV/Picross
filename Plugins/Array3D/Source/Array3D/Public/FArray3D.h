// Copyright Sanya Larsson 2020

#pragma once

#include "HAL/Platform.h"
#include "Math/IntVector.h"

class ARRAY3D_API FArray3D
{
public:
	FArray3D() = delete;

	/**
	 * Converts a 3D Index (X,Y,Z) into a 1D Index.
	 * @param DX - Size of the X-dimension.
	 * @param DY - Size of the Y-dimension.
	 * @param DZ - Size of the Z-dimension.
	 * @param X - Index in the X-dimension.
	 * @param Y - Index in the Y-dimension.
	 * @param Z - Index in the Z-dimension.
	 * @returns a 1D Index >= 0 and < Size(DX, DY, DZ).
	 */
	static int32 TranslateTo1D(int32 DX, int32 DY, int32 DZ, int32 X, int32 Y, int32 Z);
	/**
	 * Converts a 3D Index (X,Y,Z) into a 1D Index.
	 * @param Dimensions - Size of the dimensions.
	 * @param X - Index in the X-dimension.
	 * @param Y - Index in the Y-dimension.
	 * @param Z - Index in the Z-dimension.
	 * @returns a 1D Index >= 0 and < Size(Dimensions).
	 */
	static int32 TranslateTo1D(FIntVector Dimensions, int32 X, int32 Y, int32 Z);
	/**
	 * Converts a 3D Index (X,Y,Z) into a 1D Index.
	 * @param Dimensions - Size of the dimensions.
	 * @param XYZ - Index in the 3D-dimensions.
	 * @returns a 1D Index >= 0 and < Size(Dimensions).
	 */
	static int32 TranslateTo1D(FIntVector Dimensions, FIntVector XYZ);

	/**
	 * Converts a 1D Index into a 3D Index (X,Y,Z).
	 * @param DX - Size of the X-dimension.
	 * @param DY - Size of the Y-dimension.
	 * @param DZ - Size of the Z-dimension.
	 * @param I - 1D Index to convert to 3D Index.
	 * @returns a 3D Index within the dimensions set by DX, DY & DZ.
	 */
	static FIntVector TranslateTo3D(int32 DX, int32 DY, int32 DZ, int32 I);
	/**
	 * Converts a 1D Index into a 3D Index (X,Y,Z).
	 * @param Dimensions - Size of the dimensions.
	 * @param I - 1D Index to convert to 3D Index.
	 * @returns a 3D Index within Dimensions.
	 */
	static FIntVector TranslateTo3D(FIntVector Dimensions, int32 I);

	/**
	 * Gets the Size of the 3D array as in the number of cells/elements.
	 * @param DX - Size of the X-dimension.
	 * @param DY - Size of the Y-dimension.
	 * @param DZ - Size of the Z-dimension.
	 * @returns DX*DY*DZ.
	 */
	static int32 Size(int32 DX, int32 DY, int32 DZ);
	/**
	 * Gets the Size of the 3D array as in the number of cells/elements.
	 * @param Dimensions - Size of the dimensions.
	 * @returns the product of all dimensions.
	 */
	static int32 Size(FIntVector Dimensions);

	/**
	 * Validates the given dimensions, making sure they are all larger than 0.
	 * @param DX - Size of the X-dimension.
	 * @param DY - Size of the Y-dimension.
	 * @param DZ - Size of the Z-dimension.
	 * @returns true if dimensions are valid otherwise false.
	 */
	static bool ValidateDimensions(int32 DX, int32 DY, int32 DZ);
	/**
	 * Validates the given dimensions, making sure X, Y & Z are all larger than 0.
	 * @param Dimensions - The dimensions (X,Y,Z) to check.
	 * @returns true if dimensions are valid otherwise false.
	 */
	static bool ValidateDimensions(FIntVector Dimensions);

	/**
	 * Makes sure the given index is within the dimensions.
	 * @param DX - Size of the X-dimension.
	 * @param DY - Size of the Y-dimension.
	 * @param DZ - Size of the Z-dimension.
	 * @param X - Index in the X-dimension.
	 * @param Y - Index in the Y-dimension.
	 * @param Z - Index in the Z-dimension.
	 * @returns true if index is within the dimensions otherwise false.
	 */
	static bool IndexWithinDimensions(int32 DX, int32 DY, int32 DZ, int32 X, int32 Y, int32 Z);
	/**
	 * Makes sure the given index is within the 1D bounds of the dimension.
	 * @param DX - Size of the X-dimension.
	 * @param DY - Size of the Y-dimension.
	 * @param DZ - Size of the Z-dimension.
	 * @param I - 1D Index.
	 * @returns true if index is within the dimensions otherwise false.
	 */
	static bool IndexWithinDimensions(int32 DX, int32 DY, int32 DZ, int32 I);
	
};

// Copyright Sanya Larsson 2020

#pragma once

#include "HAL/Platform.h"
#include "Math/IntVector.h"

namespace Array3D
{
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
	int32 GetIndex(int32 DX, int32 DY, int32 DZ, int32 X, int32 Y, int32 Z);
	/**
	 * Converts a 3D Index (X,Y,Z) into a 1D Index.
	 * @param Dimensions - Size of the dimensions.
	 * @param X - Index in the X-dimension.
	 * @param Y - Index in the Y-dimension.
	 * @param Z - Index in the Z-dimension.
	 * @returns a 1D Index >= 0 and < Size(Dimensions).
	 */
	int32 GetIndex(FIntVector Dimensions, int32 X, int32 Y, int32 Z);
	/**
	 * Converts a 3D Index (X,Y,Z) into a 1D Index.
	 * @param Dimensions - Size of the dimensions.
	 * @param XYZ - Index in the 3D-dimensions.
	 * @returns a 1D Index >= 0 and < Size(Dimensions).
	 */
	int32 GetIndex(FIntVector Dimensions, FIntVector XYZ);

	/**
	 * Converts a 1D Index into a 3D Index (X,Y,Z).
	 * @param DX - Size of the X-dimension.
	 * @param DY - Size of the Y-dimension.
	 * @param DZ - Size of the Z-dimension.
	 * @param I - 1D Index to convert to 3D Index.
	 * @returns a 3D Index within the dimensions set by DX, DY & DZ.
	 */
	FIntVector GetXYZ(int32 DX, int32 DY, int32 DZ, int32 I);
	/**
	 * Converts a 1D Index into a 3D Index (X,Y,Z).
	 * @param Dimensions - Size of the dimensions.
	 * @param I - 1D Index to convert to 3D Index.
	 * @returns a 3D Index within Dimensions.
	 */
	FIntVector GetXYZ(FIntVector Dimensions, int32 I);

	/**
	 * Gets the Size of the 3D array as in the number of cells/elements.
	 * @param DX - Size of the X-dimension.
	 * @param DY - Size of the Y-dimension.
	 * @param DZ - Size of the Z-dimension.
	 * @returns DX*DY*DZ.
	 */
	int32 Size(int32 DX, int32 DY, int32 DZ);
	/**
	 * Gets the Size of the 3D array as in the number of cells/elements.
	 * @param Dimensions - Size of the dimensions.
	 * @returns the product of all dimensions.
	 */
	int32 Size(FIntVector Dimensions);

	/**
	 * Validates the given dimensions, making sure they are all larger than 0.
	 * @param DX - Size of the X-dimension.
	 * @param DY - Size of the Y-dimension.
	 * @param DZ - Size of the Z-dimension.
	 */
	void ValidateDimensions(int32 DX, int32 DY, int32 DZ);

	/**
	 * Makes sure the given index is within the dimensions.
	 * @param DX - Size of the X-dimension.
	 * @param DY - Size of the Y-dimension.
	 * @param DZ - Size of the Z-dimension.
	 * @param X - Index in the X-dimension.
	 * @param Y - Index in the Y-dimension.
	 * @param Z - Index in the Z-dimension.
	 */
	void IndexWithinDimensions(int32 DX, int32 DY, int32 DZ, int32 X, int32 Y, int32 Z);
	/**
	 * Makes sure the given index is within the 1D bounds of the dimension.
	 * @param DX - Size of the X-dimension.
	 * @param DY - Size of the Y-dimension.
	 * @param DZ - Size of the Z-dimension.
	 * @param I - 1D Index.
	 */
	void IndexWithinDimensions(int32 DX, int32 DY, int32 DZ, int32 I);
}

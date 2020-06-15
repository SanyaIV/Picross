// Copyright Sanya Larsson 2020

#pragma once

#include "CoreTypes.h"
#include "Templates/UnrealTypeTraits.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Containers/Array.h"
#include "CoreGlobals.h"

/**
 * A templated static three dimensional specialisation of the TArray class template. 
 *
 * Super-imposes X, Y and Z dimensions on the single-dimensional TArray, accessible by functions, not brackets. 
 * Follows C++ row-major meaning implemention is similar to [Z][Y][X]. 
 * Nest X loop within Y loop within Z loop to be the least cache unfriendly.
 *
 * Caution: The dimensions are const and can't be changed after creation.
 * Caution: Since the size is assumed to be constant, calling any non-const functions from the TArray class can corrupt the container.
 * Caution: Memory will leak if instance is deleted through TArray instead of TArray3D.
 *
 **/
template<typename T>
class TArray3D
{

public:
	/** Delete default constructor */
	TArray3D() = delete;

	/**
	 * Constructor
	 * @param Dimensions - The dimensions for the 3D array in X, Y and Z.
	 *
	 * Caution - Will set dimensions to 1 if less than 1 is supplied.
	 */
	TArray3D(FIntVector Dimensions) : Array(), Dimensions(ValidateDimensions(Dimensions))
	{
		ProdXY = Dimensions.X * Dimensions.Y;
		DimensionsProduct = ProdXY * Dimensions.Z;
		Array.SetNumZeroed(DimensionsProduct);
	}

	/**
	 * Constructor
	 *
	 * @param X - The X-dimension of the 3D array.
	 * @param Y - The Y-dimension of the 3D array.
	 * @param Z - The Z-dimension of the 3D array.
	 * 
	 * Caution - Will set dimensions to 1 if less than 1 is supplied.
	 */
	TArray3D(int32 X, int32 Y, int32 Z) : TArray3D(FIntVector(X,Y,Z)) {}

	/** Copy constructor and assignment. */
	FORCEINLINE TArray3D(const TArray3D<T>& Other) : Array(Other.Array), Dimensions(Other.Dimensions), ProdXY(Other.ProdXY), DimensionsProduct(Other.DimensionsProduct){}
	TArray3D<T>& operator=(const TArray3D<T>& Other)
	{
		Array = Other.Array;
		Dimensions = Other.Dimensions;
		ProdXY = Other.ProdXY;
		DimensionsProduct = Other.DimensionsProduct;
	}

	/** Default destructor. */
	~TArray3D() = default;

public:
	/**
	* Array bracket operator. Returns reference to element at given 1D index.
	*
	* @param Index - 1D array index.
	* @returns Reference to indexed element.
	*/
	FORCEINLINE T& operator[](int32 Index) const
	{
		return Array[Index];
	}

	/**
	 * Array bracket operator. Returns reference to const element at given 1D index.
	 *
	 * @param Index - 1D array index.
	 * @returns Reference to indexed element as const.
	 */
	FORCEINLINE const T& operator[](int32 Index) const
	{
		return Array[Index];
	}

	/**
	 * Array bracket operator. Returns reference to element at given 3D index.
	 *
	 * @param Index - 3D array index.
	 * @returns Reference to indexed element.
	 */
	FORCEINLINE T& operator[](FIntVector Index) const
	{
		DimensionCheck(Index);
		return Array[GetIndexFromXYZ(Index)];
	}

	/**
	 * Array bracket operator. Returns reference to const element at given 3D index.
	 *
	 * @param Index - 3D array index.
	 * @returns Reference to indexed element as const.
	 */
	FORCEINLINE const T& operator[](FIntVector Index) const
	{
		DimensionCheck(Index);
		return Array[GetIndexFromXYZ(Index)];
	}

private:
	/**
	 * Gets the 1D array index from a 3D array index.
	 *
	 * @param XYZ - An FIntVector with the X, Y and Z Index.
	 * @returns The 1D array index or -1 (see Caution).
	 *
	 * Caution - If the 3D array index is out of the 3D dimensions, -1 is returned.
	 */
	FORCEINLINE int32 GetIndexFromXYZ(FIntVector XYZ) const
	{
		if (IndexWithinDimensions(Index))
		{
			return ProdXY * XYZ.Z + Dimensions.X * XYZ.Y + XYZ.X;
		}
		
		return -1;
	}

	/**
	 * Checks if the given 3D index is within the dimensions of the 3D Array.
	 *
	 * @param Index - 3D index to check.
	 * @returns true if within the dimensions, otherwise false.
	 */
	FORCEINLINE bool IndexWithinDimensions(FIntVector Index) const
	{
		return	Index.X >= 0 && Index.X <= Dimensions.X &&
				Index.Y >= 0 && Index.Y <= Dimensions.Y &&
				Index.Z >= 0 && Index.Y <= Dimensions.Z;
	}
	
	/**
	 * Checks and returns valid dimensions.
	 *
	 * @param Dimensions - The dimensiosn to validate.
	 * @returns valid dimensions.
	 */
	FORCEINLINE static FIntVector ValidateDimensions(FIntVector Dimensions)
	{
		return (Dimensions.X > 0 ? Dimensions.X : 1,
				Dimensions.Y > 0 ? Dimensions.Y : 1,
				Dimensions.Y > 0 ? Dimensions.Z : 1);
	}
	
	/**
	 * Check if Index is within dimensions, halting execution if fail.
	 *
	 * @param Index - The 3D index to check.
	 */
	FORCEINLINE void DimensionCheck(FIntVector Index) const
	{
		checkf(IndexWithinDimensions(Index), TEXT("Index out of bounds: %s out of %s"), *Index.ToString(), *Dimensions.ToString());
	}

private:
	TArray<T> Array;				// The underlying 1D array that we use.
	FIntVector Dimensions;			// The dimensions of the 3D array.
	int32 ProdXY;					// The product of X and Y dimensions.
};

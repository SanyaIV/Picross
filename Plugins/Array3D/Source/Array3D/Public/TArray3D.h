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
class TArray3D : TArray<T>
{

public:
	/** Default constructor. */
	FORCEINLINE TArray3D() : TArray<T>(), _Dimensions(FIntVector::ZeroValue), _XY(0), _XYZ(0){}

	/**
	* Constructor
	* @param Dimensions - The dimensions for the 3D array in X, Y and Z.
	*/
	TArray3D(FIntVector Dimensions) : TArray<T>(), _Dimensions(Dimensions)
	{
		_XY = _Dimensions.X * _Dimensions.Y;
		_XYZ = _XY * _Dimensions.Z;
		TArray<T>::SetNumZeroed(_XYZ);
	}

	/** Copy constructor and assignment. */
	FORCEINLINE TArray3D(const TArray3D<T>& Other) : TArray<T>(Other), _Dimensions(Other._Dimensions), _XY(Other._XY), _XYZ(Other._XYZ){}
	TArray3D<T>& operator=(const TArray3D<T>& Other)
	{
		TArray<T>::operator=(Other);
		_Dimensions = Other._Dimensions;
		_XY = Other._XY;
		_XYZ = Other._XYZ;
	}

	/** Move constructor and assignment. */
	FORCEINLINE TArray3D(TArray3D<T>&& Other) : TArray<T>(MoveTemp(Other)), _Dimensions(MoveTemp(Other._Dimensions)), _XY(MoveTemp(Other._XY)), _XYZ(MoveTemp(Other._XYZ)) {}
	TArray3D<T>& operator=(TArray3D<T>&& Other)
	{
		TArray<T>::operator=(MoveTemp(Other));
		_Dimensions = MoveTemp(Other._Dimensions);
		_XY = MoveTemp(Other._XY);
		_XYZ = MoveTemp(Other._XYZ);
	}

	/** Default destructor. */
	virtual ~TArray3D() = default;

	/**
	* Array bracket operator. Returns reference to element at given index.
	*
	* @returns Reference to indexed element.
	*/
	FORCEINLINE T& operator[](SizeType Index)
	{
		return TArray<T>::operator[](Index);
	}

	/**
	 * Array bracket operator. Returns reference to element at give index.
	 *
	 * Const version of the above.
	 *
	 * @returns Reference to indexed element.
	 */
	FORCEINLINE const T& operator[](SizeType Index) const
	{
		return TArray<T>::operator[](Index);
	}

private:
	FIntVector _Dimensions;
	SizeType _XY;
	SizeType _XYZ;
};

// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "PicrossNumber.generated.h"

class UMaterialInstance;

USTRUCT(BlueprintType)
struct PICROSS_API FAxisPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EAxis::Type> Axis;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EAxis::Type> SelectionAxis;

	bool operator==(const FAxisPair& Other) const
	{
		return (Axis == Other.Axis && SelectionAxis == Other.SelectionAxis);
	}

	friend uint32 GetTypeHash(const FAxisPair& Other)
	{
		return GetTypeHash(Other.Axis) + GetTypeHash(Other.SelectionAxis);
	}
};

USTRUCT(BlueprintType)
struct PICROSS_API FTextPairData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FRotator MainRotation;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EHorizTextAligment> MainHorizontalAlignment;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EVerticalTextAligment> MainVerticalAlignment;

	UPROPERTY(EditAnywhere)
	FRotator ReversedRotation;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EHorizTextAligment> ReversedHorizontalAlignment;
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EVerticalTextAligment> ReversedVerticalAlignment;

	bool operator==(const FTextPairData& Other) const
	{
		return 
		(
			MainRotation == Other.MainRotation
			&& MainHorizontalAlignment == Other.MainHorizontalAlignment
			&& MainVerticalAlignment == Other.MainVerticalAlignment
			&& ReversedRotation == Other.ReversedRotation 
			&& ReversedHorizontalAlignment == Other.ReversedHorizontalAlignment 
			&& ReversedVerticalAlignment == Other.ReversedVerticalAlignment
		);
	}

	friend uint32 GetTypeHash(const FTextPairData& Other)
	{
		return GetTypeHash(Other.MainRotation.Vector())
			+ GetTypeHash(Other.MainHorizontalAlignment)
			+ GetTypeHash(Other.MainVerticalAlignment)
			+ GetTypeHash(Other.ReversedRotation.Vector())
			+ GetTypeHash(Other.ReversedHorizontalAlignment)
			+ GetTypeHash(Other.ReversedVerticalAlignment);
	}
};

UCLASS()
class PICROSS_API APicrossNumber : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APicrossNumber();

	void Setup(const EAxis::Type AxisToSet, const FFormatOrderedArguments& NumbersToSet);
	void UpdateRotation(const EAxis::Type GridSelectionAxis);
	EAxis::Type GetAxis() const { return Axis; }

protected:
	void GenerateTexts();

private:
	UPROPERTY(VisibleInstanceOnly)
	UTextRenderComponent* MainText = nullptr;
	UPROPERTY(VisibleInstanceOnly)
	UTextRenderComponent* ReversedText = nullptr;

	// Information about the pairs of data for different axis combinations.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotations", meta = (AllowPrivateAccess = "true"))
	TMap<FAxisPair, FTextPairData> TextPairDatas;

	UPROPERTY(EditAnywhere, Category = "Numbers", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* NumbersTextMaterial = nullptr;

	FFormatOrderedArguments Numbers;
	UPROPERTY(VisibleInstanceOnly)
	TEnumAsByte<EAxis::Type> Axis = EAxis::Type::None;
};

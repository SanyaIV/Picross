// Copyright Sanya Larsson 2020


#include "PicrossNumber.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstance.h"

// Sets default values
APicrossNumber::APicrossNumber()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	MainText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Main Text"));
	if (MainText)
	{
		MainText->SetupAttachment(GetRootComponent());
		if (NumbersTextMaterial)
		{
			MainText->SetMaterial(0, NumbersTextMaterial);
		}
	}
	
	ReversedText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Reversed Text"));
	if (ReversedText)
	{
		ReversedText->SetupAttachment(GetRootComponent());
		if (NumbersTextMaterial)
		{
			ReversedText->SetMaterial(0, NumbersTextMaterial);
		}
	}
}

void APicrossNumber::Setup(const EAxis::Type AxisToSet, const FFormatOrderedArguments& NumbersToSet)
{
	Axis = AxisToSet;
	Numbers = NumbersToSet;

	const FColor Color = Axis == EAxis::Z ? FColor::Blue : Axis == EAxis::Y ? FColor::Green : FColor::Red;
	MainText->SetTextRenderColor(Color);
	ReversedText->SetTextRenderColor(Color);
	UpdateRotation(EAxis::Type::None);
	GenerateTexts();
}

void APicrossNumber::UpdateRotation(const EAxis::Type GridSelectionAxis)
{
	const FAxisPair AxisPair{ Axis, GridSelectionAxis };
	
	if (TextPairDatas.Contains(AxisPair))
	{
		const EHorizTextAligment OldHorizontalAlignment = MainText->HorizontalAlignment;
		const EVerticalTextAligment OldVerticalAlignment = MainText->VerticalAlignment;

		const FTextPairData& TextPairData = TextPairDatas.FindChecked(AxisPair);
		MainText->SetRelativeRotation(TextPairData.MainRotation);
		MainText->SetHorizontalAlignment(TextPairData.MainHorizontalAlignment);
		MainText->SetVerticalAlignment(TextPairData.MainVerticalAlignment);

		ReversedText->SetRelativeRotation(TextPairData.ReversedRotation);
		ReversedText->SetHorizontalAlignment(TextPairData.ReversedHorizontalAlignment);
		ReversedText->SetVerticalAlignment(TextPairData.ReversedVerticalAlignment);

		if (OldHorizontalAlignment != MainText->HorizontalAlignment || OldVerticalAlignment != MainText->VerticalAlignment)
		{
			GenerateTexts();
		}
	}
}

void APicrossNumber::GenerateTexts()
{
	if (Numbers.Num() > 0)
	{
		if (MainText && ReversedText)
		{
			const bool bVerticalText = (MainText->VerticalAlignment == EVerticalTextAligment::EVRTA_TextBottom && MainText->HorizontalAlignment == EHorizTextAligment::EHTA_Center);
			const FText Delimiter = FText::FromString(bVerticalText ? TEXT("\n") : TEXT(", "));
			MainText->Text = FText::Join(Delimiter, Numbers);

			FFormatOrderedArguments ReversedNumbers = Numbers;
			Algo::Reverse(ReversedNumbers);
			ReversedText->Text = FText::Join(Delimiter, Numbers);
		}
	}
}

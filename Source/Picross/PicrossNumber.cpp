// Copyright Sanya Larsson 2020


#include "PicrossNumber.h"
#include "Components/TextRenderComponent.h"

// Sets default values
APicrossNumber::APicrossNumber()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	MainText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Main Text"));
	MainText->SetupAttachment(GetRootComponent());
	ReversedText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Reversed Text"));
	ReversedText->SetupAttachment(GetRootComponent());
}

void APicrossNumber::Setup(const EAxis::Type AxisToSet, const FFormatOrderedArguments& NumbersToSet)
{
	Axis = AxisToSet;
	Numbers = NumbersToSet;

	GenerateTexts();
	const FColor Color = Axis == EAxis::Type::Z ? FColor::Blue : Axis == EAxis::Type::Y ? FColor::Green : FColor::Red;
	MainText->SetTextRenderColor(Color);
	ReversedText->SetTextRenderColor(Color);
	UpdateRotation(EAxis::Type::None);
}

void APicrossNumber::GenerateTexts()
{
	if (Numbers.Num() > 0)
	{
		if (MainText && ReversedText)
		{
			const FText Delimiter = FText::FromString(Axis == EAxis::Type::Z ? TEXT("\n") : TEXT(", "));
			MainText->Text = FText::Join(Delimiter, Numbers);

			FFormatOrderedArguments ReversedNumbers = Numbers;
			Algo::Reverse(ReversedNumbers);
			ReversedText->Text = FText::Join(Delimiter, Numbers);
		}
	}
}

void APicrossNumber::UpdateRotation(const EAxis::Type GridSelectionAxis)
{
	const FAxisPair AxisPair{ Axis, GridSelectionAxis };
	
	if (TextPairDatas.Contains(AxisPair))
	{
		const FTextPairData& TextPairData = TextPairDatas.FindChecked(AxisPair);
		MainText->SetRelativeRotation(TextPairData.MainRotation);
		MainText->SetHorizontalAlignment(TextPairData.MainHorizontalAlignment);
		MainText->SetVerticalAlignment(TextPairData.MainVerticalAlignment);

		ReversedText->SetRelativeRotation(TextPairData.ReversedRotation);
		ReversedText->SetHorizontalAlignment(TextPairData.ReversedHorizontalAlignment);
		ReversedText->SetVerticalAlignment(TextPairData.ReversedVerticalAlignment);
	}
}

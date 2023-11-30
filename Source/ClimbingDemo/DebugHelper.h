#pragma once

namespace Debug
{
	static void Print(const FString& Msg, const FColor& Color = FColor::MakeRandomColor(), int32 key = -1,
	                  bool printCallStack = false)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(key, 6.0f, Color, Msg);
		}

		if (printCallStack)
		{
			FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		}

		UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
	}

	template<typename TEnum>
	static void PrintEnumValue(const TEnum EnumeratorValue)
	{
		Print(StaticEnum<TEnum>()->GetValueAsString(EnumeratorValue));
	}

	static void PrintWithStack(const FString& Msg, const FColor& Color = FColor::MakeRandomColor(), int32 key = -1)
	{
		Print(Msg, Color, key, true);
	}
}

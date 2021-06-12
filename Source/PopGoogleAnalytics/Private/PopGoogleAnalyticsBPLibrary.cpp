// Copyright Epic Games, Inc. All Rights Reserved.

#include "PopGoogleAnalyticsBPLibrary.h"
#include "PopGoogleAnalytics.h"
#include "HttpModule.h"
#include "Containers/UnrealString.h"

#define LOCTEXT_NAMESPACE "PopGoogleAnalytics"


class GoogleAnalyticsRequest_t //: public UObject
{
public:
	GoogleAnalyticsRequest_t();
	
protected:
	void		OnRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
	
public:
	TSharedPtr<IHttpRequest,ESPMode::ThreadSafe>	mRequest;
};

//	put these in the library?
TArray<TSharedRef<GoogleAnalyticsRequest_t>> gRequests;



GoogleAnalyticsRequest_t::GoogleAnalyticsRequest_t()
{
	auto Request = FHttpModule::Get().CreateRequest();
	mRequest = Request;
	//mRequest->SetVerb("POST");
	//mRequest->SetHeader("Content-Type", "application/json");
	FString Url = "https://four.tours/gitlastcommit";
	mRequest->SetURL(*FString::Printf(TEXT("%s"), *Url));
	//mRequest->SetContentAsString(JsonString);
	
	auto Callback = [this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		this->OnRequestCompleted( Request, Response, bWasSuccessful );
	};
	mRequest->OnProcessRequestComplete().BindLambda(Callback);
	//mRequest->OnProcessRequestComplete().BindUObject(this, &GoogleAnalyticsRequest_t::OnRequestCompleted);

	mRequest->ProcessRequest();
}
	
void GoogleAnalyticsRequest_t::OnRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	UE_LOG( LogTemp, Log, TEXT( "%s" ), __PRETTY_FUNCTION__ );
}








UPopGoogleAnalyticsBPLibrary::UPopGoogleAnalyticsBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UPopGoogleAnalyticsBPLibrary::PopGoogleAnalyticsPostEvent(FString Json)
{
	UE_LOG( LogTemp, Log, TEXT( "PopGoogleAnalyticsPostEvent" ) );
	try
	{
		TSharedRef<GoogleAnalyticsRequest_t> NewRequest( new GoogleAnalyticsRequest_t() );
		gRequests.Add(NewRequest);
	}
	catch(std::exception& e)
	{
		FString ErrorString = FString::Printf( TEXT( "PopGoogleAnalyticsPostEvent exception %s" ), e.what() );
		UE_LOG( LogTemp, Error, TEXT( "%s" ), *ErrorString );
	}
}

#undef LOCTEXT_NAMESPACE

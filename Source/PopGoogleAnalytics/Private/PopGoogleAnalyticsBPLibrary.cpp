// Copyright Epic Games, Inc. All Rights Reserved.

#include "PopGoogleAnalyticsBPLibrary.h"
#include "PopGoogleAnalytics.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Containers/UnrealString.h"

#define LOCTEXT_NAMESPACE "PopGoogleAnalytics"


class GoogleAnalyticsRequest_t //: public UObject
{
public:
	GoogleAnalyticsRequest_t(FString JsonString,bool ValidationRequest);
	
protected:
	void		OnRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
	
public:
	TSharedPtr<IHttpRequest,ESPMode::ThreadSafe>	mRequest;
};

//	put these in the library?
TArray<TSharedRef<GoogleAnalyticsRequest_t>> gRequests;

//	ids and keys from google analytics
//	see live feedback in the debug view
//	https://analytics.google.com/analytics/web/#/m/p275755571/debugview/overview
//	mmeasurement id from "Measurement Protocol API secrets"
const FString MeasurementId = "G-H854WHM16Z";

//const FString StreamId = "2652615313";
const FString ApiSecret = "_qzH82vMRJ2jU2WoGEBqiQ";
const FString MeasurementProtocolUrl = "https://www.google-analytics.com/mp/collect";
//	this is a validation url which sends back debug if the response is malformed, the normal url doesn't report errors
const FString ValidateMeasurementProtocolUrl = "https://www.google-analytics.com/debug/mp/collect";

GoogleAnalyticsRequest_t::GoogleAnalyticsRequest_t(FString JsonString,bool ValidationRequest)
{
	auto Request = FHttpModule::Get().CreateRequest();
	mRequest = Request;
	
	//	https://developers.google.com/analytics/devguides/collection/protocol/ga4/sending-events
	mRequest->SetVerb("POST");
	mRequest->SetHeader("Content-Type", "application/json");
	FString Url = ValidationRequest ? ValidateMeasurementProtocolUrl : MeasurementProtocolUrl;
	mRequest->SetURL(*FString::Printf( TEXT("%s?api_secret=%s%measurement_id=%s"), *Url, *ApiSecret, *MeasurementId ) );
	
	//	required json params
	//	.client_id=string	https://developers.google.com/gtagjs/reference/api#get_mp_example
	//	.events=[]
	//	https://developers.google.com/analytics/devguides/collection/protocol/ga4/reference?client_type=gtag#payload_query_parameters
	mRequest->SetContentAsString(JsonString);
	
	auto Callback = [this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		this->OnRequestCompleted( Request, Response, bWasSuccessful );
	};
	mRequest->OnProcessRequestComplete().BindLambda(Callback);
	//mRequest->OnProcessRequestComplete().BindUObject(this, &GoogleAnalyticsRequest_t::OnRequestCompleted);

	if ( !mRequest->ProcessRequest() )
	{
		UE_LOG( LogTemp, Error, TEXT( "GoogleAnalytics ProcessRequest() failed") );
	}
}
	
void GoogleAnalyticsRequest_t::OnRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	auto ResponseCode = Response ? Response->GetResponseCode() : 0;
	FString ResponseBody = Response ? Response->GetContentAsString() : "";
	//if ( bWasSuccessful )
	if ( ResponseCode == 200 )
	{
		UE_LOG( LogTemp, Log, TEXT( "GoogleAnalytics report completed. (%d), body:%s"), ResponseCode, *ResponseBody );
		return;
	}
	
	UE_LOG( LogTemp, Error, TEXT( "GoogleAnalytics report failed; response code %d; %s"), ResponseCode, *ResponseBody );
}








UPopGoogleAnalyticsBPLibrary::UPopGoogleAnalyticsBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UPopGoogleAnalyticsBPLibrary::PopGoogleAnalyticsPostEvent(FString Json,bool Validate)
{
	UE_LOG( LogTemp, Log, TEXT( "PopGoogleAnalyticsPostEvent" ) );
	try
	{
		//	.client_id=string	https://developers.google.com/gtagjs/reference/api#get_mp_example
		//	.events=[]
		//FString TestJson = "{ \"client_id\":\"UEClientTest\", \"events\":[{\"name\":\"TestEvent1\"}] }";

		TSharedRef<GoogleAnalyticsRequest_t> NewRequest( new GoogleAnalyticsRequest_t(Json,Validate) );
		gRequests.Add(NewRequest);
	}
	catch(std::exception& e)
	{
		FString ErrorString = FString::Printf( TEXT( "PopGoogleAnalyticsPostEvent exception %s" ), e.what() );
		UE_LOG( LogTemp, Error, TEXT( "%s" ), *ErrorString );
	}
}

#undef LOCTEXT_NAMESPACE

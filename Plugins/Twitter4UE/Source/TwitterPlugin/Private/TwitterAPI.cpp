// Copyright 2015 Narikazu Demura. All Rights Reserved.
/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "TwitterPrivatePCH.h"
#include "picojson.h"
#include "liboauthcpp/urlencode.h"
#include "liboauthcpp/liboauthcpp.h"

UTwitterAPI::UTwitterAPI(const class FObjectInitializer& PCIP)
	: Super(PCIP) {

}

void UTwitterAPI::DisplayHeaderAndParams(TArray<FString> Headers, const FString& OutStr){

	for (FString Head: Headers){
		UE_LOG(TwitterLoger, Log, TEXT("Header %s"), *Head);
	}
	UE_LOG(TwitterLoger, Log, TEXT("Param %s"), *OutStr);
}

void UTwitterAPI::RequestOAuthTokenWithPin(const FString& PinAuthAccessToken, const FString& PinAuthTokenSecret, const FString& PinNumber){

	OAuth::Consumer Consumer(TCHAR_TO_UTF8(*ConsumerKey), TCHAR_TO_UTF8(*ConsumerSecret));
	OAuth::Token Token(TCHAR_TO_UTF8(*PinAuthAccessToken), TCHAR_TO_UTF8(*PinAuthTokenSecret));
	
	Token.setPin(TCHAR_TO_UTF8(*PinNumber));

	OAuth::Client oauth(&Consumer, &Token);

	TSharedRef< IHttpRequest > HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("POST");

	FString url = "https://api.twitter.com/oauth/access_token";
	HttpRequest->SetURL(url);

	HttpRequest->SetHeader("Content-Type", "application/x-www-form-urlencoded");

	std::string rawUrl(TCHAR_TO_UTF8(*url));
	HttpRequest->SetHeader("Authorization", oauth.getHttpHeader(OAuth::Http::Post, rawUrl, "", true).c_str());

	HttpRequest->SetHeader("Content-Length", "0");
	HttpRequest->SetContentAsString("");

	DisplayHeaderAndParams(HttpRequest->GetAllHeaders(), "");

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UTwitterAPI::OnReadyAuthWithPin);

	// Execute the request
	HttpRequest->ProcessRequest();
}

void UTwitterAPI::RequestAccessToken(){

	OAuth::Consumer Consumer(TCHAR_TO_UTF8(*ConsumerKey), TCHAR_TO_UTF8(*ConsumerSecret));
	OAuth::Client Oauth(&Consumer);

	FString url = "https://api.twitter.com/oauth/request_token";

	TSharedRef< IHttpRequest > HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("POST");

	HttpRequest->SetURL(url);

	HttpRequest->SetHeader("Content-Type", "application/x-www-form-urlencoded");

	std::string rawUrl(TCHAR_TO_UTF8(*url));

	HttpRequest->SetHeader("Authorization", Oauth.getHttpHeader(OAuth::Http::Post, rawUrl, "").c_str());
	HttpRequest->SetHeader("Content-Length", "0");
	HttpRequest->SetContentAsString("");

	DisplayHeaderAndParams(HttpRequest->GetAllHeaders(), "");

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UTwitterAPI::OnReadyToken);

	// Execute the request
	HttpRequest->ProcessRequest();

}

FString UTwitterAPI::GetAuthURL(const FString& PinAuthAccessToken){

	FString Url("https://api.twitter.com/oauth/authorize?oauth_token=");

	return Url + PinAuthAccessToken;
}

void UTwitterAPI::UpdateTweet(const FString& Tweet){
	
	FString url = "https://api.twitter.com/1.1/statuses/update.json";

	OAuth::Consumer Consumer(TCHAR_TO_UTF8(*ConsumerKey), TCHAR_TO_UTF8(*ConsumerSecret));
	OAuth::Token Token(TCHAR_TO_UTF8(*AccessToken), TCHAR_TO_UTF8(*TokenSecret));
	OAuth::Client Oauth(&Consumer, &Token);

	std::string RawData = "status=";
	RawData.append(TCHAR_TO_UTF8(*Tweet));

	FString TmpOutStr(urlencode(TCHAR_TO_UTF8(*Tweet), URLEncode_QueryValue).c_str());
	FString OutStr("status=");
	OutStr.Append(TmpOutStr);

	// Create the post request with the generated data
	TSharedRef< IHttpRequest > HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("POST");
	
	HttpRequest->SetURL(url);

	HttpRequest->SetHeader("Content-Type", "application/x-www-form-urlencoded");

	std::string RawUrl("https://api.twitter.com/1.1/statuses/update.json");

	HttpRequest->SetHeader("Authorization", Oauth.getHttpHeader(OAuth::Http::Post, RawUrl, RawData).c_str());
	HttpRequest->SetHeader("Content-Length", FString::FromInt(OutStr.Len()));
	HttpRequest->SetContentAsString(OutStr);

	DisplayHeaderAndParams(HttpRequest->GetAllHeaders(), OutStr);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UTwitterAPI::OnReady);

	// Execute the request
	HttpRequest->ProcessRequest();
}

UTwitterAPI* UTwitterAPI::Create(const FString& AppConsumerKey, const FString& AppConsumerSecret) {

	// Construct the object and return it
	UTwitterAPI* FieldData = NewObject<UTwitterAPI>();

	FieldData->ConsumerKey = AppConsumerKey;
	FieldData->ConsumerSecret = AppConsumerSecret;
	return FieldData;
}


void UTwitterAPI::OnReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {

	if (!bWasSuccessful) {
		// Broadcast the failed event
		OnFailedPost.Broadcast();
		return;
	}

	UE_LOG(TwitterLoger, Log, TEXT("OnReady %s"), *Response->GetContentAsString());

	// post tweet
	picojson::value v;
	picojson::parse(v, TCHAR_TO_UTF8(*Response->GetContentAsString()));
	bool flg = v.contains("created_at");
	if (!flg){
		// Broadcast the failed event
		OnFailedPost.Broadcast();
		return;
	}
	// Broadcast the result event
	OnGetResult.Broadcast();
}

void UTwitterAPI::OnReadyToken(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {

	if (!bWasSuccessful) {
		// Broadcast the failed event
		OnFailedOAuth.Broadcast();
		return;
	}

	UE_LOG(TwitterLoger, Log, TEXT("OnReadyToken %s"), *Response->GetContentAsString());
	std::string Content(TCHAR_TO_UTF8(*Response->GetContentAsString()));
	std::size_t pos = Content.find("oauth_token");
	if (pos == std::string::npos){
		// Broadcast the failed event
		OnFailedOAuth.Broadcast();
		return;
	}

	OAuth::Token RequestToken = OAuth::Token::extract(Content);
	FString Key(RequestToken.key().c_str());
	FString Secret(RequestToken.secret().c_str());

	// Broadcast the result event
	OnGetAccessToken.Broadcast(Key, Secret);
}

void UTwitterAPI::OnReadyAuthWithPin(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {

	if (!bWasSuccessful) {
		// Broadcast the failed event
		OnFailedOAuth.Broadcast();
		return;
	}

	UE_LOG(TwitterLoger, Log, TEXT("OnReadyAuthWithPin %s"), *Response->GetContentAsString());
	std::string Content(TCHAR_TO_UTF8(*Response->GetContentAsString()));
	std::size_t pos = Content.find("oauth_token");
	if (pos == std::string::npos){
		// Broadcast the failed event
		OnFailedOAuth.Broadcast();
		return;
	}

	OAuth::Token RequestToken = OAuth::Token::extract(Content);
	FString Key(RequestToken.key().c_str());
	FString Secret(RequestToken.secret().c_str());

	// Broadcast the result event
	OnSuccessOAuth.Broadcast(Key, Secret);
}


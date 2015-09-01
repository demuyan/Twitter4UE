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
#include "liboauthcpp/urlencode.h"
#include "liboauthcpp/liboauthcpp.h"
#include "ImageUtils.h"
#include "Base64.h"

UTwitterAPI::UTwitterAPI(const class FObjectInitializer& PCIP)
	: Super(PCIP) {

}

void UTwitterAPI::DisplayHeader(TArray<FString> Headers){

	for (FString Head : Headers){
		UE_LOG(TwitterLoger, Log, TEXT("Header %s"), *Head);
	}
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

void UTwitterAPI::UpdateTweet(const FString& Tweet, const TArray<FString>& MediaIds){
	
	FString url = "https://api.twitter.com/1.1/statuses/update.json";

	OAuth::Consumer Consumer(TCHAR_TO_UTF8(*ConsumerKey), TCHAR_TO_UTF8(*ConsumerSecret));
	OAuth::Token Token(TCHAR_TO_UTF8(*AccessToken), TCHAR_TO_UTF8(*TokenSecret));
	OAuth::Client Oauth(&Consumer, &Token);

	std::string RawData = "status=";
	RawData.append(TCHAR_TO_UTF8(*Tweet));

	FString Params("status=");
	Params.Append(urlencode(TCHAR_TO_UTF8(*Tweet), URLEncode_QueryValue).c_str());

	for (const FString& Itr : MediaIds){
		if (Itr.IsEmpty() == false){
			Params.Append("&media_ids=");
			Params.Append(Itr);
			RawData.append("&media_ids=");
			RawData.append(TCHAR_TO_UTF8(*Itr));
		}
	}

	// Create the post request with the generated data
	TSharedRef< IHttpRequest > HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("POST");
	
	HttpRequest->SetURL(url);

	HttpRequest->SetHeader("Content-Type", "application/x-www-form-urlencoded");

	std::string RawUrl("https://api.twitter.com/1.1/statuses/update.json");


	HttpRequest->SetHeader("Authorization", Oauth.getHttpHeader(OAuth::Http::Post, RawUrl, RawData).c_str());
	HttpRequest->SetHeader("Content-Length", FString::FromInt(Params.Len()));
	HttpRequest->SetContentAsString(Params);

	DisplayHeaderAndParams(HttpRequest->GetAllHeaders(), Params);

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

	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	TSharedPtr<FJsonObject> ObjectPtr;
	if (FJsonSerializer::Deserialize(Reader, ObjectPtr) == false){
		// Broadcast the failed event
		OnFailedPost.Broadcast();
		return;
	}

	FJsonObject& Object = *ObjectPtr.Get();
	FString CreateAt;
	if (Object.TryGetStringField(TEXT("created_at"), CreateAt) == false){
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


void UTwitterAPI::InitScreenShot(){
	UGameViewportClient::OnScreenshotCaptured().AddUObject(this, &UTwitterAPI::OnScreenshotCapture);
}


void UTwitterAPI::OnScreenshotCapture(int32 ImageWidth, int32 ImageHeight, const TArray<FColor>& Bitmap) {

	UE_LOG(TwitterLoger, Log, TEXT("ScreenCaptured"));

	TArray<uint8> PngImage;
	TArray<FColor> Bitmap2(Bitmap);
	FImageUtils::CompressImageArray(ImageWidth, ImageHeight, Bitmap2, PngImage);

	UploadImage(ImageWidth, ImageHeight, PngImage);
}

void UTwitterAPI::GetPostDataBody(FString Boundary, uint32 ImageWidth, uint32 ImageHeight, const TArray<uint8>& Image, TArray<uint8>& out)
{
	FString PostDataBegin("");
	PostDataBegin += "--";
	PostDataBegin += Boundary;
	PostDataBegin += "\r\n";
	PostDataBegin += "Content-Disposition: form-data; name=\"media\"; filename=\"1px.png\"\r\n";
	PostDataBegin += "Content-Type: application/octet-stream\r\n";
	PostDataBegin += "\r\n";

	FString PostDataEnd("");
	PostDataEnd += "\r\n";
	PostDataEnd += "--";
	PostDataEnd += Boundary;
	PostDataEnd += "--\r\n";

	out.Empty();
	out.Append((uint8*)TCHAR_TO_ANSI(*PostDataBegin), PostDataBegin.Len());
	out.Append(Image);
	out.Append((uint8*)TCHAR_TO_ANSI(*PostDataEnd), PostDataEnd.Len());
}

FString UTwitterAPI::UploadImageDebug()
{
	OAuth::Consumer Consumer(TCHAR_TO_UTF8(*ConsumerKey), TCHAR_TO_UTF8(*ConsumerSecret));
	OAuth::Token Token(TCHAR_TO_UTF8(*AccessToken), TCHAR_TO_UTF8(*TokenSecret));
	OAuth::Client Oauth(&Consumer, &Token);

	uint8 onepx_png[] = {
		0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
		0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
		0x01, 0x03, 0x00, 0x00, 0x00, 0x25, 0xdb, 0x56, 0xca, 0x00, 0x00, 0x00,
		0x03, 0x50, 0x4c, 0x54, 0x45, 0xff, 0x4d, 0x00, 0x5c, 0x35, 0x38, 0x7f,
		0x00, 0x00, 0x00, 0x01, 0x74, 0x52, 0x4e, 0x53, 0xcc, 0xd2, 0x34, 0x56,
		0xfd, 0x00, 0x00, 0x00, 0x0a, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x63,
		0x62, 0x00, 0x00, 0x00, 0x06, 0x00, 0x03, 0x36, 0x37, 0x7c, 0xa8, 0x00,
		0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
	};
	unsigned int onepx_png_len = 95;

	FString Boundary("0123456789");
	TArray<uint8> PostDataBinary;

	TArray<uint8> Source;
	Source.Empty();
	Source.Append(onepx_png, onepx_png_len);

	GetPostDataBody(Boundary, 1, 1, Source, PostDataBinary);

	// Create the post request with the generated data
	TSharedRef< IHttpRequest > HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("POST");

	FString Url = "https://upload.twitter.com/1.1/media/upload.json";
	HttpRequest->SetURL(Url);

	FString ContentType("multipart/form-data; boundary=");
	ContentType.Append(Boundary);
	HttpRequest->SetHeader("Content-Type", ContentType);

	std::string RawUrl(TCHAR_TO_UTF8(*Url));
	//	std::string RawData(TCHAR_TO_UTF8(*PostData));

	HttpRequest->SetHeader("Authorization", Oauth.getHttpHeaderMultipart(OAuth::Http::Post, RawUrl, (const char*)PostDataBinary.GetData(), PostDataBinary.Num()).c_str());
	HttpRequest->SetHeader("Content-Length", FString::FromInt(PostDataBinary.Num()));

	HttpRequest->SetContent(PostDataBinary);

	DisplayHeader(HttpRequest->GetAllHeaders());

	FString returnval("");

	return returnval;
}



void UTwitterAPI::UploadImage(int32 ImageWidth, int32 ImageHeight, const TArray<uint8>& Image)
{
	OAuth::Consumer Consumer(TCHAR_TO_UTF8(*ConsumerKey), TCHAR_TO_UTF8(*ConsumerSecret));
	OAuth::Token Token(TCHAR_TO_UTF8(*AccessToken), TCHAR_TO_UTF8(*TokenSecret));
	OAuth::Client Oauth(&Consumer, &Token);

	FString Boundary("twit4ue");
	Boundary.AppendInt(FMath::Rand());
	TArray<uint8> PostDataBinary;

	GetPostDataBody(Boundary, ImageWidth, ImageHeight, Image, PostDataBinary);

	// Create the post request with the generated data
	TSharedRef< IHttpRequest > HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("POST");

	FString Url = "https://upload.twitter.com/1.1/media/upload.json";
	HttpRequest->SetURL(Url);

	FString ContentType("multipart/form-data; boundary=");
	ContentType.Append(Boundary);
	HttpRequest->SetHeader("Content-Type", ContentType);

	std::string RawUrl(TCHAR_TO_UTF8(*Url));

	HttpRequest->SetHeader("Authorization", Oauth.getHttpHeaderMultipart(OAuth::Http::Post, RawUrl, (const char*)PostDataBinary.GetData(), PostDataBinary.Num()).c_str());
	HttpRequest->SetHeader("Content-Length", FString::FromInt(PostDataBinary.Num()));

	HttpRequest->SetContent(PostDataBinary);

	DisplayHeader(HttpRequest->GetAllHeaders());

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UTwitterAPI::OnMultimediaUploadComplete);

	// Execute the request
	HttpRequest->ProcessRequest();

}

void UTwitterAPI::OnMultimediaUploadComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {

	if (!bWasSuccessful) {
		// Broadcast the failed event
		OnFailMultimediaUpload.Broadcast();
		return;
	}

	UE_LOG(TwitterLoger, Log, TEXT("OnMultimediaUploadComplete %s"), *Response->GetContentAsString());

	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	TSharedPtr<FJsonObject> ObjectPtr;
	if (FJsonSerializer::Deserialize(Reader, ObjectPtr) == false){
		// Broadcast the failed event
		OnFailMultimediaUpload.Broadcast();
		return;
	}

	FJsonObject& Object = *ObjectPtr.Get();
	FString MediaIdString;
	if (Object.TryGetStringField(TEXT("media_id_string"), MediaIdString) == false){
		// Broadcast the failed event
		OnFailMultimediaUpload.Broadcast();
		return;
	}

	// Broadcast the result event
	OnCompleteMultimediaupload.Broadcast(MediaIdString);
}

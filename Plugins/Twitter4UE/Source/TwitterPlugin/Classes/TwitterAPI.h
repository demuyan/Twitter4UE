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

#pragma once

#include "Http.h"
#include "TwitterAPI.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGetResult);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetAccessToken, FString, accessToken, FString, accessTokenSecret);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSuccessOAuth, FString, accessToken, FString, accessTokenSecret);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFailedOAuth);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFailedPost);

/**
  * Twitter APIクラス
  */
UCLASS(BlueprintType, Blueprintable)
class UTwitterAPI : public UObject
{
	GENERATED_UCLASS_BODY()

private:

	/**
	* @brief Tweet関数の処理完了時のコールバック関数
	* @param Request サーバへのリクエスト内容
	* @param Response サーバからの反応
	* @param bWasSuccess サーバからの反応
	*/
	void OnReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
	/** 
	 * @brief RequestAccessToken関数の処理完了時のコールバック関数
	 * @param Request サーバへのリクエスト内容
	 * @param Response サーバからの反応
	 * @param bWasSuccess サーバからの反応
	 */
	void OnReadyToken(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/**
	* @brief RequestOAuthTokenWithPin関数の処理完了時のコールバック関数
	* @param Request サーバへのリクエスト内容
	* @param Response サーバからの反応
	* @param bWasSuccess サーバからの反応
	*/
	void OnReadyAuthWithPin(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/**
	* @brief HTTPヘッダ情報と引数を表示する
	* @param headers ヘッダサーバへのリクエスト内容
	* @param Response サーバからの反応
	*/
	void DisplayHeaderAndParams(TArray<FString> headers, const FString& outStr);

public:

	/** コンシューマ　キー　*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Twitter")
	FString ConsumerKey;

	/** コンシューマ　シークレット　*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Twitter")
	FString ConsumerSecret;

	/** アクセストークン　*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Twitter")
	FString AccessToken;

	/** トークンシークレット　*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Twitter")
	FString TokenSecret;

	/* 認証前のアクセストークン（公開鍵と秘密鍵）の取得時のイベント */
	UPROPERTY(BlueprintAssignable, Category = "Twitter")
	FOnGetResult OnGetResult;

	/** アクセストークンを取得時のイベント */
	UPROPERTY(BlueprintAssignable, Category = "Twitter")
	FOnGetAccessToken OnGetAccessToken;

	/** OAuth認証に成功時のイベント */
	UPROPERTY(BlueprintAssignable, Category = "Twitter")
	FOnSuccessOAuth OnSuccessOAuth;

	/** 投稿に失敗時のイベント */
	UPROPERTY(BlueprintAssignable, Category = "Twitter")
	FOnFailedPost OnFailedPost;

	/** 認証に失敗時のイベント */
	UPROPERTY(BlueprintAssignable, Category = "Twitter")
	FOnFailedOAuth OnFailedOAuth;

	/** @brief PIN認証用URLを取得する 
	 *  @param accessToken URLに渡すアクセストークン（公開鍵）
	 *  @retval PIN認証用URL
	 *
	 *  PIN認証をするためのURLを取得する。取得したURLは、標準ブラウザで開くことで認証画面へ遷移する。
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get AuthURL"), Category = "Twitter")
	FString GetAuthURL(const FString& PinAuthAccessToken);

	/** @brief TwitterAPIインスタンスを取得する
	 *  @param AppConsumerKey コンシューマーキー（公開鍵）
	 *  @param AppConsumerKeySecret コンシューマーキー（秘密鍵）
	 *  @retval TwitterAPI本体
	 *
	 *   TwitterAPIインスタンスを取得する。このインスタンスを通して、プラグインとやりとりを行う。
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Create Twitter Client"), Category = "Twitter")
	static UTwitterAPI* Create( const FString& AppConsumerKey, const FString& AppConsumerSecret );

	/** @brief ツイートする
	 *  @param tweet 投稿内容（文章）
     *  
	 *  引数で渡した内容をツイートする
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Update Tweet"), Category = "Twitter")
	void UpdateTweet(const FString& Tweet);

	/** @brief PIN認証用アクセストークンを取得する。
     *  @param tweet 投稿内容（文章）
	 *
	 *  PIN認証の画面を表示したり、投稿用アクセストークンを取得するためのアクセストークンを取得する
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Request Access Token"), Category = "Twitter")
	void RequestAccessToken();

	/** @brief 投稿用アクセストークンを取得する
	*   @param PinAuthAccessToken PIN認証用アクセストークン（公開鍵）
	*   @param PinAuthTokenSecret PIN認証用アクセストークン（秘密鍵）
	*   @param PinNumber PIN番号
	*
	*   PIN認証した後の投稿用アクセストークンを取得するためのトークンを取得する
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Request OAuth Token With PIN"), Category = "Twitter")
	void RequestOAuthTokenWithPin( const FString& PinAuthAccessToken, const FString& PinAuthTokenSecret, const FString& PinNumber);

};
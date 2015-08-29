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
  * Twitter API�N���X
  */
UCLASS(BlueprintType, Blueprintable)
class UTwitterAPI : public UObject
{
	GENERATED_UCLASS_BODY()

private:

	/**
	* @brief Tweet�֐��̏����������̃R�[���o�b�N�֐�
	* @param Request �T�[�o�ւ̃��N�G�X�g���e
	* @param Response �T�[�o����̔���
	* @param bWasSuccess �T�[�o����̔���
	*/
	void OnReady(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
	/** 
	 * @brief RequestAccessToken�֐��̏����������̃R�[���o�b�N�֐�
	 * @param Request �T�[�o�ւ̃��N�G�X�g���e
	 * @param Response �T�[�o����̔���
	 * @param bWasSuccess �T�[�o����̔���
	 */
	void OnReadyToken(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/**
	* @brief RequestOAuthTokenWithPin�֐��̏����������̃R�[���o�b�N�֐�
	* @param Request �T�[�o�ւ̃��N�G�X�g���e
	* @param Response �T�[�o����̔���
	* @param bWasSuccess �T�[�o����̔���
	*/
	void OnReadyAuthWithPin(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/**
	* @brief HTTP�w�b�_���ƈ�����\������
	* @param headers �w�b�_�T�[�o�ւ̃��N�G�X�g���e
	* @param Response �T�[�o����̔���
	*/
	void DisplayHeaderAndParams(TArray<FString> headers, const FString& outStr);

public:

	/** �R���V���[�}�@�L�[�@*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Twitter")
	FString ConsumerKey;

	/** �R���V���[�}�@�V�[�N���b�g�@*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Twitter")
	FString ConsumerSecret;

	/** �A�N�Z�X�g�[�N���@*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Twitter")
	FString AccessToken;

	/** �g�[�N���V�[�N���b�g�@*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Twitter")
	FString TokenSecret;

	/* �F�ؑO�̃A�N�Z�X�g�[�N���i���J���Ɣ閧���j�̎擾���̃C�x���g */
	UPROPERTY(BlueprintAssignable, Category = "Twitter")
	FOnGetResult OnGetResult;

	/** �A�N�Z�X�g�[�N�����擾���̃C�x���g */
	UPROPERTY(BlueprintAssignable, Category = "Twitter")
	FOnGetAccessToken OnGetAccessToken;

	/** OAuth�F�؂ɐ������̃C�x���g */
	UPROPERTY(BlueprintAssignable, Category = "Twitter")
	FOnSuccessOAuth OnSuccessOAuth;

	/** ���e�Ɏ��s���̃C�x���g */
	UPROPERTY(BlueprintAssignable, Category = "Twitter")
	FOnFailedPost OnFailedPost;

	/** �F�؂Ɏ��s���̃C�x���g */
	UPROPERTY(BlueprintAssignable, Category = "Twitter")
	FOnFailedOAuth OnFailedOAuth;

	/** @brief PIN�F�ؗpURL���擾���� 
	 *  @param accessToken URL�ɓn���A�N�Z�X�g�[�N���i���J���j
	 *  @retval PIN�F�ؗpURL
	 *
	 *  PIN�F�؂����邽�߂�URL���擾����B�擾����URL�́A�W���u���E�U�ŊJ�����ƂŔF�؉�ʂ֑J�ڂ���B
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get AuthURL"), Category = "Twitter")
	FString GetAuthURL(const FString& PinAuthAccessToken);

	/** @brief TwitterAPI�C���X�^���X���擾����
	 *  @param AppConsumerKey �R���V���[�}�[�L�[�i���J���j
	 *  @param AppConsumerKeySecret �R���V���[�}�[�L�[�i�閧���j
	 *  @retval TwitterAPI�{��
	 *
	 *   TwitterAPI�C���X�^���X���擾����B���̃C���X�^���X��ʂ��āA�v���O�C���Ƃ��Ƃ���s���B
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Create Twitter Client"), Category = "Twitter")
	static UTwitterAPI* Create( const FString& AppConsumerKey, const FString& AppConsumerSecret );

	/** @brief �c�C�[�g����
	 *  @param tweet ���e���e�i���́j
     *  
	 *  �����œn�������e���c�C�[�g����
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Update Tweet"), Category = "Twitter")
	void UpdateTweet(const FString& Tweet);

	/** @brief PIN�F�ؗp�A�N�Z�X�g�[�N�����擾����B
     *  @param tweet ���e���e�i���́j
	 *
	 *  PIN�F�؂̉�ʂ�\��������A���e�p�A�N�Z�X�g�[�N�����擾���邽�߂̃A�N�Z�X�g�[�N�����擾����
	 */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Request Access Token"), Category = "Twitter")
	void RequestAccessToken();

	/** @brief ���e�p�A�N�Z�X�g�[�N�����擾����
	*   @param PinAuthAccessToken PIN�F�ؗp�A�N�Z�X�g�[�N���i���J���j
	*   @param PinAuthTokenSecret PIN�F�ؗp�A�N�Z�X�g�[�N���i�閧���j
	*   @param PinNumber PIN�ԍ�
	*
	*   PIN�F�؂�����̓��e�p�A�N�Z�X�g�[�N�����擾���邽�߂̃g�[�N�����擾����
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Request OAuth Token With PIN"), Category = "Twitter")
	void RequestOAuthTokenWithPin( const FString& PinAuthAccessToken, const FString& PinAuthTokenSecret, const FString& PinNumber);

};
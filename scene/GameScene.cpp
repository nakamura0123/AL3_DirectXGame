#include "GameScene.h"
#include "AxisIndicator.h"
#include "ImGuiManager.h"
#include "MathUtilityForText.h"
#include "PrimitiveDrawer.h"
#include "TextureManager.h"
#include "time.h"
#include <cassert>

GameScene::GameScene() {}

GameScene::~GameScene() {
	delete modelStage_;
	delete spriteBG_;
	delete modelPlayer_;
	delete modelBeam_;
	delete modelEnemy_;
	delete modelItem_;
	delete spriteTitle_; // タイトル
	delete spriteEnter_; // ゲームオーバー

	for (int s = 0; s < 5; s++) {
		delete spriteNumber_[s]; // スコア数値
	}

	delete spriteScore_; // スコア

	for (int l = 0; l < 3; l++) {
		delete spriteLife_[l]; // ライフ
	}
}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	//// スプライトの生成
	textreHandleBG_ = TextureManager::Load("bg.jpg");
	spriteBG_ = Sprite::Create(textreHandleBG_, {0, 0});

	// 3Dモデルの生成 ステージ
	textureHandleStage_ = TextureManager::Load("stage2.jpg");
	modelStage_ = Model::Create();

	// ステージの数ループする
	for (int w = 0; w < 20; w++) {

		worldTransformStage_[w].Initialize();

		// ステージ位置を変更
		worldTransformStage_[w].translation_ = {0, -1.5f, 2.0f * w - 5};
		worldTransformStage_[w].scale_ = {4.5f, 1, 1}; // 4.5f

		// 変数行列を更新
		worldTransformStage_->matWorld_ = MakeAffineMatrix(
		    worldTransformStage_[w].scale_, worldTransformStage_[w].rotation_,
		    worldTransformStage_[w].translation_);

		// 変換行列を定数バッファに転送
		worldTransformStage_[w].TransferMatrix();
	}

	// ビュープロジェクションの初期化
	viewProjection_.translation_.y = 1;
	viewProjection_.translation_.z = -6;
	viewProjection_.Initialize();

	// プレイヤー
	textureHandlePlayer_ = TextureManager::Load("player.png");
	modelPlayer_ = Model::Create();
	worldTransformPlayer_.scale_ = {0.4f, 0.4f, 0.4f};
	worldTransformPlayer_.Initialize();

	// ビーム
	textureHandleBeam_ = TextureManager::Load("beam.png");
	modelBeam_ = Model::Create();

	for (int b = 0; b < 10; b++) {

		worldTransformBeam_[b].scale_ = {0.25f, 0.25f, 0.25f};
		worldTransformBeam_[b].Initialize();
	}

	// 敵
	textureHandleEnemy_ = TextureManager::Load("enemy.png");
	modelEnemy_ = Model::Create();

	for (int e = 0; e < enemy; e++) {
		worldTransformEnemy_[e].scale_ = {0.4f, 0.4f, 0.4f};
		worldTransformEnemy_[e].Initialize();
	}

	// アイテム
	textureHandleItem_ = TextureManager::Load("player.png");
	modelItem_ = Model::Create();

	for (int i = 0; i < item; i++) {
		worldTransformItem_[i].scale_ = {0.4f, 0.4f, 0.4f};
		worldTransformItem_[i].Initialize();
	}

	// 乱数の初期化
	srand((unsigned int)time(NULL));

	// デバッグテキスト
	debugText_ = DebugText::GetInstance();
	debugText_->Initialize();

	// タイトル(2Dスプライト)
	textureHandleTitle_ = TextureManager::Load("title.png");
	spriteTitle_ = Sprite::Create(textureHandleTitle_, {0, 0});

	textureHandleEnter_ = TextureManager::Load("enter.png");
	spriteEnter_ = Sprite::Create(textureHandleEnter_, {400, 450});

	textureHandleGameOver_ = TextureManager::Load("gameover.png");
	spriteGameOver_ = Sprite::Create(textureHandleGameOver_, {0, 0});

	// サウンドデータの読み込み

	soundDataHandleTitleBGM_ = audio_->LoadWave("Audio/Ring05.wav");
	soundDataHandleGamePlayerBGM_ = audio_->LoadWave("Audio/Ring08.wav");
	soundDataHandleGameOverBGM_ = audio_->LoadWave("Audio/Ring09.wav");
	soundDataHandleEnemyHitSE_ = audio_->LoadWave("Audio/chord.wav");
	soundDataHandlePlayerHitSE_ = audio_->LoadWave("Audio/tada.wav");

	; // タイトルBGM再生
	voiceHandleBGM_ = audio_->PlayWave(soundDataHandleTitleBGM_, true);

	// スコア数値（2Dスプライト)
	textureHandleNumber_ = TextureManager::Load("number.png");

	for (int s = 0; s < 5; s++) {

		spriteNumber_[s] = Sprite::Create(textureHandleNumber_, {300.0f + s * 26, 0});
	}

	textureHandleScore_ = TextureManager::Load("score.png");
	spriteScore_ = Sprite::Create(textureHandleScore_, {175.0f, 0});

	textureHandleLife_ = TextureManager::Load("player.png");

	// ライフ(2Dスプライト)
	for (int l = 0; l < 3; l++) {

		spriteLife_[l] = Sprite::Create(textureHandleLife_, {800.0f + l * 60, 0});
		spriteLife_[l]->SetSize({40, 40});
	}
}

void GameScene::Update() {

	switch (sceneMode_) {
	case 0:
		GamePlayUpdate();
		break;

	case 1:
		TitleUpdate();
		break;

	case 2:
		GameOverUpdate();
		break;
	}
}

void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	switch (sceneMode_) {
	case 0:
		GamePlayDraw2DBack();
		break;

	case 2:
		GamePlayDraw2DBack();
		break;
	}

	// スプライト描画後処理
	Sprite::PostDraw();

	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>

	switch (sceneMode_) {
	case 0:
		GamePlayDraw3D();
		break;

	case 2:
		GamePlayDraw3D();
		break;
	}

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	switch (sceneMode_) {
	case 0:
		GamePlayDraw2DNear();
		break;

	case 1:
		TitleDraw2DNear();
		break;

	case 2:
		GamePlayDraw2DNear();
		GameOverDraw2DNear();
		break;
	}

	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}

// プレイヤー
//--------------------------------------------------------------------

// プレイヤー更新
void GameScene::PlayerUpdate() {

	// 移動
	// 右へ移動
	playerSpeed = 0.1f;

	if (input_->PushKey(DIK_RIGHT)) {

		if (player == 0) {
			if (worldTransformPlayer_.translation_.x > -2.5) {
				playerSpeed = 0;
			}
			worldTransformPlayer_.translation_.x += playerSpeed;
		}
		if (player == 1) {
			if (worldTransformPlayer_.translation_.x > 0.7) {
				playerSpeed = 0;
			}
			worldTransformPlayer_.translation_.x += playerSpeed;
		}
		if (player == 2) {
			if (worldTransformPlayer_.translation_.x > 4) {
				playerSpeed = 0;
			}
			worldTransformPlayer_.translation_.x += playerSpeed;
		}
	}

	// 左へ移動

	if (input_->PushKey(DIK_LEFT)) {

		if (player == 0) {
			if (worldTransformPlayer_.translation_.x < -4) {
				playerSpeed = 0;
			}
			worldTransformPlayer_.translation_.x -= playerSpeed;
		}
		if (player == 1) {
			if (worldTransformPlayer_.translation_.x < -1) {
				playerSpeed = 0;
			}
			worldTransformPlayer_.translation_.x -= playerSpeed;
		}
		if (player == 2) {
			if (worldTransformPlayer_.translation_.x < 2.3) {
				playerSpeed = 0;
			}
			worldTransformPlayer_.translation_.x -= playerSpeed;
		}
	}

	if (input_->PushKey(DIK_1)) {

		player = 0;
		worldTransformPlayer_.translation_.x = -2.5;
	}
	if (input_->PushKey(DIK_2)) {

		player = 1;
		worldTransformPlayer_.translation_.x = 0;
	}
	if (input_->PushKey(DIK_3)) {

		player = 2;
		worldTransformPlayer_.translation_.x = 2.5;
	}

	/*if (player == 0) {

	    worldTransformPlayer_.translation_.x = max(worldTransformPlayer_.translation_.x, -4);
	    worldTransformPlayer_.translation_.x = min(worldTransformPlayer_.translation_.x, -1.5);
	}
	if (player == 1) {

	    worldTransformPlayer_.translation_.x = max(worldTransformPlayer_.translation_.x, -1);
	    worldTransformPlayer_.translation_.x = min(worldTransformPlayer_.translation_.x, 1);
	}

	if (player == 2) {

	    worldTransformPlayer_.translation_.x = max(worldTransformPlayer_.translation_.x, 2.5);
	    worldTransformPlayer_.translation_.x = min(worldTransformPlayer_.translation_.x, 4);
	}*/

	// 変数行列を更新
	worldTransformPlayer_.matWorld_ = MakeAffineMatrix(
	    worldTransformPlayer_.scale_, worldTransformPlayer_.rotation_,
	    worldTransformPlayer_.translation_);

	// 変換行列を定数バッファに転送
	worldTransformPlayer_.TransferMatrix();

	// 点滅
	if (PlayerTimer_ > 0) {

		PlayerTimer_--;
	}
}

// ビーム
//--------------------------------------------------------------

// ビーム更新
void GameScene::BeamUpdate() {

	BeamMove();
	BeamBorn();

	for (int b = 0; b < 10; b++) {

		// 変数行列を更新
		worldTransformBeam_[b].matWorld_ = MakeAffineMatrix(
		    worldTransformBeam_[b].scale_, worldTransformBeam_[b].rotation_,
		    worldTransformBeam_[b].translation_);

		// 変換行列を定数バッファに転送
		worldTransformBeam_[b].TransferMatrix();
	}
}

void GameScene::BeamMove() {

	for (int b = 0; b < 10; b++) {
		if (isBeam_[b] == false)
			continue;

		// 奥へ移動
		worldTransformBeam_[b].translation_.z += 0.3f;
		// 回転
		worldTransformBeam_[b].rotation_.x += 0.1f;

		if (worldTransformBeam_[b].translation_.z > 40) {

			isBeam_[b] = false;
			worldTransformBeam_[b].translation_.z = 0;
		}
	}
}

void GameScene::BeamBorn() {
	for (int b = 0; b < 10; b++) {

		if (isBeam_[b] == true)
			continue;
		// 発射タイマーが0ならば
		if (beamTimer_ == 0) {

			if (input_->TriggerKey(DIK_SPACE)) {
				isBeam_[b] = true;
				worldTransformBeam_[b].translation_.x = worldTransformPlayer_.translation_.x;
				worldTransformBeam_[b].translation_.z = worldTransformPlayer_.translation_.z;
				break;
			}
		} else {

			// 発射タイマーが１以上
			// １０を超えると再び発射が可能

			beamTimer_++;
		}
		if (beamTimer_ > 10) {

			beamTimer_ = 0;
		}
	}
}
// 敵
//  --------------------------------------------------------------------
void GameScene::EnemyUpdate() {

	EnemyMove();
	EnemyBorn();
	EnemyJump();

	enemyTimer -= 1;

	if (enemyTimer == 0) {

		enemyTimer = 60;
		enemyBorn -= 50;
	}
	if (enemyBorn <= 100) {

		enemyBorn = 50;
	}

	for (int e = 0; e < enemy; e++) {
		// 変数行列を更新
		worldTransformEnemy_[e].matWorld_ = MakeAffineMatrix(
		    worldTransformEnemy_[e].scale_, worldTransformEnemy_[e].rotation_,
		    worldTransformEnemy_[e].translation_);

		// 変換行列を定数バッファに転送
		worldTransformEnemy_[e].TransferMatrix();
	}
}

void GameScene::EnemyMove() {

	for (int e = 0; e < enemy; e++) {

		// 手前へ移動
		worldTransformEnemy_[e].translation_.x += enemySpeed_[e];
		worldTransformEnemy_[e].translation_.z -= 0.01f;
		worldTransformEnemy_[e].translation_.z -= gameTimer_ / 5000.0f;

		if (isEnemy_[e] == 1) {

			if (worldTransformEnemy_[e].translation_.x >= 4) {

				enemySpeed_[e] = -0.1f;
			}
			if (worldTransformEnemy_[e].translation_.x <= -4) {

				enemySpeed_[e] = 0.1f;
			}
		}

		// 回転
		worldTransformEnemy_[e].rotation_.x -= 0.15f;
	}
}

void GameScene::EnemyBorn() {

	for (int e = 0; e < enemy; e++) {

		if (rand() % enemyBorn == 0) { // 1000
			/*int en = rand() % 3;
			if (en==1) {

			}

			else if (en == 2) {
			} else if (en == 3) {
			}*/

			if (isEnemy_[e] == 0) {

				isEnemy_[e] = 1;
				worldTransformEnemy_[e].translation_.y = 0;
				worldTransformEnemy_[e].translation_.z = 40;
				int x = rand() % 80;

				float x2 = (float)x / 10 - 4;
				worldTransformEnemy_[e].translation_.x = x2;

				if (rand() % 2 == 0) {

					enemySpeed_[e] = 0.1f;
				} else {
					enemySpeed_[e] = -0.1f;
				}

				// ループ終了
				break;
			}
		}

		if (worldTransformEnemy_[e].translation_.z < 0) {

			isEnemy_[e] = false;
		}
	}
}

void GameScene::EnemyJump() {

	for (int e = 0; e < enemy; e++) {

		// 消滅演出ならば
		if (isEnemy_[e] == 2) {
			// 移動（Y座標に速度を与える）
			worldTransformEnemy_[e].translation_.y += enemyJumpSpeed_[e];

			// 速度を減らす
			enemyJumpSpeed_[e] -= 0.1f;

			// 斜め移動
			worldTransformEnemy_[e].translation_.x += enemySpeed_[e] * 4;

			// 下に落ちると消滅
			//void GameScene::ItemJump() {

			//	for (int i = 0; i < 10; i++) {

			//		// 消滅演出ならば
			//		if (isItem_[i] == 2) {
			//			// 移動（Y座標に速度を与える）
			//			worldTransformItem_[i].translation_.y += itemSpeed_[i];

			//			// 横回転
			//			worldTransformItem_[i].rotation_.y -= 0.15f;

			//			// 下に落ちると消滅
			//			if (worldTransformItem_[i].translation_.y > 3) {
			//				isItem_[i] = 0; // 存在しない
			//			}
			//		}
			//	}
			//}
		}
	}
}
	

// 衝突判定
//-------------------------------------------------------------------------
void GameScene::Collision()
{
	CollisionPlayerEnemy();
	CollisionBeamEnemy();
	CollisionPlayerItem();
}

void GameScene::CollisionPlayerEnemy() {

	for (int e = 0; e < enemy; e++) {

		if (isEnemy_[e] == 1) {

			float dx =
				abs(worldTransformPlayer_.translation_.x -
					worldTransformEnemy_[e].translation_.x);
			float dz =
				abs(worldTransformPlayer_.translation_.z -
					worldTransformEnemy_[e].translation_.z);

			// 衝突したら
			if (dx < 1 && dz < 1) {

				isEnemy_[e] = 0;  // 存在しない
				playerLife_ -= 1; // ライフの減算
				audio_->PlayWave(soundDataHandlePlayerHitSE_);
				PlayerTimer_ = 60;
			}
		}
	}

	if (playerLife_ <= 0) {

		sceneMode_ = 2;
	}
}

void GameScene::CollisionBeamEnemy() {
	for (int e = 0; e < enemy; e++) {
		for (int b = 0; b < 10; b++) {
			if (isEnemy_[e] == 1) {
				if (isBeam_[b] == true) {

					float dx =
						abs(worldTransformBeam_[b].translation_.x -
							worldTransformEnemy_[e].translation_.x);
					float dz =
						abs(worldTransformBeam_[b].translation_.z -
							worldTransformEnemy_[e].translation_.z);

					// 衝突したら
					if (dx < 1 && dz < 1) {

						enemyJumpSpeed_[e] = 1;
						isEnemy_[e] = 2;
						isBeam_[b] = false;
						gameScore_ += 100;
						audio_->PlayWave(soundDataHandleEnemyHitSE_);
					}
				}
			}
		}
	}
}

void GameScene::CollisionPlayerItem() {

	for (int i = 0; i < 10; i++) {

		if (isItem_[i] == 1) {

			float dx =
				abs(worldTransformPlayer_.translation_.x -
					worldTransformItem_[i].translation_.x);
			float dz =
				abs(worldTransformPlayer_.translation_.z -
					worldTransformItem_[i].translation_.z);

			// 衝突したら
			if (dx < 1 && dz < 1) {

				isItem_[i] = 2; // 衝突演出
				PlayerTimer_ = 60;
				// audio_->PlayWave(soundDataHandlePlayerHitSE_);
			}
		}
	}
}

// タイトル
//-------------------------------------------------------------------------
// タイトル更新  初期化処理
void GameScene::TitleUpdate() {

	gameTimer_ += 1;

	// エンターキーを押した瞬間
	if (input_->TriggerKey(DIK_RETURN)) {
		// モードをゲームプレイへ変更
		sceneMode_ = 0;
		gameScore_ = 0;
		playerLife_ = 3;
		gameTimer_ = 0;
		enemyTimer = 60;
		enemyBorn = 2000;
		for (int e = 0; e < 10; e++) {
			isEnemy_[e] = 0;
		}
		for (int b = 0; b < 10; b++) {
			isBeam_[b] = false;
		}

		worldTransformPlayer_.translation_.x = 0;

		audio_->StopWave(voiceHandleBGM_); // 現在のBGMを停止
		voiceHandleBGM_ = audio_->PlayWave(soundDataHandleGamePlayerBGM_, true);
		// ゲームプレイBGMを再生
	}
}

// タイトル表示
void GameScene::TitleDraw2DNear() {

	spriteTitle_->Draw();

	// エンター表示
	if (gameTimer_ % 40 >= 20) {

		spriteEnter_->Draw();
	}
}

void GameScene::GameOverUpdate() {

	gameTimer_ += 1;

	// エンターキーを押した瞬間
	if (input_->TriggerKey(DIK_RETURN)) {
		// モードをタイトルへ変更
		sceneMode_ = 1;
	}
	audio_->StopWave(voiceHandleBGM_); // 現在のBGMを停止
	voiceHandleBGM_ = audio_->PlayWave(soundDataHandleGameOverBGM_, true);
	// ゲームオーバーBGMを再生
}

void GameScene::GameOverDraw2DNear() {
	spriteGameOver_->Draw();

	// エンター表示
	if (gameTimer_ % 40 >= 20) {

		spriteEnter_->Draw();
	}
}

void GameScene::StageUpdata() {

	for (int w = 0; w < 20; w++) {

		// 手前に移動
		worldTransformStage_[w].translation_.z -= 0.1f;
		// 端まで来たら奥へ戻る
		if (worldTransformStage_[w].translation_.z < -5) {

			worldTransformStage_[w].translation_.z += 40;
		}

		// 変換行列を更新
		worldTransformStage_[w].matWorld_ = MakeAffineMatrix(
			worldTransformStage_[w].scale_, worldTransformStage_[w].rotation_,
			worldTransformStage_[w].translation_);
		// 変換行列を定位数バッファに転送
		worldTransformStage_[w].TransferMatrix();
	}
}



// 更新
void GameScene::GamePlayUpdate() {

	gameTimer_++;

	PlayerUpdate();
	BeamUpdate();
	EnemyUpdate();
	Collision();
	StageUpdata();
}
// ゲームプレイ表示3D
void GameScene::GamePlayDraw3D() {

	for (int w = 0; w < 20; w++) {
		// ステージ
		modelStage_->Draw(
			worldTransformStage_[w], viewProjection_, textureHandleStage_);
	}

	if (PlayerTimer_ % 4 < 2) {
		// プレイヤー
		modelPlayer_->Draw(
			worldTransformPlayer_, viewProjection_, textureHandlePlayer_);
	}
	// ビーム
	for (int b = 0; b < 10; b++) {
		if (isBeam_[b] == true) {

			modelBeam_->Draw(
				worldTransformBeam_[b], viewProjection_, textureHandleBeam_);
		}
	}
	for (int e = 0; e < 10; e++) {
		// 敵
		if (isEnemy_[e] != 0) {

			modelEnemy_->Draw(
				worldTransformEnemy_[e], viewProjection_, textureHandleEnemy_);
		}
	}

	for (int i = 0; i < 10; i++) {
		// アイテム
		if (isItem_[i] != 0) {

			modelItem_->Draw(
				worldTransformItem_[i], viewProjection_, textureHandleItem_);
		}
	}
}
// ゲームプレイ表示2D背景
void GameScene::GamePlayDraw2DBack() {

	// 背景
	spriteBG_->Draw();
}
// ゲームプレイ表示2D前景
void GameScene::GamePlayDraw2DNear() {

	DrawScore();

	// ゲームスコア

	/*char score[10000];
	char life[10];*/

	// sprintf_s(score, "SCORE: %d ", gameScore_);
	// sprintf_s(life, "LIFE %d ", playerLife_);

	for (int l = 0; l < playerLife_; l++) {

		spriteLife_[l]->Draw();
	}

	/*debugText_->Print(score, 0, 10, 2);
	debugText_->Print(life, 400, 10, 2);
	debugText_->DrawAll();*/
}
// スコア数値の表示
void GameScene::DrawScore() {

	// 各桁の値を取り出す
	int eachNumber[5] = {};  // 各桁の値
	int number = gameScore_; // 表示する数字

	int keta = 10000; // 最初の桁
	for (int s = 0; s < 5; s++) {

		eachNumber[s] = number / keta; // 今の桁の値を求める
		number = number % keta;        // 次の桁以下の値を取り出す
		keta = keta / 10;              // 桁を進める
	}
	// 各桁の数値を描画

	for (int s = 0; s < 5; s++) {

		spriteNumber_[s]->SetSize({32, 64});
		spriteNumber_[s]->SetTextureRect({32.0f * eachNumber[s], 0}, {32, 64});
		spriteNumber_[s]->Draw();
	}
	spriteScore_->Draw();
}
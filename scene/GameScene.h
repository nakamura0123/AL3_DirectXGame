#pragma once

#include "Audio.h"
#include "DebugCamera.h"
#include "DebugText.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "Model.h"
#include "SafeDelete.h"
#include "Sprite.h"
#include "ViewProjection.h"
#include "WorldTransform.h"

/// <summary>
/// ゲームシーン
/// </summary>
class GameScene {

	// BG
	uint32_t textreHandleBG_ = 0;
	Sprite* spriteBG_ = nullptr;

	// ビュープロジェクション(共通)

	ViewProjection viewProjection_;

	// ステージ
	uint32_t textureHandleStage_ = 0;
	Model* modelStage_ = nullptr;
	WorldTransform worldTransformStage_[20];

	// プレイヤー
	uint32_t textureHandlePlayer_ = 0;
	Model* modelPlayer_ = nullptr;
	WorldTransform worldTransformPlayer_;

	// ビーム
	uint32_t textureHandleBeam_ = 0;
	Model* modelBeam_ = nullptr;
	WorldTransform worldTransformBeam_[10];

	bool isBeam_[10] = {}; // ビームフラグ

	// 敵
	uint32_t textureHandleEnemy_ = 0;
	Model* modelEnemy_ = nullptr;
	WorldTransform worldTransformEnemy_[10];

	int isEnemy_[10] = {}; // 敵フラグ

	// アイテム
	uint32_t textureHandleItem_ = 0;
	Model* modelItem_ = nullptr;
	WorldTransform worldTransformItem_[10];

	int isItem_[10] = {};

	// タイトル(スプライト)
	uint32_t textureHandleTitle_ = 0;
	Sprite* spriteTitle_ = nullptr;

	uint32_t textureHandleEnter_ = 0;
	Sprite* spriteEnter_ = nullptr;

	uint32_t textureHandleGameOver_ = 0;
	Sprite* spriteGameOver_ = nullptr;

	// サウンド
	uint32_t soundDataHandleTitleBGM_ = 0;      // タイトルBGM
	uint32_t soundDataHandleGamePlayerBGM_ = 0; // ゲームプレイBGM
	uint32_t soundDataHandleGameOverBGM_ = 0;   // ゲームオーバーBGM
	uint32_t soundDataHandleEnemyHitSE_ = 0;    // 敵ヒットSE
	uint32_t soundDataHandlePlayerHitSE_ = 0;   // プレイヤーヒット
	uint32_t voiceHandleBGM_ = 0;               // 音声再生ハンドル

	// スコア数値（スプライト）
	uint32_t textureHandleNumber_ = 0;
	Sprite* spriteNumber_[5] = {};

	// スコア（スプライト）
	uint32_t textureHandleScore_ = 0;
	Sprite* spriteScore_ = nullptr;

	// ライフ（スプライト）
	uint32_t textureHandleLife_ = 0;
	Sprite* spriteLife_[3] = {};

	//// プレイヤータイマー
	// uint32_t textureHandlePleyerTimer_ = 0;
	// Sprite* spritePleyerTimer_[3] = {};

public: // メンバ関数
	/// <summary>
	/// コンストクラタ
	/// </summary>
	GameScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameScene();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	void PlayerUpdate();

	void GamePlayUpdate();
	void GamePlayDraw3D();
	void GamePlayDraw2DBack();
	void GamePlayDraw2DNear();

	void BeamUpdate();
	void BeamMove();
	void BeamBorn();

	void EnemyUpdate();
	void EnemyMove();
	void EnemyBorn();
	void EnemyJump();

	void ItemUpdate();
	void ItemMove();
	void ItemBorn();
	void ItemJump();

	void Collision(); // 衝突判定
	void CollisionPlayerEnemy();
	void CollisionBeamEnemy();
	void CollisionPlayerItem();

	void TitleUpdate();     // タイトル更新
	void TitleDraw2DNear(); // タイトル2D

	void GameOverUpdate();     // ゲームオーバー更新
	void GameOverDraw2DNear(); // ゲームオーバー2D

	void StageUpdata(); // ステージ

	void DrawScore(); // スコア

	DebugText* debugText_ = nullptr;
	int gameScore_ = 0;
	int playerLife_ = 1;
	int gameTimer_ = 0; // 難易度のタイマー

	int sceneMode_ = 1; // シーンモード(0:ゲームプレイ　1:タイトル
	                    // 2:ゲームオーバー)

	int beamTimer_ = 0; // ビームタイマー

	float enemySpeed_[10] = {}; // 敵のスピード

	float enemyJumpSpeed_[10] = {}; // 敵ジャンプの移動速度

	int PlayerTimer_ = 0; // プレイヤータイマー

	int player = 1;

	float playerSpeed = 0.1f;

	int enemy = 10;

	int enemyTimer = 60;

	int enemyBorn = 2000;

	int item = 10;

	int itemTimer = 60;

	int itemBorn = 200;

	float itemSpeed_[10] = {};

	float itemJumpSpeed_[10] = {}; // 敵ジャンプの移動速度

	float level;

private: // メンバ変数
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;
	Audio* audio_ = nullptr;

	/// <summary>
	/// ゲームシーン用
	/// </summary>
};
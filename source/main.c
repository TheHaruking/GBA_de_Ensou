
#include <gba.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mappy.h>
#include "hal_eightinput.h"
#include "sound_play.h"
#include "chr003.h"
#include "common.h"
#include "graphic_mode4.h"
#include "obj_utils.h"


// 演奏を楽しむ "PLAYモード"
// 右から流れてくる音符の演奏に挑戦する "GAMEモード"
// ... の予定
#define MODE_PLAY	0
#define MODE_GAME	1
#define OBJ_LEFT9	9

// 黒鍵を1に。背景描画に使用
const int keycolor_tbl[12] = {
	0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 
};

// 映像用データ
typedef struct _VISUAL_PLAY_ {
	int			    frame;		// スクロール用カウンタ
	unsigned char** mem;		// 色保存
	int				mode_flag;	// "演奏モード" or "楽譜モード"
	OBJATTR*		icon_key;		// 左のアイコン
	OBJATTR*		icon_ab;		// AB押したときのアイコン
} VISUAL_PLAY, *PVISUAL_PLAY;

// 初期化
void InitVisualPlay(VISUAL_PLAY* vpd){
	int m_mem, n_mem;	// 確保するメモリの横と縦

	// メモリ初期化
	vpd->frame  = 0;
	vpd->mode_flag = MODE_PLAY;

	// 配列メモリ確保
	// mem[幅480][音程128]
	m_mem = SCREEN_WIDTH * 2;
	n_mem = 128;
	vpd->mem = (u8**)malloc_arr((void**)vpd->mem,  sizeof(u8), m_mem, n_mem);
}

void FinishVisualPlay(VISUAL_PLAY* vpd){
}

void InitGraphic(VISUAL_PLAY* vpd, OBJ_UTILS* oud){
	int n_key/*, n_ab*/;
	int pos_bottom;

	// BGパレットに色を設定 (とりあえず)
	BG_COLORS[0x00]  = RGB5( 0, 0, 0);
	BG_COLORS[0x01]  = RGB5(31,15, 0);
	BG_COLORS[0x02]  = RGB5( 0, 0,31);
	BG_COLORS[0x10]  = RGB5(13,13,13);
	BG_COLORS[0x11]  = RGB5( 7, 7, 7);

	// グラフィックデータをメモリへコピー
	dmaCopy((u16*)chr003Tiles, BITMAP_OBJ_BASE_ADR, chr003TilesLen);
	dmaCopy((u16*)chr003Pal,   OBJ_COLORS,          chr003PalLen);

	// obj_utilsが確保したメモリのアドレスをこちらに登録
	n_key	= 4 * OBJ_LEFT9;		// 1アイコン4つぶん 
//	n_ab	= 2 * OBJ_LEFT9 * 2;	// 1アイコン2つぶん * AとB (未使用)
	objReg(&vpd->icon_key, oud, 0);
	objReg(&vpd->icon_ab,  oud, n_key);

	// OBJ に データセット
	// ※下から順に置いていく。
	pos_bottom = 16 * (OBJ_LEFT9 - 1); // Y : 128
	for (int i = 0; i < OBJ_LEFT9; i++) {
		obj2draw(&vpd->icon_ab[i*4    ], 512 + 0x03, 0, pos_bottom - i*16);
		obj2draw(&vpd->icon_ab[i*4 + 2], 512 + 0x03, 0, pos_bottom - i*16+8);
		obj2pal(&vpd->icon_ab[i*4    ], 2); // 緑
		obj2pal(&vpd->icon_ab[i*4 + 2], 5); // 灰
	}

	// 画面左 ９箱
	for (int i = 0; i < OBJ_LEFT9; i++) {
		obj4draw(&vpd->icon_key[i*4], 512 + 0x86, 0, pos_bottom - i*16);
	}
}

void LightObj(OBJATTR* attr, int num, int enable){
	// 十字キー入力無しの場合、光らせない
	if (!enable) {
		num = -1;
	}
	// 画面左 ９箱
	for (int i = 0; i < OBJ_LEFT9; i++) {
		obj4pal(&attr[i*4], 4);
	}
	// 入力のあった方向を光らせる
	if (num >= 0) {
		obj4pal(&attr[num*4], 0);
	}
}

void LightObjAB(OBJATTR* attr, int num, int enable){
	int n;
	// abキー入力無しの場合、光らせない
	switch (enable) {
		case BTN_A:		n = 0;	break;
		case BTN_B:		n = 1;	break;
		default:		n = 0;	num = -1;	break;
	}
	// 画面左 ９箱
	for (int i = 0; i < OBJ_LEFT9; i++) {
		obj2chr(&attr[i*4  ], 512);
		obj2chr(&attr[i*4+2], 512);
	}
	// 入力のあった方向を光らせる
	if (num >= 0) {
		obj2chr(&attr[num*4 + n*2], 512 + 3);
	}
}

// スクロール用数値(frame)を計算
void MoveLine(VISUAL_PLAY* vpd) {
	// 0 ~ 239 をループする数値(スクロール用)
	// 演奏モード : →方向
	// 楽譜モード : ←方向
	switch (vpd->mode_flag) {
		case MODE_PLAY:
			if ((--vpd->frame) < 0) {
				vpd->frame = SCREEN_WIDTH - 1;
			}
			break;
		case MODE_GAME:
			if ((++vpd->frame) >= (SCREEN_WIDTH)) {
				vpd->frame = 0;
			}
			break;
	}
}

// 音の高さデータを色に変換
void DrawLines(VISUAL_PLAY* vpd, unsigned int y, int flag){
	int f = vpd->frame;
	// 上下反転させるため、128 から引いておく
	y = 128 - (y & 0x7f);

	// 音程に色をセット
	if (flag){
		vpd->mem[f][y] = 0x01;
	}
}

// 音の高さデータを色に変換(背景)
void DrawLinesBack(VISUAL_PLAY* vpd){
	int f = vpd->frame;

	// 背景色をセット
	for (int j = 0; j < 10; j++) {
		for (int i = 0; i < 12; i++) {
			vpd->mem[f][128 - (i + j*12)] = (keycolor_tbl[i]) ? 0x11 : 0x10;
		}
	}
}

// 上から128音程描画確認
void DrawLinesTest(VISUAL_PLAY* vpd){
	int f = vpd->frame;
	u16* dest = (u16*)(VRAM + SCREEN_WIDTH * 2 * f);
	dmaCopy((u16*)vpd->mem[f], (u16*)dest, 128);
}

// mem を、実際に描画する際の絵に変換
void ConvertMem(VISUAL_PLAY* vpd, GRAPHIC_MODE4* gmd){
	int n  = vpd->frame;
	int n2 = n + SCREEN_WIDTH;
	// 棒出現位置を右に15 ずらす
	int m  = (n + 15) % 240;
	int m2 = m + SCREEN_WIDTH;

	// とりあえずnote 83にして即時確認できるように。
	// 本当は、現在のキーボードのオクターブをみて決めないとダメ
	int note = 83;

	// セットした色を、実際の描画サイズ・向きに変換
	// i >> 3 して、棒を縦8 にしている
	// m は +15 されていて、棒出現位置を右に15 ずらす役割
	for (int i = 0; i < SCREEN_HEIGHT; i++){
		gmd->vram[i][m ] = vpd->mem[n][(i >> 3) + note]; 
		gmd->vram[i][m2] = vpd->mem[n][(i >> 3) + note]; 
	}

	// 左パネルが汚されるのを防ぐ
	for (int i = 0; i < SCREEN_HEIGHT; i++){
		gmd->vram[i][n ] = 0x00; 
		gmd->vram[i][n2] = 0x00;; 
	}
}


//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	SOUND_PLAY   	sp_data;
	GRAPHIC_MODE4	gmd_data;
	OBJ_UTILS		ou_data;
	VISUAL_PLAY  	vp_data;
	BUTTON_INFO  	b;

	// Init
	irqInit();
	irqEnable(IRQ_VBLANK);
	SetMode(4 | BG2_ON | OBJ_ON);

	// Init private
	halInitKeys(&b, 4, 5, 6, 7, 0, 1, 3, 2);
	InitSoundPlay(&sp_data);
	InitMode4(&gmd_data);
	InitObj(&ou_data);

	// Init Video
	InitVisualPlay(&vp_data);
	InitGraphic(&vp_data, &ou_data);

	// Sound Init
	REG_SOUNDCNT_X = 0x80;		// turn on sound circuit
	REG_SOUNDCNT_L = 0x1177;	// full volume, enable sound 1 to left and right
	REG_SOUNDCNT_H = 2;			// Overall output ratio - Full

	while (1) {
		// ボタン入力初期化
		scanKeys();
		halSetKeys(&b, keysHeld());

		// 音処理
		SoundPlay(&sp_data, &b);

		// 映像
		MoveLine(&vp_data);
		DrawLinesBack(&vp_data);
		DrawLines(&vp_data, sp_data.note, halIsAB_hold(&b));

		// 左のアイコン類
		LightObj(vp_data.icon_key,  sp_data.vector, halIsKey_hold(&b));
		LightObjAB(vp_data.icon_ab, sp_data.vector, halIsAB_hold(&b));

		// 書き込み処理
		ConvertMem(&vp_data, &gmd_data);
		FlushVramOfs(&gmd_data, vp_data.frame);
		FlushSprite(&ou_data);

		// 垂直同期
		VBlankIntrWait();
	}
}



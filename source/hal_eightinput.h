//---------------------------------------------------------------------------------
#ifndef _hal_eightinput_h_
#define _hal_eightinput_h_
//---------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------------
typedef struct _BUTTON_INFO_ {
	// 初期設定
	unsigned int x0, x1, y0, y1, a, b, ctr, esc; // sortinfo
	// ボタン関連
	unsigned int b0;				// new (押している間)
	unsigned int b1, b2;			// 押したとき、離したとき
	unsigned int b_old;				// old
	// 方向関連
	unsigned int k0, k1, k2, k_old;	// 方向キー単体の情報
	int			 key_8;				// ↑を 0 とした 7 までの 角度。
	int 		 f_key;				// ファーストキー。最初に押した方向。
	int  		 f_key_flag;		// 最初に押したフレームか
	int			 f_key_move;		// 傾け量。1(←) ~ 4(中) ~ 7(→)。0(逆)
	// 他
	unsigned int k12_stage;
} BUTTON_INFO, *PBUTTON_INFO;

extern int  test(BUTTON_INFO* b);
extern int  inc(int*);

// ボタンの並びを設定。
extern void halSetKeysSort(BUTTON_INFO* btn, unsigned int x0, unsigned int x1, unsigned int y0, unsigned int y1, unsigned int a, unsigned int b, unsigned int ctr, unsigned int esc);
// ボタンを送信。
extern void halSetKeys(BUTTON_INFO* btn, int src);
// 1 ~ 16 の 数値を出力。
extern int  halKeyToNum(BUTTON_INFO* btn);
extern int  halKeyAB(BUTTON_INFO* btn);
extern int  halKeyCtr4(BUTTON_INFO* btn);
extern int  halKeyCtr8(BUTTON_INFO* btn);
extern int  halKeyCtr12(BUTTON_INFO* btn);
extern int  halKey8(BUTTON_INFO* btn);

extern int  halIsA(BUTTON_INFO* btn);
extern int  halIsB(BUTTON_INFO* btn);
extern int  halIsAB(BUTTON_INFO* btn);
extern int  halIsKey(BUTTON_INFO* btn);

#endif
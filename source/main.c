
#include <gba.h>
#include <stdio.h>
#include <stdlib.h>

#include "hal_eightinput.h"

const unsigned short freq_tbl[84] = {
	0x0000, 0x0073, 0x00DF, 0x0146, 
	0x01A7, 0x0202, 0x0258, 0x02A9, 
	0x02F6, 0x033E, 0x0383, 0x03C3, 
	0x0400, 0x0439, 0x0470, 0x04A3, 
	0x04D3, 0x0501, 0x052C, 0x0555, 
	0x057B, 0x059F, 0x05C1, 0x05E2, 
	0x0600, 0x061D, 0x0638, 0x0651, 
	0x066A, 0x0680, 0x0696, 0x06AA, 
	0x06BD, 0x06D0, 0x06E1, 0x06F1, 
	0x0700, 0x070E, 0x071C, 0x0729, 
	0x0735, 0x0740, 0x074B, 0x0755, 
	0x075F, 0x0768, 0x0770, 0x0778, 
	0x0780, 0x0787, 0x078E, 0x0794, 
	0x079A, 0x07A0, 0x07A5, 0x07AB, 
	0x07AF, 0x07B4, 0x07B8, 0x07BC, 
	0x07C0, 0x07C4, 0x07C7, 0x07CA, 
	0x07CD, 0x07D0, 0x07D3, 0x07D5, 
	0x07D8, 0x07DA, 0x07DC, 0x07DE, 
	0x07E0, 0x07E2, 0x07E3, 0x07E5, 
	0x07E7, 0x07E8, 0x07E9, 0x07EB, 
	0x07EC, 0x07ED, 0x07EE, 0x07EF, 
};

// 設定モード
void SettingMode(){

}

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	int count;
	int moji, moji_old;
	int offset;
	int ab;
	int key;
	int ab_sounding;
	int env_time;
	int env_time_set;
	BUTTON_INFO b;

	// Init
	irqInit();
	irqEnable(IRQ_VBLANK);
	consoleDemoInit();

	// Init private
	count 		= 0;
	offset		= 8;
	key 		= 0;
	ab 			= 0;
	ab_sounding = 0;
	env_time 	= 0;
	env_time_set = 2;	// 余韻の長さ
	halSetKeysSort(&b, 4, 5, 6, 7, 0, 1, 3, 2);

	// Sound Init
	REG_SOUNDCNT_X = 0x80;		//turn on sound circuit
	REG_SOUNDCNT_L = 0x1177;	//full volume, enable sound 1 to left and right
	REG_SOUNDCNT_H = 2;			// Overall output ratio - Full

	REG_SOUND1CNT_L = (u16)0x007A;
	REG_SOUND1CNT_H = (u16)0xF781;
	REG_SOUND1CNT_X = (u16)0x01B8;

	while (1) {
		scanKeys();
		halSetKeys(&b, keysHeld());
		// Ctr + key
		switch(halKeyCtr4(&b)) {
			case 1: offset++; break;
			case 3: offset--; break;
			case 0: offset += 12; break;
			case 2: offset -= 12; break; 			
		}

		// キー を 12通りに変更
		if (halKeyCtr12(&b) >= 0)
			key = halKeyCtr12(&b);

		if (halIsKey(&b))
			moji = halKey8(&b) * 2;

		
		
		// 入力
		if (halIsAxB(&b) & (PUSH_AI | PUSH_BI)) {
			ab = (keysHeld() & KEY_B) ? 0 : 1;
			int mojiofs = (16 - moji) + offset + key + ab;
			ab_sounding = (halIsA(&b)) ? 0x10 : 0x20;
			env_time = 0;

			// sound test
			REG_SOUND1CNT_L = 0x0000;
			REG_SOUND1CNT_H = 0xF000;
			REG_SOUND1CNT_X = 0x8000 | freq_tbl[mojiofs];
			
			// 表示
			iprintf("%04X : %04X \n ", count, mojiofs);
			iprintf("ab_s : %04X \n ", ab_sounding);
			count++;
		}

		// キーオフ
		if (halIsAB_rrse(&b) & ab_sounding){
			env_time = env_time_set;
			REG_SOUND1CNT_H = 0xF000 | ((env_time & 7) << 8);
			iprintf("test : rrse \n ", count);
		}

		if (halIsAxB(&b))
			iprintf("yes %d \n ",halIsAxB(&b) );
		VBlankIntrWait();
	}
}



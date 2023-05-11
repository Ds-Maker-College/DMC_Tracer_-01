/////////////////////////////////////////////////////
//    Sample soft for DMC_tracer dijital
// 
//    Copyright(C) 2021 <DMC>
//    Copyright(C) 2021 M.Hirai, Y.Okada
//    All rights reserved.
// 
//    License: Apache License, Version 2.0
//    https://www.apache.org/licenses/LICENSE-2.0
// 
/////////////////////////////////////////////////////

////////ハードウェアパラメータ/////////
// ピン設定
//LED
#define LED       13

//プッシュSW
#define SW_L      7 //スイッチ左
#define SW_R      8 //スイッチ右

////////ソフトウェアパラメータ/////////

// 定数設定-------------------
#define SW_OFF  HIGH
#define SW_ON   LOW
//---------------------------

//変数宣言--------------------

//-----------------------------

//初期設定関数
void setup() {
  //  put your setup code here, to run once:
  //  動時最初に１回だけ走るプログラム

  //IOポート設定
  pinMode(LED,OUTPUT);
  pinMode(SW_R,INPUT_PULLUP); //スイッチポートの入力はプルアップ設定
  pinMode(SW_L,INPUT_PULLUP); //スイッチポートの入力はプルアップ設定
  
}

//メインループ
void loop() {

  //　左スイッチ左がオン　
  if( digitalRead(SW_L) == SW_ON ){
    
    digitalWrite(LED,HIGH);   //LED点灯
    }
  
  //　右スイッチがオン
  else if( digitalRead(SW_R) == SW_ON ){
    
    digitalWrite(LED,LOW);   //LED消灯
  }
    
  // 上記以外
  else{}
 
 }

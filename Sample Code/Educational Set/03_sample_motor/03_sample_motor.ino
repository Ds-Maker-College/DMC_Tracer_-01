/////////////////////////////////////////////////////
//    Sample soft for DMC_tracer motor
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

//ラインセンサAD
#define LS_L2     3 //外側左センサAD (黒　0->1023　白)
#define LS_L1     2 //内側左センサAD (黒　0->1023　白)
#define LS_R1     1 //内側右センサAD (黒　0->1023　白)
#define LS_R2     0 //外側右センサAD (黒　0->1023　白)

//可変抵抗
#define VR_L      7 //左可変抵抗AD(0->1023)
#define VR_R      6 //右可変抵抗AD(0->1023)

#define MT_PWM_L  5 //左モータPWM (停止 0 -> 255 早)
#define MT_DIR_L  3 //左モータ回転方向 (バック 0 -> 1　前進)
#define MT_PWM_R  6 //右モータPWM (停止 0 -> 255 早)
#define MT_DIR_R  2 //右モータ回転方向 (前進 0 -> 1　バック)

////////ソフトウェアパラメータ/////////

// 定数設定-------------------------------------------
#define SW_OFF  HIGH
#define SW_ON   LOW
#define MT_FORWARD_L  HIGH
#define MT_REVERSE_L  LOW
#define MT_FORWARD_R  LOW
#define MT_REVERSE_R  HIGH
//---------------------------------------------------

//変数宣言--------------------
long  line_sensor_l1;
long  line_sensor_r1;
long  line_sensor_l2;
long  line_sensor_r2;

long  vr_ad_l;
long  vr_ad_r;

long  vel_set_l;
long  vel_set_r;

//---------------------------

//初期設定関数
void setup() {
  //  put your setup code here, to run once:
  //  動時最初に１回だけ走るプログラム

  //IOポート設定
  pinMode(LED,OUTPUT);
  pinMode(SW_R,INPUT_PULLUP);
  pinMode(SW_L,INPUT_PULLUP);

  //モータの回転方向を設定
  digitalWrite(MT_DIR_L,MT_FORWARD_L);//前進
  digitalWrite(MT_DIR_R,MT_FORWARD_R);//前進

  //シリアル通信を開始
  Serial.begin(9600);
}

//メインループ
void loop() {

  //ラインセンサを読み込む
  line_sensor_l1 = analogRead(LS_L1);
  line_sensor_r1 = analogRead(LS_R1);
  line_sensor_l2 = analogRead(LS_L2);
  line_sensor_r2 = analogRead(LS_R2);

  //ボリュームの読み込み
  vr_ad_l  = analogRead(VR_L);
  vr_ad_r  = analogRead(VR_R);  

  //シリアル通信でセンサの値を確認
  Serial.print(line_sensor_l2);   //ラインセンサ左の外側の値を表示
  Serial.print("\t");
  Serial.print(line_sensor_l1);   //ラインセンサ左の内側の値を表示
  Serial.print("\t");
  Serial.print(line_sensor_r1);   //ラインセンサ右の内側の値を表示
  Serial.print("\t");
  Serial.print(line_sensor_r2);   //ラインセンサ右の外側の値を表示
  Serial.print("\t");

  Serial.print(vr_ad_l);          //左ボリュームの値を表示
  Serial.print("\t");
  Serial.print(vr_ad_r);          //右ボリュームの値を表示
  Serial.print("\t");
  Serial.println("");                         // 改行
  
  //ボリュームのAD値(10bit)をPWM値(8bit)に変換
  vel_set_l = vr_ad_l * 255 / 1023;
  vel_set_r = vr_ad_r * 255 / 1023;

  //モーター回転テストコード
  digitalWrite(MT_DIR_L, MT_FORWARD_L); //左モーター前進設定
  digitalWrite(MT_DIR_R, MT_FORWARD_R); //右モーター前進設定
  analogWrite(MT_PWM_L, vel_set_l);     //左モーター速度　PWM値設定
  analogWrite(MT_PWM_R, vel_set_r);     //右モーター速度　PWM値設定
 
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

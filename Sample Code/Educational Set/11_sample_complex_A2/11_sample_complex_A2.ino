/////////////////////////////////////////////////////
//    Sample soft for DMC_tracer simple complex A2 
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
//OLED 関係 基本的に触らない
#include<Wire.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

// 定数設定--------------------
#define SW_OFF  HIGH
#define SW_ON   LOW
#define MT_FORWARD_L  HIGH
#define MT_REVERSE_L  LOW
#define MT_FORWARD_R  LOW
#define MT_REVERSE_R  HIGH
//----------------------------

//パラメータ設定-------------------------------------------
#define THREAD_LINE 200 //IRセンサがゴールラインを認識する閾値
#define LAP_NUM 2       //周回数(レースモード時)
//-------------------------------------------------------

//変数宣言---------------------
long  line_sensor_l1;
long  line_sensor_r1;
long  line_sensor_l2;
long  line_sensor_r2;

long  vr_ad_l;
long  vr_ad_r;

long  vel_set_l;
long  vel_set_r;

unsigned long start_time;
unsigned long total_time;
unsigned long lap_time[LAP_NUM+1];
unsigned long boot_time;
byte n_lap = 0;

//----------------------------

//初期設定関数
void setup() {
  // put your setup code here, to run once:
  //　起動時最初に１回だけ走るプログラム

  //IOポート設定
  pinMode(LED,OUTPUT);
  pinMode(SW_R,INPUT_PULLUP);
  pinMode(SW_L,INPUT_PULLUP);

  //モータの回転方向を設定
  digitalWrite(MT_DIR_L,MT_FORWARD_L);  //前進
  digitalWrite(MT_DIR_R,MT_FORWARD_R);  //前進
  
  //シリアル通信を開始
  Serial.begin(9600);

  // Wire(Arduino-I2C)の初期化
  Wire.begin();
  
  //OLED初期化　Address 0x3C for 128x32
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){ 
    Serial.println(F("SSD1306 allocation failed"));
    while(1); // Don't proceed, loop forever
  }

  //機種名をOLEDに表示----------------
  display.clearDisplay();
  display.setTextSize(2); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(4,7);
  display.print(F("DMC TRACER"));
  display.display();
  delay(1000);
 //----------------------------------

  //走行前にセンサの確認
  while(1){

    //右スイッチが推されたらループを出る
    if( digitalRead(SW_R) == SW_ON ){break;}
    
    //ラインセンサを読み込む
    line_sensor_l1 = analogRead(LS_L1);
    line_sensor_r1 = analogRead(LS_R1);
    line_sensor_l2 = analogRead(LS_L2);
    line_sensor_r2 = analogRead(LS_R2);

    //ボリュームの読み込み
    vr_ad_l  = analogRead(VR_L);
    vr_ad_r  = analogRead(VR_R);

    //ボリュームのAD値(10bit)をPWM値(8bit)に変換
    vel_set_l = vr_ad_l * 255 / 1023;
    vel_set_r = vr_ad_r * 255 / 1023;

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

    Serial.print(vel_set_l);        //左輪速度設定の値を表示
    Serial.print("\t");
    Serial.print(vel_set_r);        //右輪速度設定の値を表示
    Serial.print("\t");
    Serial.println("");   // 改行

    //OLEDに各パラメータ値出力(デバッグ用)
    display.clearDisplay();
    display.setTextSize(1); // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);

    //一段目の表示　ソフトのバージョン
    display.setCursor(0, 0);
    display.print(F("Ver:Complex Course A2"));
    
    //二段目の表示　ボリュームの速度設定
    display.setCursor(0, 11);
    display.print(F("VsetL:"));
    display.print(vel_set_l);
    display.print(", ");
    display.print(F("VsetR:"));
    display.print(vel_set_r);
    display.print("\n");

    //三段目の表示　ラインセンサの値
    display.setCursor(0,22);
    display.println(F("Sens"));
    display.setCursor(35,22);
    display.println(line_sensor_l2, DEC);
    display.setCursor(60,22);
    display.println(line_sensor_l1, DEC);
    display.setCursor(85,22);
    display.println(line_sensor_r1, DEC);
    display.setCursor(110,22);
    display.println(line_sensor_r2, DEC);
    display.display();
    delay(10);
  }
}

//メインループ
void loop() {

  // put your main code here, to run repeatedly:

  int   motor_set_L;  //左モーターの速度セット変数
  int   motor_set_R;  //右モーターの速度セット変数
  int   steering1;    //ステアリング計算の変数
  int   steering2;    //ステアリング計算の変数

  long thread_l1 = 250; //左内側センサの白黒閾値
  long thread_r1 = 250; //左内側センサの白黒閾値
  long thread_l2 = 250; //左外側センサの白黒閾値
  long thread_r2 = 250; //左外側センサの白黒閾値

  static bool lineout_ready_L = 0; // 記憶型変数でラインアウト待ち　左用
  static bool lineout_ready_R = 0; // 記憶型変数でラインアウト待ち　右用
  
  //繰り返し走るプログラム

  //ラインセンサを読み込む
  line_sensor_l1 = analogRead(LS_L1);
  line_sensor_r1 = analogRead(LS_R1);
  line_sensor_l2 = analogRead(LS_L2);
  line_sensor_r2 = analogRead(LS_R2);

  //ステアリング制御用計算
  steering1 = (int)line_sensor_l1 - (int)line_sensor_r1;
  steering2 = (int)line_sensor_l2 - (int)line_sensor_r2;

  motor_set_L = vel_set_l + (steering1 /14) + (steering2 /9);
  motor_set_R = vel_set_r - (steering1 /14) - (steering2 /9);

  //ラインセンサデジタル処理
  //センサ値に応じた走行パターンを決定----------------------------------
 
  //センサが左から 黒白白白 と認識しているとき
  if((line_sensor_l2 < thread_l2) && (line_sensor_l1 > thread_l1) && (line_sensor_r1 > thread_r1) && (line_sensor_r2 > thread_r2)){
    
    //左折大
    lineout_ready_L = 1;  //左ラインアウトレディフラグを上げる
    lineout_ready_R = 0;
  }
  //センサが左から 白白白黒 と認識しているとき
  else if((line_sensor_l2 > thread_l2) && (line_sensor_l1 > thread_l1) && (line_sensor_r1 > thread_r1) && (line_sensor_r2 < thread_r2)){

    //右折大
    lineout_ready_L = 0;
    lineout_ready_R = 1;  //右ラインアウトレディフラグを上げる
  }
  else{}
  
  //左ラインアウト対応
  if(lineout_ready_L == 1){

    //全センサが白 で左ラインアウト 左緊急旋回
    if((line_sensor_l2 > thread_l2) && (line_sensor_l1 > thread_l1) && (line_sensor_r1 > thread_r1) && (line_sensor_r2 > thread_r2)){
      digitalWrite(LED,1);      //LED点灯
      analogWrite(MT_PWM_L,0);
      analogWrite(MT_PWM_R,255);
      delay(100);
      digitalWrite(LED,0);      //LED消灯
      lineout_ready_L = 0;      //左ラインアウトレディフラグを解除
    }
    else if((line_sensor_l2 < thread_l2) && (line_sensor_l1 < thread_l1) && (line_sensor_r1 > thread_r1) && (line_sensor_r2 > thread_r2)){
      analogWrite(MT_PWM_L,motor_set_L);
      analogWrite(MT_PWM_R,motor_set_R);
      lineout_ready_L = 0;      //左ラインアウトレディフラグを解除
    }
    else{
      analogWrite(MT_PWM_L,motor_set_L);
      analogWrite(MT_PWM_R,motor_set_R);
    }
  }
   
  //右ラインアウト対応
  else if(lineout_ready_R == 1){

    //全センサが白 で右ラインアウト 右緊急旋回
    if((line_sensor_l2 > thread_l2) && (line_sensor_l1 > thread_l1) && (line_sensor_r1 > thread_r1) && (line_sensor_r2 > thread_r2)){   
      digitalWrite(LED,1);      //LED点灯
      analogWrite(MT_PWM_L,255);
      analogWrite(MT_PWM_R,0);
      delay(100);
      digitalWrite(LED,0);      //LED消灯
      lineout_ready_R = 0;      //右ラインアウトレディフラグを解除
    }
    else if((line_sensor_l2 > thread_l2) && (line_sensor_l1 > thread_l1) && (line_sensor_r1 < thread_r1) && (line_sensor_r2 < thread_r2)){
      analogWrite(MT_PWM_L,motor_set_L);
      analogWrite(MT_PWM_R,motor_set_R);
      lineout_ready_R = 0;      //左ラインアウトレディフラグを解除
    }
    else{
      analogWrite(MT_PWM_L,motor_set_L);
      analogWrite(MT_PWM_R,motor_set_R);
    }
  }
  
  else{
    analogWrite(MT_PWM_L,motor_set_L);
    analogWrite(MT_PWM_R,motor_set_R);
  }

  //デバック用コード
/*  Serial.print(steering1);   //ステアリング１の値を表示
  Serial.print("\t");
  Serial.print(steering2);   //ステアリング２の値を表示
  Serial.print("\t");

  Serial.print(motor_set_L);  //左モータ速度変数の値を表示
  Serial.print("\t");
  Serial.print(motor_set_R);  //右モータ速度変数の値を表示
  Serial.print("\t");
  Serial.print("\t");
  Serial.println("");   // 改行
*/

  //↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ここから編集NG↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
  //走行時間や周回計測をしています
  boot_time = millis();
  goal_lap_check();//編集禁止
  time_check();//編集禁止
  //周回数が所定以上で停止
  if (n_lap > LAP_NUM){
    analogWrite(MT_PWM_R,0);
    analogWrite(MT_PWM_L,0);
    disp_lap_time();
    while(1)
    {
      //ゴール時無限ループ
    }
  }
  //↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑ここまで編集NG↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑
 
}

//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓以下関数編集禁止↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

//走行時間処理(編集禁止)
void time_check(void)
{
  static unsigned long boot_time_prev = 0;
  //レース中、タイマー計算
  if ((n_lap > 0) && (n_lap <= LAP_NUM)){
    total_time = boot_time - start_time;
  }
  //10msに1回ディスプレイ更新
  if(boot_time - boot_time_prev < 10)
  {
  }else{
    boot_time_prev = boot_time;
    disp_lap_time();
  }
}

//ゴール判定とラップカウンタ(編集禁止)
void goal_lap_check(void){

  bool flg_sgline;
  bool static flg_sgline_prev = 0; 
  byte static disp_count;
  
  //ゴールラインのチェック処理----------------------------------------
  bool t_ls11 = (line_sensor_l1 < THREAD_LINE);
  bool t_lsr1 = (line_sensor_r1 < THREAD_LINE);
  bool t_lsl2 = (line_sensor_l2 < THREAD_LINE);
  bool t_lsr2 = (line_sensor_r2 < THREAD_LINE);

  flg_sgline = 0;
  //すべてのセンサでラインを検出したとき、ゴールラインの検出とする。
  if ((t_ls11 == 1) && (t_lsr1 == 1) && (t_lsl2 == 1) && (t_lsr2 == 1)) 
  {
    flg_sgline = 1;
  } 
  bool t_line = LOW;
  //ゴールラインを検出　かつ　前回ゴールラインを検出していないとき
  //ゴールライン検出フラグをたて、ラップ数をインクリメント、ラップタイムを記録する。
  if ((flg_sgline == 1) && (flg_sgline_prev == 0)) {
    t_line = HIGH;
    if (n_lap == 0) {
      start_time = boot_time;
    } else if (n_lap <= LAP_NUM){
      if (n_lap > 1) {
        lap_time[n_lap] = boot_time - lap_time[n_lap - 1];
      } else {
        lap_time[n_lap] = boot_time - start_time;
      }
    }
    n_lap = n_lap + 1;
  }

  //前回ゴールライン検出フラグを更新
  flg_sgline_prev = flg_sgline;
}

//走行時間表示処理(編集禁止)
void disp_lap_time(void)
{
  display.clearDisplay();

  display.setTextSize(2); 
  display.setCursor(0, 0);
  display.print(F("T:"));

  float tt = ((float)total_time /1000.0);//時間の単位を変換[ms->s]

  display.print(tt,2);

  display.setTextSize(2); 
  display.setCursor(0, 16);
  display.print(F("L:"));

  display.setCursor(40, 16);
  display.print(n_lap);
  display.display();
}

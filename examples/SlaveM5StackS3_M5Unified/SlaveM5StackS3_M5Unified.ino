#include <M5Unified.h>
#include <ESPNowAutoPairing.h>

ESPNowAutoPairing espNow(SLAVE);

#define NES_COMMAND   0x10

#define NOT_PAIRED 0
#define PAIRING 1
#define PAIRED 2

static int PAIR_STATUS = NOT_PAIRED;

pairing_message_t msg;
bool RCVD = false;

// ユーザー受信ハンドラ
void OnDataRecvUser(const uint8_t *mac_addr, const uint8_t *incomingData, int data_len){
  if (data_len >= (int)sizeof(pairing_message_t)) {
    RCVD = true;
    memcpy(&msg, incomingData, sizeof(msg));
  }
}

void setup() {
  M5.begin(); // M5Unifiedの初期化
  Serial.begin(115200);
  
  espNow.begin();
  espNow.setUserRecvCallback(OnDataRecvUser);

  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(BLUE, BLACK);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.print("ESP-NOW Slave Mode");
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setCursor(10, 30);
  M5.Lcd.print("Hold BtnA to Pairing mode");
}

void loop() {
  M5.update();

  // ボタンAの長押しでペアリングモード開始
  if (M5.BtnA.pressedFor(1000)) {
    // ペアリングデータをクリア
    Serial.println("Clear pairing data...");
    espNow.clearPairingData();
    delay(600);
    // ペアリングモード開始
    espNow.startPairingMode();
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.print("Pairing...");
    PAIR_STATUS = PAIRING;
  }

  if (espNow.isPaired()) {
    if (PAIR_STATUS != PAIRED) {  // 状態変化時のみクリア
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(GREEN, BLACK);
      M5.Lcd.setCursor(10, 10);
      M5.Lcd.print("Paired (ESP-NOW: Slave)");
    }
    PAIR_STATUS = PAIRED;
    // ペアリング済みMACアドレスを表示
    uint8_t* pairedMac = espNow.getPairedMacAddress();
    M5.Lcd.setTextColor(MAGENTA, BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.printf("Master MAC:\n %02X:%02X:%02X:%02X:%02X:%02X",
                  pairedMac[0], pairedMac[1], pairedMac[2],
                  pairedMac[3], pairedMac[4], pairedMac[5]);

    // スレーブのMACアドレスをデータバイトとして送信（8バイト枠に収める）
    static unsigned long lastDataSendTime = 0;
    if (millis() - lastDataSendTime > 2000) { // 2秒ごとにデータを送信
      pairing_message_t msg;
      msg.type = NES_COMMAND;
      uint8_t myMac[6];
      WiFi.macAddress(myMac);
      // 先頭6バイトにMACを格納、残りは0埋め
      memcpy(msg.data, myMac, 6);
      msg.data[6] = 0x00;
      msg.data[7] = 0x00;
      espNow.sendData((uint8_t*)&msg, sizeof(msg));
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(CYAN, BLACK);
      M5.Lcd.setCursor(10, 70); // 送信メッセージ表示位置
      M5.Lcd.printf("Sent data:\n %02X:%02X:%02X:%02X:%02X:%02X",
                    myMac[0], myMac[1], myMac[2], myMac[3], myMac[4], myMac[5]);

      Serial.printf("Master MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  pairedMac[0], pairedMac[1], pairedMac[2],
                  pairedMac[3], pairedMac[4], pairedMac[5]);

      Serial.printf("Sent DATA: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    myMac[0], myMac[1], myMac[2], myMac[3], myMac[4], myMac[5]);
      lastDataSendTime = millis();
    }
  }

  if(RCVD == true && PAIR_STATUS == PAIRED){
    if (msg.type == NES_COMMAND) {
      // Masterからの8バイトのMacアドレスを16進で表示
      M5.Lcd.setTextSize(2);
      M5.Lcd.setCursor(10, 110);
      M5.Lcd.setTextColor(YELLOW, BLACK);
      M5.Lcd.printf("Received data:\n %02X %02X %02X %02X %02X %02X %02X %02X",
                    msg.data[0], msg.data[1], msg.data[2], msg.data[3],
                    msg.data[4], msg.data[5], msg.data[6], msg.data[7]);

      Serial.printf("Received: %02X %02X %02X %02X %02X %02X %02X %02X\n",
                    msg.data[0], msg.data[1], msg.data[2], msg.data[3],
                    msg.data[4], msg.data[5], msg.data[6], msg.data[7]);
    }
    RCVD = false;
  }

  delay(10);
}

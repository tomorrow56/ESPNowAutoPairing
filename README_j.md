# ESP-NOW Auto Pairing Library

ESP32デバイス間でESP-NOWを使用した自動ペアリング機能を提供するライブラリです。

## 特徴

- Master/Slaveの役割をインスタンス時に選択可能
- ペアリング結果を不揮発性メモリ（EEPROM）に自動保存
- ペアリングデータの読み出し・クリア機能
- 簡単なAPIでESP-NOW通信を実現

## インストール

1. このライブラリフォルダを Arduino IDE の libraries フォルダにコピーしてください
2. Arduino IDE を再起動してください

## 基本的な使用方法

### 1. ライブラリのインクルード

```cpp
#include <ESPNowAutoPairing.h>
```

### 2. インスタンスの作成

```cpp
// Masterとして使用する場合
ESPNowAutoPairing espNow(MASTER);

// Slaveとして使用する場合
ESPNowAutoPairing espNow(SLAVE);
```

### 3. 初期化

```cpp
void setup() {
    Serial.begin(115200);
    espNow.begin();
}
```

## API リファレンス

### コンストラクタ

```cpp
ESPNowAutoPairing(DeviceRole role)
```

- `role`: `MASTER` または `SLAVE` を指定

### メソッド

#### `void begin()`
ライブラリを初期化します。setup()内で呼び出してください。

#### `void startPairingMode()`
ペアリングモードを開始します。

#### `void stopPairingMode()`
ペアリングモードを終了します。

#### `bool isPaired()`
ペアリング済みかどうかを確認します。

**戻り値:**
- `true`: ペアリング済み
- `false`: 未ペアリング

#### `uint8_t* getPairedMacAddress()`
ペアリング済みのMACアドレスを取得します。

**戻り値:**
- ペアリング済みMACアドレスへのポインタ（6バイト）

#### `void clearPairingData()`
保存されているペアリングデータをクリアします。

#### `void sendData(uint8_t* data, size_t len)`
ペアリング済みデバイスにデータを送信します。

**パラメータ:**
- `data`: 送信するデータへのポインタ
- `len`: データのサイズ

#### `void setUserRecvCallback(void (*callback)(const uint8_t*, const uint8_t*, int))`
ユーザー定義の受信コールバック関数を設定します。

**パラメータ:**
- `callback`: 受信時に呼び出される関数ポインタ

## ペアリング手順

### Master側の手順
1. `startPairingMode()` を呼び出してペアリングモードに入る
2. PAIR_REQUESTをブロードキャスト送信する
3. SlaveからのPAIR_RESPONSEを受信する
4. PAIR_CONFIRMを送信してペアリング完了

### Slave側の手順
1. `startPairingMode()` を呼び出してペアリングモードに入る
2. MasterからのPAIR_REQUESTを待機する
3. PAIR_REQUESTを受信したらPAIR_RESPONSEを返す
4. MasterからのPAIR_CONFIRMを受信してペアリング完了

## メッセージ構造体

```cpp
typedef struct {
    uint8_t type;        // メッセージタイプ
    uint8_t mac[6];      // 送信者のMACアドレス
    uint8_t data[8];     // 追加データ
} pairing_message_t;
```

### メッセージタイプ

- `PAIR_REQUEST (0x01)`: ペアリング要求
- `PAIR_RESPONSE (0x02)`: ペアリング応答
- `PAIR_CONFIRM (0x03)`: ペアリング確認
- `NES_COMMAND (0x10)`: 通常のデータ通信

## コールバック関数の重要な注意事項

**⚠️ 重要**: ESP-NOWのコールバック関数内でM5.Lcdによる描画処理を行うとメモリ違反が発生します。

### 問題の原因
- ESP-NOWコールバック関数は割り込みコンテキストで実行される
- M5.Lcdの描画処理は内部でタスク切り替えやメモリ割り当てを行う
- SPI通信の競合状態が発生する可能性がある

### 正しい実装方法

```cpp
// グローバル変数でデータを管理
pairing_message_t msg;
bool RCVD = false;

// コールバック関数内では最小限の処理のみ
void OnDataRecvUser(const uint8_t *mac_addr, const uint8_t *incomingData, int data_len) {
  if (data_len >= (int)sizeof(pairing_message_t)) {
    RCVD = true;
    memcpy(&msg, incomingData, sizeof(msg));
  }
}

// メインループ内で安全に描画処理
void loop() {
  if(RCVD == true && PAIR_STATUS == PAIRED) {
    if (msg.type == NES_COMMAND) {
      // ここで安全に描画処理を実行
      M5.Lcd.printf("Received: %02X %02X...", msg.data[0], msg.data[1]);
    }
    RCVD = false;  // フラグをリセット
  }
}
```

## 使用例

詳細な使用例は `examples` フォルダを参照してください：

- `MasterAtomS3_M5Unified`: AtomS3を使用したMaster側の実装例
- `SlaveM5StackS3_M5Unified`: M5StackS3を使用したSlave側の実装例

## サンプルコードの特徴

### 画面表示の改善
- ペアリング状態に応じた適切な画面表示
- 状態変化時のみ画面クリアを実行（ちらつき防止）
- MACアドレスと送受信データの視覚的表示

### 操作方法
- **Master/Slave共通**: ボタンA長押し（1秒）でペアリングモード開始
- ペアリングデータの自動クリア機能
- リアルタイムでのデータ送受信表示

### データ通信
- 2秒間隔でのMACアドレス送信
- 8バイトデータフォーマットでの通信
- シリアル出力による詳細ログ

## 注意事項

- EEPROMを7バイト使用します（MACアドレス6バイト + ペアリング状態1バイト）
- ペアリングモード中は通常のデータ通信は行われません
- 一度ペアリングが完了すると、電源を切っても設定は保持されます
- **コールバック関数内では描画処理を行わないでください**（メモリ違反の原因）
- M5Unifiedライブラリとの組み合わせで動作確認済み

## ライセンス

このライブラリはMITライセンスの下で公開されています。


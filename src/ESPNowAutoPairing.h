#ifndef ESPNOW_AUTOPAIRING_H
#define ESPNOW_AUTOPAIRING_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <EEPROM.h>

#define EEPROM_SIZE 7 // MACアドレス6バイト + ペアリング状態1バイト

// ペアリングメッセージ構造体
typedef struct {
    uint8_t type;        // メッセージタイプ
    uint8_t mac[6];      // 送信者のMACアドレス
    uint8_t data[8];     // 追加データ
} pairing_message_t;

// メッセージタイプ定義
#define PAIR_REQUEST  0x01
#define PAIR_RESPONSE 0x02
#define PAIR_CONFIRM  0x03
#define NES_COMMAND   0x10

// ペアリング状態
#define PAIRING_NONE    0x00
#define PAIRING_PAIRED  0x01

enum DeviceRole {
    MASTER,
    SLAVE
};

class ESPNowAutoPairing {
public:
    ESPNowAutoPairing(DeviceRole role);
    void begin();
    void startPairingMode();
    void stopPairingMode();
    bool isPaired();
    uint8_t* getPairedMacAddress();
    void clearPairingData();
    void sendData(uint8_t* data, size_t len);

    // ユーザー受信コールバックを登録（ライブラリ内部処理の後に呼ばれる）
    typedef void (*UserRecvCallback)(const uint8_t *mac_addr,
                                     const uint8_t *incomingData,
                                     int data_len);
    void setUserRecvCallback(UserRecvCallback cb);

private:
    DeviceRole _role;
    uint8_t _pairedMacAddress[6];
    uint8_t _pairingStatus;
    bool _pairingMode;
    unsigned long _lastSendTime;
    esp_now_peer_info_t _peerInfo;

    void loadPairingData();
    void savePairingData();
    void addPeer(const uint8_t* mac_addr);
    void deletePeer(const uint8_t* mac_addr);

    // ESP-NOWコールバック関数
    static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
    static void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int data_len);

    // 外部からアクセスできるようにするためのフレンド関数または静的メンバ
    static ESPNowAutoPairing* _instance;
    static UserRecvCallback _userRecvCb;
};

#endif // ESPNOW_AUTOPAIRING_H



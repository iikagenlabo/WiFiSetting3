//  WiFiの設定
#ifndef WIFISETUP_H
#define WIFISETUP_H

#include <WiFiMulti.h>
#include <IPAddress.h>
#include <Preferences.h>

class WiFiSetup
{
public:
    WiFiSetup() : apIP(192, 168, 4, 1) {}
    ~WiFiSetup();

    bool setup();
    //  WiFiの設定モード
    void setupMode();
    void clearSettings();
    void handleClient();

private:
    Preferences preferences; //  設定を保存する場所

    String wifi_ssid;
    String wifi_password;

    const IPAddress apIP;   //(192, 168, 4, 1);
    const char* apSSID = "M5STACK_SETUP";
    bool settingMode = true;
    String ssidList;        //  Web表示用WiFiSSIDリスト
    String ssidSettings;    //  設定済みSSIDリスト

    WiFiMulti wifiMulti;

    //  設定の取得
    void setSSIDSetting(int num, String ssid);
    void setPASSWDSetting(int num, String passwd);
    String getSSIDSetting(int num);
    String getPASSWDSetting(int num);

    void clearSettingsNum(int num);

    //  設定を復元
    bool restoreConfig(int num);
    //  接続確認
    bool checkConnection();

    void startWebServer();

    String makePage(String title, String contents);
    String urlDecode(String input);
};

#endif
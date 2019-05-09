#include <M5Stack.h>
#include <Preferences.h>

#include "WiFiSetup.h"
//  WiFi設定クラス
WiFiSetup wifi;

//  起動シーケンス
void setup()
{
    M5.begin();

    Serial.begin(115200);
    delay(10);

    //  WiFi接続開始
    if (wifi.setup() == false)
    {
    }
}

//  ループ実行処理
void loop()
{
    wifi.handleClient();
}

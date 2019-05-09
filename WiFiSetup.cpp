//  WiFiの設定

#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <Preferences.h>

#include "WiFiSetup.h"

#include "WebServer.h"

WebServer webServer(80);

//  設定保存名
const char* PREF_NAME = "wifi-config";

String WIFI_SSID("WIFI_SSID");
String WIFI_PASSWD("WIFI_PASSWD");

//  終了処理
WiFiSetup::~WiFiSetup()
{
    preferences.end();
}

//  設定開始
bool WiFiSetup::setup()
{
    preferences.begin(PREF_NAME);
    delay(10);

    //  保存されている設定で接続する
    for(int i = 0; i <= 3; i++)
    {
        if (restoreConfig(i))
        {
            //  接続が確立されたらセットアップ終了
            if (checkConnection())
            {
                settingMode = false;
                //  default設定ではない場合は新しい設定を書き戻す
                if(i != 0)
                {
                    setSSIDSetting(0, wifi_ssid);
                    setPASSWDSetting(0, wifi_password);
                }

                startWebServer();
                return true;
            }
        }
    }

    //  つながらない場合はWiFiの設定モードに進む
    setupMode();

    return false;
}

void WiFiSetup::handleClient()
{
    webServer.handleClient();
}

//  WiFiの設定を初期化する
void WiFiSetup::clearSettings()
{
    preferences.remove(WIFI_SSID.c_str());
    preferences.remove(WIFI_PASSWD.c_str());

    clearSettingsNum(1);
    clearSettingsNum(2);
    clearSettingsNum(3);
}

void WiFiSetup::clearSettingsNum(int num)
{
    preferences.remove((WIFI_SSID + String(num)).c_str());
    preferences.remove((WIFI_PASSWD + String(num)).c_str());
}

//  WiFi設定の保存と取得
void WiFiSetup::setSSIDSetting(int num, String ssid)
{
    String num_str(num);
    if(num == 0) num_str = "";
    preferences.putString((WIFI_SSID + num_str).c_str(), ssid);
}

void WiFiSetup::setPASSWDSetting(int num, String passwd)
{
    String num_str(num);
    if(num == 0) num_str = "";
    preferences.putString((WIFI_PASSWD + num_str).c_str(), passwd);
}

String WiFiSetup::getSSIDSetting(int num)
{
    String num_str(num);
    if(num == 0) num_str = "";
    String str = preferences.getString((WIFI_SSID + num_str).c_str());
    return str;
}

String WiFiSetup::getPASSWDSetting(int num)
{
    String num_str(num);
    if(num == 0) num_str = "";
    String str = preferences.getString((WIFI_PASSWD + num_str).c_str());
    return str;
}

//  WiFiの設定モード
void WiFiSetup::setupMode()
{
    settingMode = true;

    WiFi.mode(WIFI_MODE_STA);
    WiFi.disconnect();
    delay(100);
    int n = WiFi.scanNetworks();
    delay(100);
    Serial.println("");
    M5.Lcd.println("");

    //  受信できているWiFiをリストにする
    ssidList = "";
    for (int i = 0; i < n; ++i)
    {
        ssidList += "<option value=\"";
        ssidList += WiFi.SSID(i);
        ssidList += "\">";
        ssidList += WiFi.SSID(i);
        ssidList += "</option>";
    }

    //  現在の設定も表示用に加工する
    ssidSettings = "";
    for(int i = 1; i <= 3; ++i)
    {
        ssidSettings += String(i) + ":";
        ssidSettings += getSSIDSetting(i);
        ssidSettings += "<br>";
    }

    delay(100);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(apSSID);
    WiFi.mode(WIFI_MODE_AP);

    startWebServer();
    Serial.print("Starting Access Point at \"");
    M5.Lcd.print("Starting Access Point at \"");
    Serial.print(apSSID);
    M5.Lcd.print(apSSID);
    Serial.println("\"");
    M5.Lcd.println("\"");

    delay(1000);
}

//  設定を復元
bool WiFiSetup::restoreConfig(int num)
{
    //  保存されている設定を読み出す
    wifi_ssid = getSSIDSetting(num);
    wifi_password = getPASSWDSetting(num);

    Serial.print(String(num).c_str());
    M5.Lcd.print(String(num).c_str());
    Serial.print(" WIFI-SSID: ");
    M5.Lcd.print(" WIFI-SSID: ");
    Serial.println(wifi_ssid);
    M5.Lcd.println(wifi_ssid);

    // Serial.print("WIFI-PASSWD: ");
    // M5.Lcd.print("WIFI-PASSWD: ");
    // Serial.println(wifi_password);
    // M5.Lcd.println(wifi_password);

    //  WiFi接続開始
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

    if (wifi_ssid.length() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//  接続確認
bool WiFiSetup::checkConnection()
{
    int count = 0;
    Serial.print("Waiting for Wi-Fi connection");
    M5.Lcd.print("Waiting for Wi-Fi connection");
    while (count < 30)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println();
            M5.Lcd.println();
            Serial.println("Connected!");
            M5.Lcd.println("Connected!");
            return (true);
        }
        delay(500);
        Serial.print(".");
        M5.Lcd.print(".");
        count++;
    }
    Serial.println("Timed out.");
    M5.Lcd.println("Timed out.");
    return false;
}

void WiFiSetup::startWebServer()
{
    if (settingMode)
    {
        //  設定モード
        Serial.print("Starting Web Server at ");
        M5.Lcd.print("Starting Web Server at ");
        Serial.println(WiFi.softAPIP());
        M5.Lcd.println(WiFi.softAPIP());

        //  設定用画面
        webServer.on("/settings", [this]() {
            String s = "<h1>Wi-Fi Settings</h1><p>Please enter your password by selecting the SSID.</p>";
            s += ssidSettings;
            s += "<form method=\"get\" action=\"setap\">";
            s += "<label>Num: </label><select name=\"num\">";
            s += "<option value=\"1\">1</option>";
            s += "<option value=\"2\">2</option>";
            s += "<option value=\"3\">3</option></select><br>";
            s += "<label>SSID: </label><select name=\"ssid\">";
            s += ssidList;
            s += "</select><br>Password: <input name=\"pass\" length=64 type=\"password\"><input type=\"submit\"></form>";
            webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
        });

        //  設定ボタンが押されたときに呼ばれる
        webServer.on("/setap", [this]() {
            String num = urlDecode(webServer.arg("num"));
            Serial.print("Num: ");
            M5.Lcd.print("Num: ");
            Serial.println(num);
            M5.Lcd.println(num);
            String ssid = urlDecode(webServer.arg("ssid"));
            Serial.print("SSID: ");
            M5.Lcd.print("SSID: ");
            Serial.println(ssid);
            M5.Lcd.println(ssid);
            String pass = urlDecode(webServer.arg("pass"));
            Serial.print("Password: ");
            M5.Lcd.print("Password: ");
            Serial.println(pass);
            M5.Lcd.println(pass);
            Serial.println("Writing SSID to EEPROM...");
            M5.Lcd.println("Writing SSID to EEPROM...");

            // Store wifi config
            Serial.println("Writing Password to nvr...");
            M5.Lcd.println("Writing Password to nvr...");

            setSSIDSetting(num.toInt(), ssid);
            setPASSWDSetting(num.toInt(), pass);

            Serial.println("Write nvr done!");
            M5.Lcd.println("Write nvr done!");
            String s = "<h1>Setup complete.</h1><p>device will be connected to \"";
            s += ssid;
            s += "\" after the restart.";
            webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
            delay(3000);
            ESP.restart();
        });

        //  設定開始
        webServer.onNotFound([this]() {
            String s = "<h1>AP mode</h1><p><a href=\"/settings\">Wi-Fi Settings</a></p>";
            webServer.send(200, "text/html", makePage("AP mode", s));
        });
    }
    else
    {
        Serial.print("Starting Web Server at ");
        M5.Lcd.print("Starting Web Server at ");
        Serial.println(WiFi.localIP());
        M5.Lcd.println(WiFi.localIP());
        webServer.on("/", [this]() {
            String s = "<h1>STA mode</h1><p><a href=\"/allreset\">All Reset Wi-Fi Settings</a></p>";
            webServer.send(200, "text/html", makePage("STA mode", s));
        });
        webServer.on("/allreset", [this]() {
            // reset the wifi config
            clearSettings();
            String s = "<h1>Wi-Fi settings was reset.</h1><p>Please reset device.</p>";
            webServer.send(200, "text/html", makePage("Reset Wi-Fi Settings", s));
            delay(3000);
            ESP.restart();
        });
    }
    webServer.begin();
}

String WiFiSetup::makePage(String title, String contents)
{
    String s = "<!DOCTYPE html><html><head>";
    s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
    s += "<title>";
    s += title;
    s += "</title></head><body>";
    s += contents;
    s += "</body></html>";
    return s;
}

String WiFiSetup::urlDecode(String input)
{
    String s = input;
    s.replace("%20", " ");
    s.replace("+", " ");
    s.replace("%21", "!");
    s.replace("%22", "\"");
    s.replace("%23", "#");
    s.replace("%24", "$");
    s.replace("%25", "%");
    s.replace("%26", "&");
    s.replace("%27", "\'");
    s.replace("%28", "(");
    s.replace("%29", ")");
    s.replace("%30", "*");
    s.replace("%31", "+");
    s.replace("%2C", ",");
    s.replace("%2E", ".");
    s.replace("%2F", "/");
    s.replace("%2C", ",");
    s.replace("%3A", ":");
    s.replace("%3A", ";");
    s.replace("%3C", "<");
    s.replace("%3D", "=");
    s.replace("%3E", ">");
    s.replace("%3F", "?");
    s.replace("%40", "@");
    s.replace("%5B", "[");
    s.replace("%5C", "\\");
    s.replace("%5D", "]");
    s.replace("%5E", "^");
    s.replace("%5F", "-");
    s.replace("%60", "`");
    return s;
}

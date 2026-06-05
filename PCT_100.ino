#include "key.h"
#include "relay.h"
#include "led.h"
#include "adc.h"
#include "ds18b20.h"
#include "oled.h"
#include "wsd.h"

#include <WiFi.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "zmjjkk"
#define WIFI_PASS "66666666"
#define LIGHT_LUX_THRESHOLD_DEFAULT 200
#define TEMP_THRESHOLD_DEFAULT 35.0f

Preferences prefs;
const char* PREFS_NAMESPACE_WIFI = "wifi_cfg";
const char* STORED_SSID = "stored_ssid";
const char* STORED_PASS = "stored_pass";

// MQTT配置常量
const char* PREFS_NAMESPACE_MQTT = "mqtt_cfg";
const char* STORED_MQTT_IP = "mqtt_ip";
const char* STORED_MQTT_PORT = "mqtt_port";
const char* STORED_MQTT_USER = "mqtt_user";
const char* STORED_MQTT_PASS = "mqtt_pass";
const char* STORED_MQTT_DEVICE_ID = "mqtt_device_id";
const char* STORED_TEMP_THRESHOLD = "temp_threshold";
const char* STORED_LIGHT_THRESHOLD = "light_threshold";

// MQTT默认配置
#define MQTT_DEFAULT_IP "47.98.170.180"
#define MQTT_DEFAULT_PORT 8081
#define MQTT_DEFAULT_USER "dzdx_emqx"
#define MQTT_DEFAULT_PASS "Jp4!sQ7$"
#define MQTT_DEFAULT_DEVICE_ID "PCT_100_043"

// MQTT参数
String mqtt_ip = MQTT_DEFAULT_IP;
int mqtt_port = MQTT_DEFAULT_PORT;
String mqtt_user = MQTT_DEFAULT_USER;
String mqtt_pass = MQTT_DEFAULT_PASS;
String device_id = MQTT_DEFAULT_DEVICE_ID;
float temp_threshold = TEMP_THRESHOLD_DEFAULT;
int light_threshold = LIGHT_LUX_THRESHOLD_DEFAULT;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long mqtt_reconnect_timer = 0;
unsigned long mqtt_publish_timer = 0;
bool mqtt_connected = false;

bool wifi_started = false;
unsigned long wifi_serial_timer = 0;
String inputSerial = "";
bool is_waiting_pass = false;
String target_ssid = "";
int scan_total = 0;

bool wifi_first_connect = false;
bool scan_ing = false;
bool need_print_scan = false;

// 存储WiFi IP
String wifi_ip_str = "No IP";

// 串口命令解析
String serial_cmd_buffer = "";
bool is_mqtt_config_mode = false;

uint8_t work_step = 0;
uint8_t system_mode = 1;
uint8_t last_key1 = 0;
uint8_t blink_en = 0;

unsigned long led_timer = 0;
unsigned long print_timer = 0;
unsigned long temp_timer = 0;
unsigned long oled_timer = 0;

float g_temp = 25.0f;

bool loadSavedWiFi(char* ssid, char* pass) {
  prefs.begin(PREFS_NAMESPACE_WIFI, false);
  String s = prefs.getString(STORED_SSID, "");
  String p = prefs.getString(STORED_PASS, "");
  prefs.end();
  if (s.length() < 2) return false;
  s.toCharArray(ssid, 32);
  p.toCharArray(pass, 64);
  return true;
}

void saveWiFiToFlash(String ssid, String pass) {
  prefs.begin(PREFS_NAMESPACE_WIFI, false);
  prefs.putString(STORED_SSID, ssid);
  prefs.putString(STORED_PASS, pass);
  prefs.end();
  Serial.println("[FLASH] WiFi已保存，下次上电自动连接");
}

void clearWiFiFlash() {
  prefs.begin(PREFS_NAMESPACE_WIFI, false);
  prefs.clear();
  prefs.end();
  WiFi.disconnect(true);
  wifi_ip_str = "No IP";
  Serial.println("[FLASH] 已清空所有WiFi记录！可连接新网络");
}

// MQTT配置保存
void saveMQTTConfig() {
  prefs.begin(PREFS_NAMESPACE_MQTT, false);
  prefs.putString(STORED_MQTT_IP, mqtt_ip);
  prefs.putInt(STORED_MQTT_PORT, mqtt_port);
  prefs.putString(STORED_MQTT_USER, mqtt_user);
  prefs.putString(STORED_MQTT_PASS, mqtt_pass);
  prefs.putString(STORED_MQTT_DEVICE_ID, device_id);
  prefs.putFloat(STORED_TEMP_THRESHOLD, temp_threshold);
  prefs.putInt(STORED_LIGHT_THRESHOLD, light_threshold);
  prefs.end();
  Serial.println("[FLASH] MQTT配置已保存");
}

// 加载MQTT配置
bool loadMQTTConfig() {
  prefs.begin(PREFS_NAMESPACE_MQTT, false);
  String ip = prefs.getString(STORED_MQTT_IP, "");
  int port = prefs.getInt(STORED_MQTT_PORT, 0);
  String user = prefs.getString(STORED_MQTT_USER, "");
  String pass = prefs.getString(STORED_MQTT_PASS, "");
  String did = prefs.getString(STORED_MQTT_DEVICE_ID, "");
  float temp = prefs.getFloat(STORED_TEMP_THRESHOLD, 0);
  int light = prefs.getInt(STORED_LIGHT_THRESHOLD, 0);
  prefs.end();
  
  if (ip.length() > 0 && port > 0 && did.length() > 0) {
    mqtt_ip = ip;
    mqtt_port = port;
    mqtt_user = user;
    mqtt_pass = pass;
    device_id = did;
    if (temp > 0) temp_threshold = temp;
    if (light > 0) light_threshold = light;
    Serial.println("[FLASH] MQTT配置加载成功");
    return true;
  }
  return false;
}

// 清空MQTT配置
void clearMQTTConfig() {
  prefs.begin(PREFS_NAMESPACE_MQTT, false);
  prefs.clear();
  prefs.end();
  mqtt_ip = MQTT_DEFAULT_IP;
  mqtt_port = MQTT_DEFAULT_PORT;
  mqtt_user = MQTT_DEFAULT_USER;
  mqtt_pass = MQTT_DEFAULT_PASS;
  device_id = MQTT_DEFAULT_DEVICE_ID;
  temp_threshold = TEMP_THRESHOLD_DEFAULT;
  light_threshold = LIGHT_LUX_THRESHOLD_DEFAULT;
  Serial.println("[FLASH] MQTT配置已清空，恢复默认值");
}

void handleSerialWiFi() {
  if (Serial.available() == 0) return;
  char c = Serial.read();
  if (c == '\n' || c == '\r') {
    inputSerial.trim();
    if (inputSerial == "clear") {
      clearWiFiFlash();
      inputSerial = "";
      return;
    }
    if (is_waiting_pass) {
      String pass = inputSerial;
      Serial.print("[WiFi] 正在连接：");
      Serial.println(target_ssid);
      WiFi.begin(target_ssid.c_str(), pass.c_str());
      unsigned long start = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
        delay(200);
        Serial.print(".");
      }
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] 连接成功！");
        saveWiFiToFlash(target_ssid, pass);
        wifi_ip_str = WiFi.localIP().toString();
        Serial.print("IP: ");
        Serial.println(wifi_ip_str);
      } else {
        Serial.println("\n[WiFi] 连接失败！");
        wifi_ip_str = "No IP";
      }
      is_waiting_pass = false;
      target_ssid = "";
    } else {
      int idx = inputSerial.toInt();
      if (idx >= 0 && idx < scan_total) {
        target_ssid = WiFi.SSID(idx);
        Serial.print("[输入] 请输入密码：");
        is_waiting_pass = true;
      } else {
        Serial.println("[输入] 无效序号！");
      }
    }
    inputSerial = "";
  } else {
    inputSerial += c;
  }
}

// MQTT命令处理
void handleMQTTCommand(char* topic, byte* payload, unsigned int length) {
  Serial.print("[MQTT] 收到命令: ");
  Serial.println(topic);
  
  DynamicJsonDocument doc(512);
  deserializeJson(doc, payload, length);
  
  String cmd = doc["cmd"].as<String>();
  
  if (cmd == "set_relay") {
    int relay = doc["relay"].as<int>();
    bool value = doc["value"].as<bool>();
    
    if (relay == 3) {
      peripherals_set(value, digitalRead(FAN_PIN));
      Serial.printf("[MQTT] 设置灯光 relay3 = %s\n", value ? "ON" : "OFF");
    } else if (relay == 4) {
      peripherals_set(digitalRead(LIGHT_PIN), value);
      Serial.printf("[MQTT] 设置风机 relay4 = %s\n", value ? "ON" : "OFF");
    }
  } else if (cmd == "set_mode") {
    String mode = doc["mode"].as<String>();
    if (mode == "auto") {
      system_mode = 1;
      Serial.println("[MQTT] 切换到自动模式");
    } else if (mode == "manual") {
      system_mode = 0;
      Serial.println("[MQTT] 切换到手动模式");
    }
  } else if (cmd == "get_status") {
    Serial.println("[MQTT] 收到查询状态命令");
    // 立即上报状态
    unsigned long now = millis();
    mqtt_publish_timer = now - 5000; // 触发立即发送
  } else if (cmd == "set_threshold") {
    if (doc.containsKey("temp")) {
      temp_threshold = doc["temp"].as<float>();
      Serial.printf("[MQTT] 设置温度阈值: %.1f\n", temp_threshold);
    }
    if (doc.containsKey("light")) {
      light_threshold = doc["light"].as<int>();
      Serial.printf("[MQTT] 设置光照阈值: %d\n", light_threshold);
    }
    saveMQTTConfig();
  } else if (cmd == "reboot") {
    Serial.println("[MQTT] 收到重启命令");
    ESP.restart();
  }
}

// MQTT回调函数
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  handleMQTTCommand(topic, payload, length);
}

// 发布设备状态
void publishStatus() {
  if (!mqtt_connected) return;
  
  int rawADC = adc_read(ADC_LIGHT_PIN);
  float lux = adc_to_lux(rawADC);
  float temp = ds18b20_read_temp();
  
  String status_topic = "chemctrl/" + device_id + "/status";
  
  DynamicJsonDocument doc(512);
  doc["temperature"] = temp;
  doc["light"] = (int)lux;
  doc["mode"] = system_mode == 1 ? "auto" : "manual";
  doc["key1_lock"] = (key1_read() == 1);
  doc["relay3"] = digitalRead(LIGHT_PIN) == HIGH;
  doc["relay4"] = digitalRead(FAN_PIN) == HIGH;
  doc["temp_threshold"] = temp_threshold;
  doc["light_threshold"] = light_threshold;
  
  char json_buffer[512];
  serializeJson(doc, json_buffer, sizeof(json_buffer));
  
  if (client.publish(status_topic.c_str(), json_buffer)) {
    Serial.println("[MQTT] 状态上报成功");
  } else {
    Serial.println("[MQTT] 状态上报失败");
  }
}

// MQTT连接
void connectMQTT() {
  if (WiFi.status() != WL_CONNECTED) return;
  
  client.setServer(mqtt_ip.c_str(), mqtt_port);
  client.setCallback(mqttCallback);
  
  String client_id = "PCT100_" + String(random(0xffff), HEX);
  
  Serial.printf("[MQTT] 连接到 %s:%d...\n", mqtt_ip.c_str(), mqtt_port);
  
  if (client.connect(client_id.c_str(), mqtt_user.c_str(), mqtt_pass.c_str())) {
    mqtt_connected = true;
    Serial.println("[MQTT] 连接成功");
    
    // 订阅命令主题
    String cmd_topic = "chemctrl/" + device_id + "/command";
    if (client.subscribe(cmd_topic.c_str())) {
      Serial.printf("[MQTT] 已订阅: %s\n", cmd_topic.c_str());
    }
    
    // 立即上报一次状态
    publishStatus();
  } else {
    mqtt_connected = false;
    Serial.printf("[MQTT] 连接失败, rc=%d\n", client.state());
  }
}

// 处理MQTT串口命令
void handleSerialMQTT() {
  if (Serial.available() == 0) return;
  char c = Serial.read();
  if (c == '\n' || c == '\r') {
    serial_cmd_buffer.trim();
    
    if (serial_cmd_buffer == "mqtt help") {
      Serial.println("\n=========== MQTT配置命令 ===========");
      Serial.println("mqtt set ip <IP地址>        设置MQTT服务器IP");
      Serial.println("mqtt set port <端口>        设置MQTT服务器端口");
      Serial.println("mqtt set user <用户名>      设置MQTT用户名");
      Serial.println("mqtt set pass <密码>        设置MQTT密码");
      Serial.println("mqtt set device_id <ID>     设置设备ID");
      Serial.println("mqtt set temp <温度>        设置温度阈值");
      Serial.println("mqtt set light <数值>       设置光照阈值");
      Serial.println("mqtt show                   显示当前配置");
      Serial.println("mqtt save                   保存配置到Flash");
      Serial.println("mqtt load                   从Flash加载配置");
      Serial.println("mqtt clear                  清空配置恢复默认");
      Serial.println("mqtt connect                连接MQTT服务器");
      Serial.println("====================================");
    }
    else if (serial_cmd_buffer.startsWith("mqtt set ip ")) {
      mqtt_ip = serial_cmd_buffer.substring(11);
      Serial.printf("[MQTT] IP设置为: %s\n", mqtt_ip.c_str());
    }
    else if (serial_cmd_buffer.startsWith("mqtt set port ")) {
      mqtt_port = serial_cmd_buffer.substring(13).toInt();
      Serial.printf("[MQTT] 端口设置为: %d\n", mqtt_port);
    }
    else if (serial_cmd_buffer.startsWith("mqtt set user ")) {
      mqtt_user = serial_cmd_buffer.substring(14);
      Serial.printf("[MQTT] 用户设置为: %s\n", mqtt_user.c_str());
    }
    else if (serial_cmd_buffer.startsWith("mqtt set pass ")) {
      mqtt_pass = serial_cmd_buffer.substring(14);
      Serial.println("[MQTT] 密码已设置");
    }
    else if (serial_cmd_buffer.startsWith("mqtt set device_id ")) {
      device_id = serial_cmd_buffer.substring(20);
      Serial.printf("[MQTT] 设备ID设置为: %s\n", device_id.c_str());
    }
    else if (serial_cmd_buffer.startsWith("mqtt set temp ")) {
      temp_threshold = serial_cmd_buffer.substring(14).toFloat();
      Serial.printf("[MQTT] 温度阈值设置为: %.1f\n", temp_threshold);
    }
    else if (serial_cmd_buffer.startsWith("mqtt set light ")) {
      light_threshold = serial_cmd_buffer.substring(15).toInt();
      Serial.printf("[MQTT] 光照阈值设置为: %d\n", light_threshold);
    }
    else if (serial_cmd_buffer == "mqtt show") {
      Serial.println("\n=========== 当前MQTT配置 ===========");
      Serial.printf("MQTT IP:      %s\n", mqtt_ip.c_str());
      Serial.printf("MQTT Port:    %d\n", mqtt_port);
      Serial.printf("MQTT User:    %s\n", mqtt_user.c_str());
      Serial.printf("MQTT Pass:    %s\n", mqtt_pass.c_str());
      Serial.printf("Device ID:    %s\n", device_id.c_str());
      Serial.printf("温度阈值:     %.1f\n", temp_threshold);
      Serial.printf("光照阈值:     %d\n", light_threshold);
      Serial.println("====================================");
    }
    else if (serial_cmd_buffer == "mqtt save") {
      saveMQTTConfig();
    }
    else if (serial_cmd_buffer == "mqtt load") {
      loadMQTTConfig();
      Serial.println("[MQTT] 配置已加载");
    }
    else if (serial_cmd_buffer == "mqtt clear") {
      clearMQTTConfig();
    }
    else if (serial_cmd_buffer == "mqtt connect") {
      connectMQTT();
    }
    else if (serial_cmd_buffer.length() > 0) {
      Serial.println("[输入] 未知命令，输入 'mqtt help' 查看帮助");
    }
    
    serial_cmd_buffer = "";
  } else {
    serial_cmd_buffer += c;
  }
}

void scanAndShowWiFi() {
  if(scan_ing) return;
  WiFi.scanDelete();
  WiFi.scanNetworks(true,false);
  scan_ing = true;
  need_print_scan = true;
  Serial.println("\nWiFi扫描中...");
}

void autoConnectSavedWiFi() {
  char ssid[32], pass[64];
  if (!loadSavedWiFi(ssid, pass)) {
    Serial.println("[FLASH] 无保存WiFi，进入手动配网");
    scanAndShowWiFi();
    wifi_ip_str = "No IP";
    return;
  }
  Serial.print("[FLASH] 自动连接：");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
    delay(200);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] 自动连接成功！");
    wifi_ip_str = WiFi.localIP().toString();
    Serial.print("IP: ");
    Serial.println(wifi_ip_str);
  } else {
    Serial.println("\n[WiFi] 自动连接失败，进入手动配网");
    scanAndShowWiFi();
    wifi_ip_str = "No IP";
  }
}

void setup() {
  Serial.begin(115200);
  delay(50);

  wsd_init();
  wsd_boot_blue_blink();

  led_init();
  key_init();
  peripherals_init();
  adc_init();
  ds18b20_init();
  oled_init();

  WiFi.mode(WIFI_STA);

  // 加载MQTT配置
  loadMQTTConfig();

  Serial.println("==================================");
  Serial.println("System Ready | Default: AUTO 模式");
  Serial.println("==================================");
  Serial.println("【指令】输入 clear 可清空已保存WiFi");
  Serial.println("【MQTT】输入 mqtt help 查看配置命令");
}

void loop() {
  // 异步WiFi扫描
  if(scan_ing) {
    int res = WiFi.scanComplete();
    if(res >=0) {
      scan_total = res;
      scan_ing = false;
      if(need_print_scan) {
        Serial.println("\n================ WiFi扫描 ================");
        if (scan_total == 0) {
          Serial.println("未扫描到WiFi热点");
        }else{
          for (int i = 0; i < scan_total; i++) {
            Serial.printf("[%d] %s (RSSI:%d)\n", i, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
          }
        }
        Serial.println("==========================================");
        Serial.print("请输入要连接的WiFi序号：");
        need_print_scan = false;
      }
    }
  }

  // 首次上电连WiFi
  if (wifi_first_connect) {
    wifi_first_connect = false;
    autoConnectSavedWiFi();
  }

  // WiFi 掉线重连 & IP 刷新
  static unsigned long reconnect_tmr=0;
  if(millis()-reconnect_tmr>3000){
    reconnect_tmr=millis();
    if(WiFi.status()!=WL_CONNECTED){
      char ssid[32],pass[64];
      if(loadSavedWiFi(ssid,pass)){
        WiFi.begin(ssid,pass);
        wifi_ip_str = "Reconnect";
      }else{
        wifi_ip_str = "No IP";
      }
    }else{
      wifi_ip_str = WiFi.localIP().toString();
    }
  }

  // RGB状态灯
  if (WiFi.status() == WL_CONNECTED) {
    if (mqtt_connected) {
      wsd_green();
    } else {
      // WiFi已连接但MQTT未连接，蓝灯闪烁
      static unsigned long wsd_blink_tmr = 0;
      if (millis() - wsd_blink_tmr > 500) {
        wsd_blink_tmr = millis();
        static bool wsd_state = false;
        wsd_state = !wsd_state;
        if (wsd_state) wsd_red();
        else wsd_green();
      }
    }
  } else {
    wsd_red();
  }

  handleSerialWiFi();
  handleSerialMQTT();

  // MQTT重连
  if (WiFi.status() == WL_CONNECTED && !mqtt_connected) {
    if (millis() - mqtt_reconnect_timer > 5000) {
      mqtt_reconnect_timer = millis();
      connectMQTT();
    }
  }

  // MQTT状态上报（每5秒）
  if (mqtt_connected) {
    client.loop();
    if (millis() - mqtt_publish_timer > 5000) {
      mqtt_publish_timer = millis();
      publishStatus();
    }
  }

  uint8_t curr_key1 = key1_read();
  uint8_t key_act = key2_scan();

  if (curr_key1 == 0) {
    if (last_key1 == 1) {
      work_step = 0;
      peripherals_set(0, 0);
      blink_en = 0;
      system_mode = 1;
      Serial.println("[INFO] 总闸关闭 → 系统全停");
    }
    last_key1 = 0;

    if (millis() - oled_timer >= 100) {
      oled_timer = millis();
      int rawADC = adc_read(ADC_LIGHT_PIN);
      float lux = adc_to_lux(rawADC);
      oled_show(system_mode, last_key1, (int)lux, g_temp, 0, 0);
    }
    return;
  }

  if (last_key1 == 0) {
    Serial.println("[INFO] 总闸开启 → 系统启动");
    last_key1 = 1;
  }

  if (blink_en) {
    unsigned long now = millis();
    if (now - led_timer >= 500) {
      LED_TOGGLE();
      led_timer = now;
    }
  } else {
    LED_OFF();
  }

  if (key_act == 2) {
    if (system_mode == 0) {
      system_mode = 1;
      work_step = 0;
      peripherals_set(0, 0);
      blink_en = 0;
      Serial.println("[MODE] → 自动模式");
    } else {
      system_mode = 0;
      work_step = 0;
      peripherals_set(0, 0);
      blink_en = 0;
      Serial.println("[MODE] → 手动模式");
    }
  }

  if (system_mode == 0) {
    if (key_act == 1) {
      work_step = (work_step + 1) % 4;
      blink_en = (work_step != 0);

      Serial.print("手动状态：");
      switch (work_step) {
        case 0: peripherals_set(0,0); Serial.println("00 全部关闭"); break;
        case 1: peripherals_set(0,1); Serial.println("01 风扇开启"); break;
        case 2: peripherals_set(1,0); Serial.println("10 灯光开启"); break;
        case 3: peripherals_set(1,1); Serial.println("11 全部开启"); break;
      }
    }

    int rawADC = adc_read(ADC_LIGHT_PIN);
    float lux = adc_to_lux(rawADC);
    uint8_t light_state = (work_step == 2 || work_step == 3) ? 1 : 0;
    uint8_t fan_state   = (work_step == 1 || work_step == 3) ? 1 : 0;

    if (millis() - oled_timer >= 100) {
      oled_timer = millis();
      oled_show(system_mode, last_key1, (int)lux, g_temp, light_state, fan_state);
    }
  } else {
    blink_en = 1;
    unsigned long now = millis();
    
    int rawADC = adc_read(ADC_LIGHT_PIN);
    float lux = adc_to_lux(rawADC);
    uint8_t light_state = (lux <= light_threshold) ? 1 : 0;

    if (now - temp_timer >= 1000) {
      g_temp = ds18b20_read_temp();
      temp_timer = now;
    }
    uint8_t fan_state = (g_temp >= temp_threshold) ? 1 : 0;

    peripherals_set(light_state, fan_state);

    if (now - print_timer >= 1000) {
      Serial.print("光照lux："); Serial.print((int)lux);
      Serial.print(" | 温度："); Serial.print(g_temp); Serial.println(" ℃");
      Serial.print("IP: "); Serial.println(wifi_ip_str);
      Serial.print("MQTT: "); Serial.println(mqtt_connected ? "Connected" : "Disconnected");
      print_timer = now;
    }

    if (millis() - oled_timer >= 100) {
      oled_timer = millis();
      oled_show(system_mode, last_key1, (int)lux, g_temp, light_state, fan_state);
    }
  }
}
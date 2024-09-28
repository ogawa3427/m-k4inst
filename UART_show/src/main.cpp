#include <Arduino.h>
#include <M5Unified.h>
#include <M5UnitSynth.h>
#include <note.h>
#include <tone.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "nvs_flash.h"
#include "nvs.h"

// https://github.com/espressif/esp-idf/blob/v5.3/examples/storage/nvs_rw_value/main/nvs_value_example_main.c

void readNVS(const char *key, String &value)
{
  nvs_handle_t my_handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    USBSerial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
  else
  {
    USBSerial.println("NVS handle opened successfully");

    size_t required_size = 0; // valueのサイズを取得するための変数
    err = nvs_get_str(my_handle, key, NULL, &required_size);
    if (err == ESP_OK)
    {
      char *buffer = new char[required_size];
      err = nvs_get_str(my_handle, key, buffer, &required_size);
      if (err == ESP_OK)
      {
        value = String(buffer);
        USBSerial.printf("%s = %s\n", key, value.c_str());
      }
      else
      {
        USBSerial.printf("Error (%s) reading %s!\n", esp_err_to_name(err), key);
      }
      delete[] buffer;
    }
    else if (err == ESP_ERR_NVS_NOT_FOUND)
    {
      USBSerial.printf("The value for %s is not initialized yet!\n", key);
    }
    else
    {
      USBSerial.printf("Error (%s) reading %s!\n", esp_err_to_name(err), key);
    }

    nvs_close(my_handle);
  }
}

void storeReceivedData(const String &receivedData, String &key)
{
  key = receivedData;
  USBSerial.printf("Stored data for key: %s\n", key.c_str());
  USBSerial.printf("Value: %s\n", receivedData.c_str());
}
void writeNVS(const char *key, const String &value)
{
  nvs_handle_t my_handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err == ESP_OK)
  {
    err = nvs_set_str(my_handle, key, value.c_str());
    if (err != ESP_OK)
    {
      USBSerial.printf("Failed to update value for key: %s, error: %s\n", key, esp_err_to_name(err));
    }
    else
    {
      USBSerial.printf("Value updated successfully for key: %s\n", key);
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK)
    {
      USBSerial.printf("Failed to commit updates for key: %s, error: %s\n", key, esp_err_to_name(err));
    }
    else
    {
      USBSerial.printf("Updates committed successfully for key: %s\n", key);
    }

    nvs_close(my_handle);
  }
  else
  {
    USBSerial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
}

struct CtrKeyCfg
{
  String enter;
  String up;
  String down;
  String esc;

  String pull_enter;
  String pull_up;
  String pull_down;
  String pull_esc;
};

struct BowingKeyCfg
{
  String upBow;
  String downBow;
  String rightClick;
  String leftClick;
};

struct ToneKeyCfg
{
  String g4;
  String g4_pull;
  String gs4;
  String gs4_pull;
  String a4;
  String a4_pull;
  String as4;
  String as4_pull;
  String b4;
  String b4_pull;
  String c5;
  String c5_pull;
};

CtrKeyCfg ctrKeyCfg;
uint8_t ctrKeyPhase = 0;

BowingKeyCfg bowingKeyCfg;
uint8_t bowingKeyPhase = 0;

ToneKeyCfg toneKeyCfg;
uint8_t toneKeyPhase = 0;

CtrKeyCfg readCtrKeyCfg()
{
  CtrKeyCfg cfg;
  String temp;
  readNVS("ctrKeyCfg_enter", temp);
  cfg.enter = String(temp.c_str());
  for (int i = 0; i < cfg.enter.length(); i++)
  {
    USBSerial.print(cfg.enter[i], HEX);
    USBSerial.print(" ");
    USBSerial.println();
  }
  readNVS("ctrKeyCfg_up", temp);
  cfg.up = String(temp.c_str());
  readNVS("ctrKeyCfg_down", temp);
  cfg.down = String(temp.c_str());
  readNVS("ctrKeyCfg_esc", temp);
  cfg.esc = String(temp.c_str());

  readNVS("p_enter", temp);
  cfg.pull_enter = String(temp.c_str());
  readNVS("p_up", temp);
  cfg.pull_up = String(temp.c_str());
  readNVS("p_down", temp);
  cfg.pull_down = String(temp.c_str());
  readNVS("p_esc", temp);
  cfg.pull_esc = String(temp.c_str());
  for (int i = 0; i < cfg.pull_esc.length(); i++)
  {
    USBSerial.print(cfg.pull_esc[i], HEX);
    USBSerial.print(" ");
  }
  USBSerial.println();
  return cfg;
}

BowingKeyCfg readBowingKeyCfg()
{
  BowingKeyCfg cfg;
  String temp;
  readNVS("bowingKeyCfg_upBow", temp);
  cfg.upBow = String(temp.c_str());
  readNVS("bowingKeyCfg_downBow", temp);
  cfg.downBow = String(temp.c_str());
  readNVS("bowingKeyCfg_rightClick", temp);
  cfg.rightClick = String(temp.c_str());
  readNVS("bowingKeyCfg_leftClick", temp);
  cfg.leftClick = String(temp.c_str());
  return cfg;
}

ToneKeyCfg readToneKeyCfg()
{
  ToneKeyCfg cfg;
  String temp;
  readNVS("toneKeyCfg_g4", temp);
  cfg.g4 = String(temp.c_str());
  readNVS("toneKeyCfg_g4_pull", temp);
  cfg.g4_pull = String(temp.c_str());
  readNVS("toneKeyCfg_gs4", temp);
  cfg.gs4 = String(temp.c_str());
  readNVS("toneKeyCfg_gs4_pull", temp);
  cfg.gs4_pull = String(temp.c_str());
  readNVS("toneKeyCfg_a4", temp);
  cfg.a4 = String(temp.c_str());
  readNVS("toneKeyCfg_a4_pull", temp);
  cfg.a4_pull = String(temp.c_str());
  readNVS("toneKeyCfg_as4", temp);
  cfg.as4 = String(temp.c_str());
  readNVS("toneKeyCfg_as4_pull", temp);
  cfg.as4_pull = String(temp.c_str());
  readNVS("toneKeyCfg_b4", temp);
  cfg.b4 = String(temp.c_str());
  readNVS("toneKeyCfg_b4_pull", temp);
  cfg.b4_pull = String(temp.c_str());
  readNVS("toneKeyCfg_c5", temp);
  cfg.c5 = String(temp.c_str());
}

enum OuterStates
{
  INIT,
  ASK_CTR_KEY_CFG,
  READ_CTR_KEY_CFG,
  MAIN,
  BOW,
  TONE
};

OuterStates outerState = INIT;

// hahaha
static int INSTRUMENT_ = 41;

enum states
{
  NTH0,
  NTH1,
  ON,
  OFF,
  CHG
};

const char *stateToString(states state)
{
  switch (state)
  {
  case NTH0:
    return "NTH0";
  case NTH1:
    return "NTH1";
  case ON:
    return "ON  ";
  case OFF:
    return "OFF ";
  case CHG:
    return "CHG ";
  default:
    return "Unknown State";
  }
}

states state = NTH0;
states pastState = NTH0;
bool goSign = false;

uint32_t timeKeep = 0;
uint32_t pastTime = 0;
int SUS_BORDER = 70;

bool isNotPassed = false;
bool pastIsNotPassed = false;

int timeStamp_LastReceive = 0;

// struct NowState
// {
//   int note;
//   bool valBow;
// };

// struct PastState
// {
//   int note;
//   bool valBow;
// };

int instrument = 41;

M5UnitSynth synth;

int VOLUME = 127;

uint32_t timer = 0;

int currentNote = 0;
int pastNote = 0;
int pastPastNote = 0;
int prevLoopNote = 0;

bool hold = false;
bool pastHold = false;
// 0 音を出さない
// 1 音を出す

char pastByte15 = 0x00;
char pastByte16 = 0x00;

bool pastBow = false;

void updateNote()
{
  if (currentNote != pastNote)
  {
    pastPastNote = pastNote;
    pastNote = currentNote;
  }
}

void setup()
{
  // UARTを初期化 (TX:GPIO1, RX:GPIO2を使用する例)
  Serial2.begin(115200, SERIAL_8N1, 5, 6);
  USBSerial.begin(115200);

  synth.begin(&Serial1, UNIT_SYNTH_BAUD, 1, 2);
  synth.setInstrument(0, 0, INSTRUMENT_);
  synth.setNoteOn(0, NOTE_C6, VOLUME);
  delay(1000);
  synth.setNoteOff(0, NOTE_C6, 0);

  // NVSの初期化
  // esp_err_t err = nvs_flash_init();
  // if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  // {
  //   ESP_ERROR_CHECK(nvs_flash_erase());
  //   err = nvs_flash_init();
  // }
  // ESP_ERROR_CHECK(err);

  // int32_t restart_counter = 0;
  // readNVS("restart_counter", restart_counter);
  // writeNVS("restart_counter", restart_counter);

  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.startWrite();
  M5.Display.setCursor(0, 0);
  M5.Display.print(millis());
  M5.Display.endWrite();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.println("Hello!");
  // M5.Lcd.printf("Restart= %" PRIu32 "\n", restart_counter);

  // outerState = ASK_CTR_KEY_CFG;
  // CtrKeyCfg ctrKeyCfg = readCtrKeyCfg();
  // cfg_ = askCtrKeyCfg();
  outerState = ASK_CTR_KEY_CFG;
}

// CtrKeyCfg ctrKeyCfg;

// 変数の初期化部分に過去の値を保存するための変数を追加
uint32_t pastTimeKeep = 0;
int pastCurrentNote = 0;
bool pastGoSign = false;
int num = 0;


void printAsHEX(const String &receivedData)
{
  for (int i = 0; i < receivedData.length(); i++)
  {
    USBSerial.print(receivedData[i], HEX); // 受信したデータを16進数で出力
    USBSerial.print(" ");                  // 数字の間にスペースを入れる
  }
  USBSerial.println(); // 改行を出力
}

bool compareReceivedData(const String &receivedData, const String &cfgData)
{
  USBSerial.println("Comparing received data with cfg data:");
  USBSerial.print("Received: ");
  printAsHEX(receivedData);
  USBSerial.print("Config  : ");
  printAsHEX(cfgData);

  if (receivedData.length() != cfgData.length())
  {
    USBSerial.println("Length mismatch");
    return false;
  }

  for (int i = 0; i < receivedData.length(); i++)
  {
    if (receivedData[i] != cfgData[i])
    {
      USBSerial.printf("Mismatch at index %d: received %02X, expected %02X\n", i, receivedData[i], cfgData[i]);
      return false;
    }
  }
  return true;
}

void loop()
{
    if (timeKeep % 1000 == 0)
    {
      USBSerial.println(timeKeep);
    }
    
  OuterStates outerState = MAIN;
  if (outerState == ASK_CTR_KEY_CFG)
  {
    timeKeep = millis();
    String receivedData = "";
    // UARTでデータを受信
    if (Serial2.available())
    {
      receivedData = Serial2.readStringUntil('\n');
    }

    static int lastPhase = -1; // 前回のフェーズを記録する変数

    if (ctrKeyPhase != lastPhase) // フェーズが変わったときだけ画面を更新
    {
      M5.Lcd.startWrite();
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.fillScreen(BLACK);
      switch (ctrKeyPhase)
      {
      case 0:
        M5.Lcd.println("Please press and hold [Enter]\nTo skip: set [Esc]");
        break;
      case 1:
        M5.Lcd.println("Release [Enter]");
        break;
      case 2:
        M5.Lcd.println("Please press and hold [Up]");
        break;
      case 3:
        M5.Lcd.println("Release [Up]");
        break;
      case 4:
        M5.Lcd.println("Please press and hold [Down]");
        break;
      case 5:
        M5.Lcd.println("Release [Down]");
        break;
      case 6:
        M5.Lcd.println("Please press and hold [Esc]");
        break;
      case 7:
        M5.Lcd.println("Release [Esc]");
        break;
      case 8:
        M5.Lcd.println("Check cfg\nRetry: BtnB\nOK: BtnA");
        break;
      case 9:
        String msg = receivedData;
        msg.trim();
        M5.Lcd.println(msg);
      }
      M5.Lcd.endWrite();
      lastPhase = ctrKeyPhase; // フェーズを更新
    }

    if (receivedData != "")
    {
      receivedData.trim(); // 受信データの前後の空白をトリム
      USBSerial.print("Phase: ");
      USBSerial.println(ctrKeyPhase);
      switch (ctrKeyPhase)
      {
      case 0:
        for (int i = 0; i < ctrKeyCfg.enter.length(); i++)
        {
          USBSerial.print(ctrKeyCfg.enter[i], HEX);
          USBSerial.print(" ");
        }
        USBSerial.println();
        for (int i = 0; i < ctrKeyCfg.esc.length(); i++)
        {
          USBSerial.print(ctrKeyCfg.esc[i], HEX);
          USBSerial.print(" ");
        }
        USBSerial.println();
        USBSerial.println("Phase 0 compairing...");
        if (compareReceivedData(receivedData, ctrKeyCfg.esc))
        {
          ctrKeyPhase = 8;
          break;
        }
        else
        {
          storeReceivedData(receivedData, ctrKeyCfg.enter);
          ctrKeyPhase++;
          break;
        }
      case 1:
        storeReceivedData(receivedData, ctrKeyCfg.pull_enter);
        ctrKeyPhase++;
        break;
      case 2:
        storeReceivedData(receivedData, ctrKeyCfg.up);
        ctrKeyPhase++;
        break;
      case 3:
        storeReceivedData(receivedData, ctrKeyCfg.pull_up);
        ctrKeyPhase++;
        break;
      case 4:
        storeReceivedData(receivedData, ctrKeyCfg.down);
        ctrKeyPhase++;
        break;
      case 5:
        storeReceivedData(receivedData, ctrKeyCfg.pull_down);
        ctrKeyPhase++;
        break;
      case 6:
        storeReceivedData(receivedData, ctrKeyCfg.esc);
        ctrKeyPhase++;
        break;
      case 7:
        storeReceivedData(receivedData, ctrKeyCfg.pull_esc);
        USBSerial.println("Check cfg");
        ctrKeyPhase++;
        break;
      case 8:
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.fillScreen(BLACK);
        // for (int i = 0; i < receivedData.length(); i++)
        // {
        //   M5.Lcd.print(receivedData[i], HEX);
        //   M5.Lcd.print(" ");
        // }
        // M5.Lcd.println();
        // if (compareReceivedData(receivedData, ctrKeyCfg.enter))
        // {
        // // if (false)
        // //{
        //   USBSerial.println("Enter-----------------");
        // }
        // else if (compareReceivedData(receivedData, ctrKeyCfg.pull_enter))
        // {
        //   USBSerial.println("Pull Enter------------");
        // }
        // else if (compareReceivedData(receivedData, ctrKeyCfg.up))
        // {
        //   USBSerial.println("Up--------------------");
        // }
        // else if (compareReceivedData(receivedData, ctrKeyCfg.pull_up))
        // {
        //   USBSerial.println("Pull Up---------------");
        // }
        // else if (compareReceivedData(receivedData, ctrKeyCfg.down))
        // {
        //   USBSerial.println("Down------------------");
        // }
        // else if (compareReceivedData(receivedData, ctrKeyCfg.pull_down))
        // {
        //   USBSerial.println("Pull Down-------------");
        // }
        // else if (compareReceivedData(receivedData, ctrKeyCfg.esc))
        // {
        //   USBSerial.println("Esc-------------------");
        // }
        // else if (compareReceivedData(receivedData, ctrKeyCfg.pull_esc))
        // {
        //   USBSerial.println("Pull Esc--------------");
        // }
        // else
        // {
        //   USBSerial.println("Unknown");
        //   USBSerial.println(receivedData);
        //   USBSerial.println(ctrKeyCfg.enter);
        //   USBSerial.println(ctrKeyCfg.pull_enter);
        //   USBSerial.println(ctrKeyCfg.up);
        //   USBSerial.println(ctrKeyCfg.pull_up);
        //   USBSerial.println(ctrKeyCfg.down);
        //   USBSerial.println(ctrKeyCfg.pull_down);
        //   USBSerial.println(ctrKeyCfg.esc);
        //   USBSerial.println(ctrKeyCfg.pull_esc);
        // }
        USBSerial.print("Received  : ");
        printAsHEX(receivedData);
        // USBSerial.print("Enter     : ");
        // printAsHEX(ctrKeyCfg.enter);
        // USBSerial.print("Pull Enter: ");
        // printAsHEX(ctrKeyCfg.pull_enter);
        break;
      }
    }
    if (ctrKeyPhase == 8)
    {
      M5.update();
      if (M5.BtnA.wasPressed())
      {
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.println("Writing to NVS...");
        writeNVS("ctrKeyCfg_enter", ctrKeyCfg.enter);
        writeNVS("ctrKeyCfg_up", ctrKeyCfg.up);
        writeNVS("ctrKeyCfg_down", ctrKeyCfg.down);
        writeNVS("ctrKeyCfg_esc", ctrKeyCfg.esc);

        writeNVS("p_enter", ctrKeyCfg.pull_enter);
        writeNVS("p_up", ctrKeyCfg.pull_up);
        writeNVS("p_down", ctrKeyCfg.pull_down);
        writeNVS("p_esc", ctrKeyCfg.pull_esc);
        ctrKeyPhase = 0;
        outerState = BOW;
        M5.Lcd.println("Done!");
      }
      else if (M5.BtnB.wasPressed())
      {
        ctrKeyPhase = 0;
      }
    }
  }
  else if (outerState == MAIN)
  {
    timeKeep = millis();
    // if (timeKeep % 1000 == 0)
    // {
    //   USBSerial.println(timeKeep);
    // }
    // UARTでデータを受信
    if (Serial2.available())
    {
      String receivedData = Serial2.readStringUntil('\n');
      for (int i = 0; i < receivedData.length(); i++)
      {
        USBSerial.print(receivedData[i], HEX); // 受信したデータを16進数で出力
        USBSerial.print(" ");                  // 数字の間にスペースを入れる
      }
      USBSerial.println(); // 改行を出力
      // 受信したデータの15バイト目と19バイト目を表示
      // USBSerial.println("Received Data:");
      if (true) //(receivedData.length() >= 19)
      {
        char byte5 = receivedData[4];
        char byte15 = receivedData[13]; // 15バイト目 (インデックスは0から始まるので14)
        char byte16 = receivedData[14];
        char byte17 = receivedData[15];
        char byte18 = receivedData[16];
        char byte19 = receivedData[17]; // 19バイト目 (インデックスは0から始まるので18)
        // USBSerial.print(byte5, HEX);    // 15バイト目を16進数で出力
        // USBSerial.print(" ");           // 数字の間にスペースを入れる
        // USBSerial.print(byte15, HEX);   // 19バイト目を16進数で出力
        // USBSerial.print(" ");           // 数字の間にスペースを入れる
        // USBSerial.print(byte16, HEX);   // 5バイト目を16進数で出力
        // USBSerial.print(" ");           // 数字の間にスペースを入れる
        // USBSerial.print(byte17, HEX);   // 5バイト目を16進数で出力
        // USBSerial.println();            // 改行を出力
        // 前回の受信から0.05秒以上経過しているか判定
        int delta = timeKeep - timeStamp_LastReceive;
        timeStamp_LastReceive = timeKeep;
        if (delta > 25)
        {
          // USBSerial.print("---------------------------------");
          // USBSerial.println(delta);
        }
        for (int i = 0; i < receivedData.length(); i++)
        {
          // USBSerial.print(receivedData[i], HEX); // 受信したデータを16進数で出力
          // USBSerial.print(" ");                  // 数字の間にスペースを入れる
        }
        // USBSerial.println(); // 改行を出力
        hold = true;
        // if (byte17 == 0xFF && byte5 == 0x03)
        // {
        //   // USBSerial.print("UpScr");
        //   pastTime = timeKeep;
        // }
        // else if (byte17 == 0x01 && byte5 == 0x03)
        // {
        //   // USBSerial.print("DownScr");
        //   pastTime = timeKeep;
        // }
        // ここに新しい条件を追加
        if (byte5 == 0x00 && byte15 == 0x00 && byte16 == 0xFF && byte17 == 0x00)
        // else if (byte5 == 0x00 && byte15 == 0x00 && byte16 == 0xFF && byte17 == 0x00)
        {
          // USBSerial.print("UpScr");
          pastTime = timeKeep;
        }
        else if (byte5 == 0x00 && byte15 == 0x00 && byte16 == 0x01 && byte17 == 0x00)
        {
          // USBSerial.print("DownScr");
          pastTime = timeKeep;
        }
        else if (byte5 == 0x06)
        {
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.fillScreen(BLACK);
          if (byte15 == 0x00 && byte16 == 0x00 && byte17 == 0x00 && byte18 == 0x00)
          {
            // USBSerial.print("OFF");
            hold = false;
            updateNote();
            currentNote = 0;
            M5.Lcd.println("OFF");
          }
          else if (byte15 != 0x00 && byte16 == 0x00 && byte17 == 0x00 && byte18 == 0x00)
          {
            // USBSerial.print(byte15, HEX);
            updateNote();
            currentNote = tonedict[byte15];
            pastByte15 = byte15;
            pastByte16 = byte16;
            M5.Lcd.println("ON");
          }
          else if (byte15 == 0x00 && byte16 != 0x00 && byte17 == 0x00 && byte18 == 0x00)
          {
            // USBSerial.print(byte16, HEX);
            updateNote();
            currentNote = tonedict[byte16];
            pastByte15 = byte15;
            pastByte16 = byte16;
            M5.Lcd.println("ON2");
          }
          else if (byte15 != 0x00 && byte16 != 0x00 && byte17 == 0x00 && byte18 == 0x00)
          {
            if (pastByte15 == 0x00 && pastByte16 != 0x00)
            {
              // USBSerial.print(byte15, HEX);
              pastNote = currentNote;
              currentNote = tonedict[byte15];
            }
            else
            {
              // USBSerial.print(byte16, HEX);
              pastNote = currentNote;
              currentNote = tonedict[byte16];
            }
            M5.Lcd.println("ON3");
          }
        }
        else if (byte19 == 0x00 && byte15 != 0x00 && byte5 == 0x06 && byte17 == 0x00 && byte18 == 0x00)
        {
          // USBSerial.print(byte15, HEX);
          currentNote = tonedict[byte15];
          M5.Lcd.println("ON4");
        }

        if (receivedData[1] == 0x07 && receivedData[16] == 0x01)
        {
          // USBSerial.print("UpBow");
          pastTime = timeKeep;
        }
        else if (receivedData[1] == 0x07 && receivedData[16] == 0xFF)
        {
          // USBSerial.print("DownBow");
          pastTime = timeKeep;
        }
        // USBSerial.println();
        // USBSerial.print("CurrentNote:");
        // USBSerial.print(currentNote);
        // USBSerial.print(" PastNote:");
        // USBSerial.print(pastNote);
        // USBSerial.print(" Hold:");
        // USBSerial.print(hold);
        // USBSerial.print(" pastHold:");
        // USBSerial.print(pastHold);
        // USBSerial.println();
      }
    }
    isNotPassed = timeKeep - pastTime < SUS_BORDER;
    goSign = isNotPassed && currentNote != 0;

    switch (state)
    {
    case NTH0:
      if (goSign)
      {
        state = ON;
      }
      else
      {
        state = NTH0;
      }
      break;
    case NTH1:
      if (goSign)
      {
        if (currentNote != prevLoopNote)
        {
          state = CHG;
        }
        else
        {
          state = NTH1;
        }
      }
      else
      {
        state = OFF;
      }
      break;
    case ON:
      synth.setNoteOff(0, pastNote, 0);
      synth.setNoteOff(0, prevLoopNote, 0);
      synth.setNoteOff(0, pastPastNote, 0);
      synth.setNoteOn(0, currentNote, VOLUME);
      if (goSign)
      {
        if (currentNote != prevLoopNote)
        {
          state = CHG;
        }
        else
        {
          state = NTH1;
        }
      }
      else
      {
        state = OFF;
      }
      break;

    case OFF:
      synth.setNoteOff(0, currentNote, 0);
      synth.setNoteOff(0, pastPastNote, 0);
      synth.setNoteOff(0, prevLoopNote, 0);
      synth.setNoteOff(0, pastNote, 0);
      if (goSign)
      {
        state = ON;
      }
      else
      {
        state = NTH0;
      }
      break;
    case CHG:
      synth.setNoteOff(0, prevLoopNote, 0);
      synth.setNoteOff(0, pastNote, 0);
      synth.setNoteOff(0, pastPastNote, 0);
      synth.setNoteOn(0, currentNote, VOLUME);
      if (goSign)
      {
        if (currentNote != prevLoopNote)
        {
          state = CHG;
        }
        else
        {
          state = NTH1;
        }
      }
      else
      {
        state = OFF;
      }
      break;
    }

    // 変化があったかどうかをチェック
    bool hasChanged = (currentNote != pastCurrentNote) ||
                      (goSign != pastGoSign) ||
                      (isNotPassed != pastIsNotPassed) ||
                      (state != pastState);

    // 変化があった場合のみ出力
    if (hasChanged)
    // if (false)
    {
      USBSerial.print(num++);
      USBSerial.print(" ");
      USBSerial.print("CurrentState:");
      USBSerial.print(" ");
      USBSerial.print(stateToString(state));
      USBSerial.print(" ");
      USBSerial.print(timeKeep - pastTime);
      USBSerial.print(" ");
      USBSerial.print("CurrentNote:");
      USBSerial.print(currentNote);
      USBSerial.print(" ");
      USBSerial.print("PastNote:");
      USBSerial.print(pastNote);
      USBSerial.print(" ");
      USBSerial.print("isNotPassed");
      USBSerial.print(" ");
      USBSerial.print(isNotPassed);
      USBSerial.print(" ");
      USBSerial.print("GoSign:");
      USBSerial.print(" ");
      USBSerial.println(goSign);
    }

    // 現在の値を「過去の値」として保存
    pastTimeKeep = timeKeep - pastTime;
    pastCurrentNote = currentNote;
    pastIsNotPassed = isNotPassed;
    pastGoSign = goSign;
    pastState = state;

    pastHold = hold;
    prevLoopNote = currentNote;
    // pastBow = valBow;
    // delay(1);
  }
}



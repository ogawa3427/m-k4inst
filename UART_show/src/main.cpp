#include <Arduino.h>
#include <M5Unified.h>
#include <M5UnitSynth.h>
#include <note.h>
#include <tone.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "nvs_flash.h"
#include "nvs.h"
// #include "Keyboard.h"

#define USB_SERIAL true

// https://github.com/espressif/esp-idf/blob/v5.3/examples/storage/nvs_rw_value/main/nvs_value_example_main.c

void readNVS(const char *key, String &value)
{
  nvs_handle_t my_handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    USBSerial.printf("[READ]Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
  else
  {
    USBSerial.println("[READ]NVS handle opened successfully");

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
        USBSerial.printf("[READ]Error (%s) reading %s!\n", esp_err_to_name(err), key);
      }
      delete[] buffer;
    }
    else if (err == ESP_ERR_NVS_NOT_FOUND)
    {
      USBSerial.printf("[READ]The value for %s is not initialized yet!\n", key);
    }
    else
    {
      USBSerial.printf("[READ]Error (%s) reading %s!\n", esp_err_to_name(err), key);
    }
    nvs_close(my_handle);
  }
}

// void storeReceivedData(const String &receivedData, String &key)
// {
//   key = receivedData;
//   USBSerial.printf("Stored data for key: %s\n", key.c_str());
//   USBSerial.printf("Value: %s\n", receivedData.c_str());
// }

void writeNVS(const char *key, const String &value)
{
  nvs_handle_t my_handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err == ESP_OK)
  {
    err = nvs_set_str(my_handle, key, value.c_str());
    if (err != ESP_OK)
    {
      USBSerial.printf("[WRITE]Failed to update value for key: %s, error: %s\n", key, esp_err_to_name(err));
    }
    else
    {
      USBSerial.printf("[WRITE]Value updated successfully for key: %s\n", key);
    }

    err = nvs_commit(my_handle);
    USBSerial.println("[WRITE]Committing updates in NVS ...");
    if (err != ESP_OK)
    {
      USBSerial.printf("[WRITE]Failed to commit updates for key: %s, error: %s\n", key, esp_err_to_name(err));
    }
    else
    {
      USBSerial.printf("[WRITE]Updates committed successfully for key: %s\n", key);
    }

    nvs_close(my_handle);
  }
  else
  {
    USBSerial.printf("[WRITE]Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
}

void printAsHEX(const String &receivedData)
{
  for (int i = 0; i < receivedData.length(); i++)
  {
    USBSerial.print(receivedData[i], HEX); // 受信したデータを16進数で出力
    USBSerial.print(" ");                  // 数字の間にスペースを入れる
  }
  USBSerial.println(); // 改行を出力
}

bool compare(const String &receivedData, const String &rootString)
{
  for (int i = 0; i < receivedData.length(); i++)
  {
    if (receivedData[i] != rootString[i])
    {
      return false;
    }
  }
  return true;
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
  String pull_rightClick;
  String pull_leftClick;
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

      // writeNVS("bowCfg_rClick", bowingKeyCfg.rightClick);
      // writeNVS("bowCfg_lClick", bowingKeyCfg.leftClick);
      // writeNVS("bowCfg_upBow", bowingKeyCfg.upBow);
      // writeNVS("bowCfg_downBow", bowingKeyCfg.downBow);
      // writeNVS("bowCfg_p_rC", bowingKeyCfg.pull_rightClick);
      // writeNVS("bowCfg_p_lC", bowingKeyCfg.pull_leftClick);

BowingKeyCfg readBowingKeyCfg()
{
  BowingKeyCfg cfg;
  String temp;
  readNVS("bowCfg_upBow", temp);
  cfg.upBow = String(temp.c_str());
  readNVS("bowCfg_downBow", temp);
  cfg.downBow = String(temp.c_str());
  readNVS("bowCfg_rClick", temp);
  cfg.rightClick = String(temp.c_str());
  readNVS("bowCfg_lClick", temp);
  cfg.leftClick = String(temp.c_str());
  readNVS("bowCfg_p_rC", temp);
  cfg.pull_rightClick = String(temp.c_str());
  readNVS("bowCfg_p_lC", temp);
  cfg.pull_leftClick = String(temp.c_str());
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
  START_CTR_KEY_CFG,
  ASK_CTR_KEY_CFG,
  CONFIRM_CTR_KEY_CFG,
  READ_CTR_KEY_CFG,
  MUS_CFG_START,
  MUS_CFG_BUF,
  ASK_MUS_CFG,
  CONFIRM_MUS_CFG,
  READ_MUS_CFG,
  TONE_CFG_START,
  ASK_TONE_CFG,
  CONFIRM_TONE_CFG,
  READ_TONE_CFG,
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

int promptBase1_index = 0;
int promptBase1_lastIndex = 0;

struct promptItem
{
  const char *prompt;
  String *strage;
};

promptItem promptBase1[] = {{.prompt = "Release [Enter]",
                             .strage = &ctrKeyCfg.pull_enter},
                            {.prompt = "Please press and hold [Up]",
                             .strage = &ctrKeyCfg.up},
                            {.prompt = "Release [Up]",
                             .strage = &ctrKeyCfg.pull_up},
                            {.prompt = "Please press and hold [Down]",
                             .strage = &ctrKeyCfg.down},
                            {.prompt = "Release [Down]",
                             .strage = &ctrKeyCfg.pull_down},
                            {.prompt = "Please press and hold [Esc]",
                             .strage = &ctrKeyCfg.esc},
                            {.prompt = "Release [Esc]",
                             .strage = &ctrKeyCfg.pull_esc}};

int lengthPromptBase1 = 7;

int promptBase2_index = 0;
int promptBase2_lastIndex = 0;
promptItem promptBase2[] = {{.prompt = "Push [R Click] and hold",
                             .strage = &bowingKeyCfg.rightClick},
                            {.prompt = "Release [R Click]",
                             .strage = &bowingKeyCfg.pull_rightClick},
                            {.prompt = "Push [L Click] and hold",
                             .strage = &bowingKeyCfg.leftClick},
                            {.prompt = "Release [L Click]",
                             .strage = &bowingKeyCfg.pull_leftClick},
                            {.prompt = "Scroll up once",
                             .strage = &bowingKeyCfg.upBow},
                            {.prompt = "Scroll down once",
                             .strage = &bowingKeyCfg.downBow}};

int lengthPromptBase2 = 6;

void promptingN(promptItem *promptBase1, int &index, int &lastIndex, OuterStates nextState, HardwareSerial *Serial2, String hexString, int lengthPromptBase)
{
  if (hexString != "" && hexString != "00")
  {
    *promptBase1[index].strage = hexString;
    index++;
    USBSerial.println("(promptingN)");
    USBSerial.println(index);

    if (index != lastIndex && index < lengthPromptBase)
    {
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.println(promptBase1[index].prompt);
      USBSerial.println(promptBase1[index].prompt);
      lastIndex = index;
    }

    if (index == lengthPromptBase)
    {
      USBSerial.println("(promptingN)nextState");
      outerState = nextState;
    }
  }
}

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

bool up_bow = false;
bool down_bow = false;
bool right_click = false;
bool left_click = false;

std::vector<uint16_t> keyboard_data;
std::vector<uint16_t> last_keyboard_data;

std::vector<uint16_t> additional;
std::vector<uint16_t> deletional;

std::vector<uint16_t> keys_queue;

void detection(String hexString, String receivedData)
{
  up_bow = false;
  down_bow = false;

  uint16_t devType = receivedData[4];
  uint16_t modKeysByte = receivedData[11];
  uint16_t ordinalKeysByte[6] = {receivedData[13], receivedData[14], receivedData[15], receivedData[16], receivedData[17], receivedData[18]};

#ifdef USB_SERIAL
  USBSerial.print("modKeysByte: ");
  USBSerial.println(modKeysByte);
  USBSerial.print("ordinalKeysByte1: ");
  USBSerial.println(ordinalKeysByte[0]);
  USBSerial.print("ordinalKeysByte2: ");
  USBSerial.println(ordinalKeysByte[1]);
  USBSerial.print("ordinalKeysByte3: ");
  USBSerial.println(ordinalKeysByte[2]);
  USBSerial.print("ordinalKeysByte4: ");
  USBSerial.println(ordinalKeysByte[3]);
  USBSerial.print("ordinalKeysByte5: ");
  USBSerial.println(ordinalKeysByte[4]);
  USBSerial.print("ordinalKeysByte6: ");
  USBSerial.println(ordinalKeysByte[5]);
#endif

  if (devType == 0x06)
  {

    // data.erase(data.begin(), data.begin() + 2);
    // for (int i = 0; i < 4; i++)
    // {
    //   if (data[i] != 0x00 && std::find(last_keyboard_data.begin(), last_keyboard_data.end(), data[i]) == last_keyboard_data.end())
    //   {
    //     additional.push_back(data[i]);
    //     USBSerial.printf("%02X ", data[i]);
    //   }
    //   if (data[i] != 0x00 && std::find(last_keyboard_data.begin(), last_keyboard_data.end(), data[i]) != last_keyboard_data.end())
    //   {
    //     deletional.push_back(data[i]);
    //     USBSerial.printf("%02X ", data[i]);
    //   }
    // }

    for (int i = 0; i < deletional.size(); i++)
    {
      keys_queue.erase(std::remove(keys_queue.begin(), keys_queue.end(), deletional[i]), keys_queue.end());
      USBSerial.printf("%02X ", deletional[i]);
    }

    for (int i = 0; i < additional.size(); i++)
    {
      keys_queue.insert(keys_queue.begin(), additional[i]);
      USBSerial.printf("%02X ", additional[i]);
    }

    for (int i = 0; i < keys_queue.size(); i++)
    {
      USBSerial.printf("%02X ", keys_queue[i]);
    }
    USBSerial.println();

    // last_keyboard_data = data;
    additional.clear();
    deletional.clear();
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

  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // OTA app partition table has a smaller NVS partition size than the non-OTA
    // partition table. This size mismatch may cause NVS initialization to fail.
    // If this happens, we erase NVS partition and initialize NVS again.
    // Once NVS is initialized, OTA app partition table will be used
    // regardless of the partition table in use.
    err = nvs_flash_erase();
    if (err != ESP_OK)
    {
      USBSerial.printf("Failed to erase NVS, error: %s\n", esp_err_to_name(err));
    }
    else
    {
      err = nvs_flash_init();
      if (err != ESP_OK)
      {
        USBSerial.printf("Failed to initialize NVS, error: %s\n", esp_err_to_name(err));
      }
    }
  }
  else if (err != ESP_OK)
  {
    USBSerial.printf("Failed to initialize NVS, error: %s\n", esp_err_to_name(err));
  }

  String value;
  readNVS("key", value);
  if (value == "")
  {
    writeNVS("key", "value");
  }

  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.startWrite();
  M5.Display.setCursor(0, 0);
  M5.Display.print(millis());
  M5.Display.endWrite();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.println("Hello!");

  ctrKeyCfg = readCtrKeyCfg();
  // cfg_ = askCtrKeyCfg();
  outerState = INIT;

  // USBSerial.println("3");
  // delay(1000);
  // USBSerial.println("2");
  // delay(1000);
  // USBSerial.println("1");
  // delay(1000);
  USBSerial.println("0");
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setBrightness(90);
  USBSerial.println("Hello!");
}

// CtrKeyCfg ctrKeyCfg;

// 変数の初期化部分に過去の値を保存するための変数を追加
uint32_t pastTimeKeep = 0;
int pastCurrentNote = 0;
bool pastGoSign = false;
int num = 0;

void loop()
{
  // outerState = MAIN;

  String receivedData = "";
  String hexString = "";
  if (Serial2.available())
  {
    receivedData = Serial2.readStringUntil('\n');
    for (int i = 0; i < receivedData.length(); i++)
    {
      char hex[3];
      sprintf(hex, "%02X", receivedData[i]);
      hexString += hex;
    }
  }
  // if (hexString != "" && hexString != "00")
  // {
  //   for (int i = 0; i < hexString.length() / 2; i++)
  //   {
  //     USBSerial.print(hexString[i * 2]);
  //     USBSerial.print(hexString[i * 2 + 1]);
  //     USBSerial.print(" ");
  //   }
  //   USBSerial.println();
  // }

  if (outerState == INIT)
  {

#ifdef USB_SERIAL
    USBSerial.println("INIT");
    USBSerial.println("Please press and hold [Enter]\nTo skip: set [Esc]");
#endif

    M5.Lcd.setCursor(0, 0);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.startWrite();
    M5.Lcd.println("Please press and hold [Enter]\nTo skip: set [Esc]");
    M5.Lcd.endWrite();

    outerState = START_CTR_KEY_CFG;
  }
  else if (outerState == START_CTR_KEY_CFG)
  {
    if (hexString != "" && hexString != "00")
    {
      USBSerial.println("START_CTR_KEY_CFG");
      if (compare(hexString, ctrKeyCfg.esc))
      {
        outerState = MUS_CFG_START;
        // outerState = MAIN;

        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.println("To start mouse config [Enter] To skip: [Esc] Be careful not to make signal");
      }
      else
      {
        ctrKeyCfg.enter = hexString;
        outerState = ASK_CTR_KEY_CFG;
      }
    }

    if (outerState == ASK_CTR_KEY_CFG)
    {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("Release [Enter]");
      USBSerial.println("-->ASK_CTR_KEY_CFG");
    }
  }
  else if (outerState == ASK_CTR_KEY_CFG)
  {
    promptingN(promptBase1, promptBase1_index, promptBase1_lastIndex, CONFIRM_CTR_KEY_CFG, &Serial2, hexString, lengthPromptBase1);

    if (outerState == CONFIRM_CTR_KEY_CFG)
    {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("Push 4 keys to confirm.\nOK:BtnA\nRetry:BtnB");

#ifdef USB_SERIAL
      USBSerial.println("Push 4 keys to confirm. OK:BtnA Retry:BtnB");
#endif
    }

#ifdef USB_SERIAL
    // USBSerial.print("Received Data: ");
    // USBSerial.println(hexString);
#endif
  }
  else if (outerState == CONFIRM_CTR_KEY_CFG)
  {
    // USBSerial.println("CONFIRM_CTR_KEY_CFG");
    M5.update();
    if (hexString != "" && hexString != "00")
    {
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextColor(YELLOW, BLACK);
      M5.Lcd.println("Push 4 keys to confirm.\nOK:BtnA\nRetry:BtnB");
      M5.Lcd.setTextColor(RED, WHITE);

      if (compare(hexString, ctrKeyCfg.enter))
      {
        M5.Lcd.println("[Enter]");
      }
      else if (compare(hexString, ctrKeyCfg.up))
      {
        M5.Lcd.println("[Up]");
      }
      else if (compare(hexString, ctrKeyCfg.down))
      {
        M5.Lcd.println("[Down]");
      }
      else if (compare(hexString, ctrKeyCfg.esc))
      {
        M5.Lcd.println("[Esc]");
      }
      else if (compare(hexString, ctrKeyCfg.pull_enter) || compare(hexString, ctrKeyCfg.pull_up) || compare(hexString, ctrKeyCfg.pull_down) || compare(hexString, ctrKeyCfg.pull_esc))
      {
        M5.Lcd.println("---");
      }
      else
      {
        M5.Lcd.println("Unknown");
      }
    }

    if (M5.BtnA.wasPressed())
    {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("Writing to NVS...");

#ifdef USB_SERIAL
      USBSerial.println("Writing to NVS...");
#endif

      writeNVS("ctrKeyCfg_enter", ctrKeyCfg.enter);
      writeNVS("ctrKeyCfg_up", ctrKeyCfg.up);
      writeNVS("ctrKeyCfg_down", ctrKeyCfg.down);
      writeNVS("ctrKeyCfg_esc", ctrKeyCfg.esc);

      writeNVS("p_enter", ctrKeyCfg.pull_enter);
      writeNVS("p_up", ctrKeyCfg.pull_up);
      writeNVS("p_down", ctrKeyCfg.pull_down);
      writeNVS("p_esc", ctrKeyCfg.pull_esc);

      M5.Lcd.println("Done!");

#ifdef USB_SERIAL
      USBSerial.println("Done!");
#endif

      ctrKeyCfg = readCtrKeyCfg();

#ifdef USB_SERIAL
      USBSerial.println("ctrKeyCfg:");
      USBSerial.println(ctrKeyCfg.enter);
      USBSerial.println(ctrKeyCfg.up);
      USBSerial.println(ctrKeyCfg.down);
      USBSerial.println(ctrKeyCfg.esc);
      USBSerial.println(ctrKeyCfg.pull_enter);
      USBSerial.println(ctrKeyCfg.pull_up);
      USBSerial.println(ctrKeyCfg.pull_down);
      USBSerial.println(ctrKeyCfg.pull_esc);
#endif

      outerState = MUS_CFG_START;

      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("To start mouse config [Enter] To skip: [Esc] Be careful not to make signal");
    }
    else if (M5.BtnB.wasPressed())
    {
      ctrKeyPhase = 0;
    }
  }
  else if (outerState == MUS_CFG_START)
  {
    if (hexString != "" && hexString != "00")
    {
      USBSerial.println("MUS_CFG_START");
      if (compare(hexString, ctrKeyCfg.esc))
      {
        outerState = MAIN;
      }
      else if (compare(hexString, ctrKeyCfg.enter))
      {
        outerState = MUS_CFG_BUF;
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.println("---");
      }
    }
  }
  else if (outerState == MUS_CFG_BUF)
  {
    if (hexString != "" && hexString != "00")
    {
      USBSerial.println("MUS_CFG_BUF");
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("Push [R Click] and hold");
      outerState = ASK_MUS_CFG;
    }
  }
  else if (outerState == ASK_MUS_CFG)
  {
    promptingN(promptBase2, promptBase2_index, promptBase2_lastIndex, CONFIRM_MUS_CFG, &Serial2, hexString, lengthPromptBase2);

    if (outerState == CONFIRM_MUS_CFG)
    {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.println("Do 4 actions to confirm.\nOK:BtnA\nRetry:BtnB");
    }
  }
  else if (outerState == CONFIRM_MUS_CFG)
  {
    M5.update();
    if (hexString != "" && hexString != "00")
    {
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextColor(YELLOW, BLACK);
      M5.Lcd.println("Do 4 actions to confirm.\nOK:BtnA\nRetry:BtnB");
      M5.Lcd.setTextColor(RED, WHITE);

      if (compare(hexString, bowingKeyCfg.rightClick))
      {
        M5.Lcd.println("[R Click]");
      }
      else if (compare(hexString, bowingKeyCfg.leftClick))
      {
        M5.Lcd.println("[L Click]");
      }
      else if (compare(hexString, bowingKeyCfg.upBow))
      {
        M5.Lcd.println("[Up Bow]");
      }
      else if (compare(hexString, bowingKeyCfg.downBow))
      {
        M5.Lcd.println("[Down Bow]");
      }
      else if (compare(hexString, bowingKeyCfg.pull_rightClick) || compare(hexString, bowingKeyCfg.pull_leftClick))
      {
        M5.Lcd.println("---");
      }
      else
      {
        M5.Lcd.println("Unknown");
      }
    }
    if (M5.BtnA.wasPressed())
    {
      writeNVS("bowCfg_rClick", bowingKeyCfg.rightClick);
      writeNVS("bowCfg_lClick", bowingKeyCfg.leftClick);
      writeNVS("bowCfg_upBow", bowingKeyCfg.upBow);
      writeNVS("bowCfg_downBow", bowingKeyCfg.downBow);
      writeNVS("bowCfg_p_rC", bowingKeyCfg.pull_rightClick);
      writeNVS("bowCfg_p_lC", bowingKeyCfg.pull_leftClick);

      bowingKeyCfg = readBowingKeyCfg();

#ifdef USB_SERIAL
      USBSerial.println("bowingKeyCfg:");
      USBSerial.println(bowingKeyCfg.rightClick);
      USBSerial.println(bowingKeyCfg.leftClick);
      USBSerial.println(bowingKeyCfg.upBow);
      USBSerial.println(bowingKeyCfg.downBow);
      USBSerial.println(bowingKeyCfg.pull_rightClick);
      USBSerial.println(bowingKeyCfg.pull_leftClick);
#endif

      outerState = MAIN;
    }
    else if (M5.BtnB.wasPressed())
    {
      outerState = MUS_CFG_BUF;
    }
  }
  // else if (outerState == MAIN)
  else if (false)
  {
    if (hexString != "" && hexString != "00")
    {
      detection(hexString, receivedData);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.println("Detection");
    }
    delay(1);
  }
  // else if (false)
  else if (outerState == MAIN)
  {
    timeKeep = millis();
    // if (timeKeep % 1000 == 0)
    // {
    //   USBSerial.println(timeKeep);
    // }
    // UARTでデータを受信
    // if (Serial2.available())
    if (hexString != "" && hexString != "00")
    {
      // String receivedData = Serial2.readStringUntil('\n');
      // for (int i = 0; i < receivedData.length(); i++)
      // {
      //   USBSerial.print(receivedData[i], HEX); // 受信したデータを16進数で出力
      //   USBSerial.print(" ");                  // 数字の間にスペースを入れる
      // }
      // USBSerial.println(); // 改行を出力
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
        // if (byte5 == 0x00 && byte15 == 0x00 && byte16 == 0xFF && byte17 == 0x00)
        if (hexString == bowingKeyCfg.upBow)
        // else if (byte5 == 0x00 && byte15 == 0x00 && byte16 == 0xFF && byte17 == 0x00)
        {
          USBSerial.print("UpScr");
          pastTime = timeKeep;
        }
        // else if (byte5 == 0x00 && byte15 == 0x00 && byte16 == 0x01 && byte17 == 0x00)
        if (hexString == bowingKeyCfg.downBow)
        {
          USBSerial.print("DownScr");
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

        // if (receivedData[1] == 0x07 && receivedData[16] == 0x01)
        if (hexString == bowingKeyCfg.upBow)
        {
          // USBSerial.print("UpBow");
          pastTime = timeKeep;
        }
        else if (hexString == bowingKeyCfg.downBow)
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

// #include "M5Unified.h"

// uint8_t  uartRxBuff[1024];
// int  rxPos = 0;
// int  cmdLength = 0;
// uint8_t  cmdType = 0;
// long lastRxReceive = 0;

// String deviceType[] = {"UNKNOWN", "POINTER", "MOUSE", "RESERVED", "JOYSTICK", "GAMEPAD", "KEYBOARD", "KEYPAD", "MULTI_AXIS", "SYSTEM"};
// String keyboardstring;

// #define MSG_TYPE_CONNECTED      0x01
// #define MSG_TYPE_DISCONNECTED   0x02
// #define MSG_TYPE_ERROR          0x03
// #define MSG_TYPE_DEVICE_POLL    0x04
// #define MSG_TYPE_DEVICE_STRING  0x05
// #define MSG_TYPE_DEVICE_INFO    0x06
// #define MSG_TYPE_HID_INFO       0x07
// #define MSG_TYPE_STARTUP        0x08

// void filterCommand(int buffLength, unsigned char *msgbuffer) {
//   int cmdLength = buffLength;
//   unsigned char msgType = msgbuffer[3];
//   unsigned char devType = msgbuffer[4];
//   unsigned char device = msgbuffer[5];
//   unsigned char endpoint = msgbuffer[6];
//   unsigned char idVendorL = msgbuffer[7];
//   unsigned char idVendorH = msgbuffer[8];
//   unsigned char idProductL = msgbuffer[9];
//   unsigned char idProductH = msgbuffer[10];
//   switch (msgType) {
//     case MSG_TYPE_CONNECTED:
//       USBSerial.print("Device Connected on port");
//       USBSerial.println(device);
//       break;
//     case MSG_TYPE_DISCONNECTED:
//       USBSerial.print("Device Disconnected on port");
//       USBSerial.println(device);
//       break;
//     case MSG_TYPE_ERROR:
//       USBSerial.print("Device Error ");
//       USBSerial.print(device);
//       USBSerial.print(" on port ");
//       USBSerial.println(devType);
//       break;
//     case MSG_TYPE_DEVICE_POLL:
//       USBSerial.print("Device HID Data from port: ");
//       USBSerial.print(device);
//       USBSerial.print(" , Length: ");
//       USBSerial.print(cmdLength);
//       USBSerial.print(" , Type: ");
//       USBSerial.print (deviceType[devType]);
//       USBSerial.print(" , ID: ");
//       for (int j = 0; j < 4; j++) {
//         USBSerial.print("0x");
//         USBSerial.print(msgbuffer[j + 7], HEX);
//         USBSerial.print(" ");
//       }
//       USBSerial.print(" ,  ");
//       for (int j = 0; j < cmdLength; j++) {
//         USBSerial.print("0x");
//         USBSerial.print(msgbuffer[j + 11], HEX);
//         USBSerial.print(" ");
//       }
//       USBSerial.println();
//       break;
//     case MSG_TYPE_DEVICE_STRING:
//       USBSerial.print("Device String port ");
//       USBSerial.print(devType);
//       USBSerial.print(" Name: ");
//       for (int j = 0; j < cmdLength; j++)
//         USBSerial.write(msgbuffer[j + 11]);
//       USBSerial.println();
//       break;
//     case MSG_TYPE_DEVICE_INFO:
//       USBSerial.print("Device info from port");
//       USBSerial.print(device);
//       USBSerial.print(", Descriptor: ");
//       for (int j = 0; j < cmdLength; j++) {
//         USBSerial.print("0x");
//         USBSerial.print(msgbuffer[j + 11], HEX);
//         USBSerial.print(" ");
//       }
//       USBSerial.println();
//       break;
//     case MSG_TYPE_HID_INFO:
//       USBSerial.print("HID info from port");
//       USBSerial.print(device);
//       USBSerial.print(", Descriptor: ");
//       for (int j = 0; j < cmdLength; j++) {
//         USBSerial.print("0x");
//         USBSerial.print(msgbuffer[j + 11], HEX);
//         USBSerial.print(" ");
//       }
//       USBSerial.println();
//       break;
//     case MSG_TYPE_STARTUP:
//       USBSerial.println("USB host ready");
//       break;
//   }
// }

// void setup(void) {
//   auto cfg = M5.config();
//   // Serial1.begin(1000000, SERIAL_8N1, 36, 26);
//   // delay(1000);
//   // Serial.println("OK There");
//   M5.begin(cfg);
//   Serial2.begin(115200, SERIAL_8N1, 5, 6);
//   USBSerial.begin(115200);
//   delay(1000);
//   USBSerial.println("OK There");
// }

// void loop() {
//   while (Serial2.available()) {
//     lastRxReceive = millis();
//     //Serial.print("h0x");//Only for Debug
//     //Serial.print(Serial1.peek(),HEX);//Only for Debug
//     //Serial.print(" ");//Only for Debug
//     uartRxBuff[rxPos] = Serial2.read();
//     if (rxPos == 0 && uartRxBuff[rxPos] == 0xFE) {
//       cmdType = 1;
//     } else if (rxPos == 1 && cmdType == 1) {
//       cmdLength = uartRxBuff[rxPos];
//     } else if (rxPos == 2 && cmdType == 1) {
//       cmdLength += (uartRxBuff[rxPos] << 8);
//       //printf( "Length: %i\n", cmdLength);//Only for Debug
//     } else if (cmdType == 0 && uartRxBuff[rxPos] == '\n') {
//       USBSerial.printf("No COMMAND Received\n");
//       for (uint8_t i = 0; i < rxPos; i ++ ) {
//         USBSerial.printf( "0x%02X ", uartRxBuff[i]);
//       }
//       USBSerial.println();
//       rxPos = 0;
//       cmdType = 0;
//     }
//     if ((rxPos > 0 && rxPos == cmdLength + 11 && cmdType) || rxPos > 1024) {
//       filterCommand(cmdLength, uartRxBuff);
//       for (int i = 0; i < rxPos; i ++ ) {
//         //USBSerial.printf( "0x%02X ", uartRxBuff[i]);//Only for Debug
//       }
//       //USBSerial.println();//Only for Debug
//       rxPos = 0;
//       cmdType = 0;
//     } else {
//       rxPos++;
//     }
//   }
//   rxPos = 0;

//   if (USBSerial.available()) {
//     Serial2.write(USBSerial.read());
//   }
// }

#include <Arduino.h>
#include <M5Unified.h>
// #include <M5UnitSynth.h>


// #define USB_SERIAL (1)
// #define SKIP_ALL_CFG (1)

// Refs
// https://github.com/espressif/esp-idf/blob/v5.3/examples/storage/nvs_rw_value/main/nvs_value_example_main.c


void setup()
{
  Serial2.begin(115200, SERIAL_8N1, 5, 6);

  // #ifdef USB_SERIAL
  USBSerial.begin(115200);
  // #endif

  // synth.begin(&Serial1, UNIT_SYNTH_BAUD, 1, 2);
  // synth.setInstrument(0, 0, INSTRUMENT_VIOLIN);
  // // synth.setNoteOn(0, NOTE_C6, VOLUME);
  // // delay(1000);
  // synth.setNoteOff(0, NOTE_C6, 0);

  // // Initialize NVS
  // esp_err_t err = nvs_flash_init();
  // if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  // {
  //   // OTA app partition table has a smaller NVS partition size than the non-OTA
  //   // partition table. This size mismatch may cause NVS initialization to fail.
  //   // If this happens, we erase NVS partition and initialize NVS again.
  //   // Once NVS is initialized, OTA app partition table will be used
  //   // regardless of the partition table in use.
  //   err = nvs_flash_erase();
  //   if (err != ESP_OK)
  //   {
  //     USBSerial.printf("Failed to erase NVS, error: %s\n", esp_err_to_name(err));
  //   }
  //   else
  //   {
  //     err = nvs_flash_init();
  //     if (err != ESP_OK)
  //     {
  //       USBSerial.printf("Failed to initialize NVS, error: %s\n", esp_err_to_name(err));
  //     }
  //   }
  // }
  // else if (err != ESP_OK)
  // {
  //   USBSerial.printf("Failed to initialize NVS, error: %s\n", esp_err_to_name(err));
  // }

  // String value;
  // readNVS("key", value);
  // if (value == "")
  // {
  //   writeNVS("key", "value");
  // }

  auto cfg = M5.config();
  M5.begin(cfg);
  // M5.Display.startWrite();
  // M5.Display.setCursor(0, 0);
  // M5.Display.print(millis());
  // M5.Display.endWrite();
  // M5.Lcd.setTextSize(2);
  // M5.Lcd.setTextColor(YELLOW);
  // M5.Lcd.println("Hello!");

  // ctrKeyCfg = readCtrKeyCfg();
  // bowingKeyCfg = readBowingKeyCfg();
  // outerState = INIT;

  // USBSerial.println("3");
  // delay(1000);
  // USBSerial.println("2");
  // delay(1000);
  // USBSerial.println("1");
  // delay(1000);
  // USBSerial.println("0");
  // M5.Lcd.fillScreen(BLACK);
  // M5.Lcd.setBrightness(90);
  // USBSerial.println("Hello!");
}



void loop()
{
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
    USBSerial.print("hexString: ");
    USBSerial.println(hexString);
  }
}
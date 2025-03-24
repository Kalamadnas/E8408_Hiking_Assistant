#include "config.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

TTGOClass *watch;
TFT_eSPI *tft;
BMA *sensor;
bool irq = false;
bool started = false;
bool sideButton = false;
bool menuTrigger = false;
lv_obj_t *btn1;
lv_obj_t *label;
lv_obj_t *slider_label_stride;
lv_obj_t *slider_stride;
lv_obj_t *slider_label_weight;
lv_obj_t *slider_weight;
char buf[20];
char buf_2[16];
unsigned long startTime;
int duration;
int weight;
int stride;
int distance;
uint32_t stepCount;

/*
0 = Default start state
1 = Start session state
2 = Settings menu
*/
int watchState = 0;

BluetoothSerial SerialBT;

// Functions for GUI events
static void slider_event_cb(lv_obj_t *slider, lv_event_t e)
{
    lv_snprintf(buf, sizeof(buf), "Stride: %d cm", (int)lv_slider_get_value(slider));
    stride = (int)lv_slider_get_value(slider);
    lv_label_set_text(slider_label_stride, buf);
}
static void slider_event_cb_2(lv_obj_t *slider, lv_event_t e)
{
    lv_snprintf(buf_2, sizeof(buf_2), "Weight: %d kg", (int)lv_slider_get_value(slider));
    weight = (int)lv_slider_get_value(slider);
    lv_label_set_text(slider_label_weight, buf_2);
}
static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        menuTrigger = true;
        stateManagement();
    }
}

// Used for handling transfers between different states
void stateManagement(){
    if (watchState == 0 && menuTrigger) {
        Serial.printf("Clicked\n");
        lv_obj_set_hidden(slider_stride, false);
        lv_obj_set_hidden(slider_weight, false);
        menuTrigger = false;
        watchState = 2;
        lv_label_set_text(label, "Save settings");
    }
    else if (watchState == 0) {
        watchState = 1;
        sensor->resetStepCounter();
        lv_obj_set_click(btn1, false);
        tft->fillScreen(TFT_WHITE);
        startTime = millis();
    }
    else if (watchState == 1) {
        watchState = 0;
        duration = (int)((millis() - startTime) / 1000);
        // Send data via BLuetooth
        SerialBT.print("W:");
        SerialBT.print(weight);
        SerialBT.print(",T:");
        SerialBT.print(duration);
        SerialBT.print(",S:");
        SerialBT.print(stepCount+1);
        SerialBT.print(",D:");
        SerialBT.print(distance);
        SerialBT.print("\n");
        Serial.print("W:");
        Serial.print(weight);
        Serial.print(",T:");
        Serial.print(duration);
        Serial.print(",S:");
        Serial.print(stepCount+1);
        Serial.print(",D:");
        Serial.print(distance);
        Serial.print("\n");
        sensor->resetStepCounter();
        lv_obj_set_click(btn1, true);
        lv_obj_set_hidden(btn1, false);
        tft->fillScreen(TFT_WHITE);
    }
    else if (watchState == 2) {
        lv_obj_set_hidden(slider_stride, true);
        lv_obj_set_hidden(slider_weight, true);
        menuTrigger = false;
        watchState = 0;
        lv_label_set_text(label, "Change settings");
        tft->fillScreen(TFT_WHITE);
    }
}

void setup(){
    Serial.begin(115200);
    SerialBT.begin("Hiking watch");

    // Initializing and setting up watch functions and interrupts
    watch = TTGOClass::getWatch();
    watch->begin();
    watch->openBL();
    watch->lvgl_begin();

    tft = watch->tft;
    sensor = watch->bma;

    Acfg cfg;
    cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    cfg.range = BMA4_ACCEL_RANGE_2G;
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;

    sensor->accelConfig(cfg);
    sensor->enableAccel();

    pinMode(BMA423_INT1, INPUT);
    attachInterrupt(BMA423_INT1, [] {
        irq = 1;
    }, RISING);

    sensor->enableFeature(BMA423_STEP_CNTR, true);
    sensor->resetStepCounter();
    sensor->enableStepCountInterrupt();

    // Setting up the screen 
    tft->setTextColor(TFT_BLACK, TFT_WHITE);
    tft->drawString("BMA423 StepCount", 3, 50, 4);
    tft->setTextFont(4);
    tft->fillScreen(TFT_BLACK);

    // Setting up GUI inputs
    btn1 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(btn1, event_handler);
    lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, -60);

    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Change settings");

    slider_stride = lv_slider_create(lv_scr_act(), NULL);
    lv_obj_align(slider_stride, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(slider_stride, slider_event_cb);
    lv_slider_set_range(slider_stride, 0, 200);

    lv_slider_set_anim_time(slider_stride, 2000);
    lv_obj_set_size(slider_stride, lv_obj_get_width(slider_stride) * 0.7, lv_obj_get_height(slider_stride) * 3);
    lv_obj_set_x(slider_stride, 30);
    slider_label_stride = lv_label_create(slider_stride, NULL);
    lv_label_set_text(slider_label_stride, "Stride: 0 cm");
    lv_slider_set_type(slider_stride, LV_BAR_TYPE_SYMMETRICAL);
    lv_obj_align(slider_label_stride, slider_stride, LV_ALIGN_IN_LEFT_MID, 10, 0);
    lv_obj_set_size(slider_label_stride, lv_obj_get_width(slider_label_stride) * 5, lv_obj_get_height(slider_label_stride) * 6);

    slider_weight = lv_slider_create(lv_scr_act(), NULL);
    lv_obj_align(slider_weight, NULL, LV_ALIGN_CENTER, 0, 60);
    lv_obj_set_event_cb(slider_weight, slider_event_cb_2);
    lv_slider_set_range(slider_weight, 0, 150);

    lv_slider_set_anim_time(slider_weight, 2000);
    lv_obj_set_size(slider_weight, lv_obj_get_width(slider_weight) * 0.7, lv_obj_get_height(slider_weight) * 3);
    lv_obj_set_x(slider_weight, 30);
    slider_label_weight = lv_label_create(slider_weight, NULL);
    lv_label_set_text(slider_label_weight, "Weight: 0 kg");
    lv_slider_set_type(slider_weight, LV_BAR_TYPE_SYMMETRICAL);
    lv_obj_align(slider_label_weight, slider_weight, LV_ALIGN_IN_LEFT_MID, 10, 0);
    lv_obj_set_size(slider_label_weight, lv_obj_get_width(slider_label_weight) * 5, lv_obj_get_height(slider_label_weight) * 6);

    lv_obj_set_hidden(slider_stride, true);
    lv_obj_set_hidden(slider_weight, true);

    // Reconfiguring power button to start session signal
    pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(AXP202_INT, [] {
        sideButton = true;
    }, FALLING);
}

void loop() {
    lv_task_handler();
    if (sideButton){
      sideButton = false;
      watch->power->readIRQ();
      stateManagement();
      watch->power->clearIRQ();
    }
    if (SerialBT.available()) {
      Serial.write(SerialBT.read());
    }
    // Only reading interrupts if watch is in active session state 1
    if (watchState == 1) {
      if (irq) {
          irq = 0;
          bool  rlst;
          do {
              // Read the BMA423 interrupt status,
              // need to wait for it to return to true before continuing
              rlst =  sensor->readInterrupt();
          } while (!rlst);

          // Check if it is a step interrupt
          if (sensor->isStepCounter()) {
              // Get step data from register
              stepCount = sensor->getCounter();
              distance = (int)((stepCount * stride) / 100);
              // Printing active session statistics
              tft->setCursor(45, 118);
              tft->print("StepCount:");
              tft->println(stepCount);
          }
      }
      tft->setCursor(45, 118);
      tft->print("\n");
      tft->setCursor(45, tft->getCursorY());
      tft->print("Distance:");
      tft->print(distance);
      tft->print("m");

      tft->setCursor(45, 118);
      tft->print("\n");
      tft->print("\n");
      tft->setCursor(45, tft->getCursorY());
      tft->print("Time:");
      tft->print((int)((millis() - startTime) / 1000));
      tft->print("s");
    }
    delay(20);
}
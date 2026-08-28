#include <Arduino.h>

#define LED_TX_PIN PB6
#define LED_RX_PIN PB7
#define HC12_SET_PIN PA8


HardwareSerial HC12Serial(PA10, PA9);

// 16MHz HSE / 2 = 8MHz → PLL × 9 = 72MHz SYSCLK, /1.5 = 48MHz USB
extern "C" void SystemClock_Config(void) {
  RCC_OscInitTypeDef osc = {0};
  RCC_ClkInitTypeDef clk = {0};
  RCC_PeriphCLKInitTypeDef periph = {0};

  osc.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
  osc.HSEState        = RCC_HSE_ON;
  osc.HSEPredivValue  = RCC_HSE_PREDIV_DIV2;
  osc.PLL.PLLState    = RCC_PLL_ON;
  osc.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
  osc.PLL.PLLMUL      = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&osc);

  clk.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                       RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  clk.APB1CLKDivider = RCC_HCLK_DIV2;
  clk.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2);

  periph.PeriphClockSelection = RCC_PERIPHCLK_USB;
  periph.UsbClockSelection    = RCC_USBCLKSOURCE_PLL_DIV1_5;
  HAL_RCCEx_PeriphCLKConfig(&periph);
}

static uint32_t txLedOff = 0;
static uint32_t rxLedOff = 0;

static void flashTx() { analogWrite(LED_TX_PIN, 40); txLedOff = millis() + 30; }
static void flashRx() { digitalWrite(LED_RX_PIN, HIGH); rxLedOff = millis() + 30; }

static void hc12FactoryReset() {
  Serial.println("putting HC12 into AT mode...");
  digitalWrite(HC12_SET_PIN, LOW);
  delay(500); // wait for HC12 to enter AT mode and finish sending its version banner

  while (HC12Serial.available()) HC12Serial.read();

  HC12Serial.print("AT+DEFAULT\r\n");

  uint32_t start = millis();
  String resp = "";
  while (millis() - start < 600) {
    if (HC12Serial.available()) resp += (char)HC12Serial.read();
  }

  if (resp.indexOf("OK") >= 0) {
    Serial.println("HC12: factory reset OK");
  } else if (resp != "") {
    Serial.print("HC12: unexpected response: '");
    Serial.print(resp);
    Serial.println("'");
  } else {
    Serial.println("HC12: no response");
  }

  digitalWrite(HC12_SET_PIN, HIGH);
  delay(80);
}

void setup() {
  pinMode(LED_TX_PIN, OUTPUT);
  pinMode(LED_RX_PIN, OUTPUT);
  pinMode(HC12_SET_PIN, OUTPUT);
  digitalWrite(HC12_SET_PIN, HIGH);

  Serial.begin(9600);
  HC12Serial.begin(9600);
  
  uint32_t start = millis();
  while (!Serial && millis() - start < 3000);

  hc12FactoryReset();
  Serial.println("RocketLink ready.");
}

void loop() {
  uint32_t now = millis();

  if (now >= txLedOff) analogWrite(LED_TX_PIN, 0);
  if (now >= rxLedOff) digitalWrite(LED_RX_PIN, LOW);

  // PC -> HC12
  while (Serial.available()) {
    HC12Serial.write(Serial.read());
    flashTx();
  }

  // HC12 -> PC
  while (HC12Serial.available()) {
    Serial.write(HC12Serial.read());
    flashRx();
  }
}

#define CHAIN_PULL_PIN 14

void toggleShade() {
  digitalWrite(CHAIN_PULL_PIN, LOW);
  delay(500);
  digitalWrite(CHAIN_PULL_PIN, HIGH);
}

void setup() {
  digitalWrite(CHAIN_PULL_PIN, HIGH);
  pinMode(CHAIN_PULL_PIN, OUTPUT_OPEN_DRAIN);

  Serial.begin(115200);
  delay(500);

  toggleShade();
}

void loop() {
  static unsigned long startTime = millis();
  unsigned long duration = millis() - startTime;
  if (duration >= 24 * 60 * 60 * 1000) {
    startTime = millis();
    toggleShade();
  } else if (duration >= 9 * 60 * 60 * 1000) {
    toggleShade();
  }
  delay(10000);
}

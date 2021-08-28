#define CHAIN_PULL_PIN 2

void toggleShade() {
  digitalWrite(CHAIN_PULL_PIN, HIGH);
  delay(500);
  digitalWrite(CHAIN_PULL_PIN, LOW);
}

void setup() {
  pinMode(CHAIN_PULL_PIN, OUTPUT);
  digitalWrite(CHAIN_PULL_PIN, LOW);

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

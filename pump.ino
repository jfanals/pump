const int flowSensorPin = 2;
const int pumpPin = 13;

const unsigned long PRIME_DURATION = 5 * 1000; // 5 seconds
const unsigned long REFILL_WAIT = 10 * 1000; // 10 seconds
const unsigned long MAX_PUMP_TIME = 60 * 1000; // 1 minute
const unsigned long NO_FLOW_DEBOUNCE = 5 * 1000; // 5 seconds

enum PumpState {
  PRIMING,
  PUMPING,
  STOPPED,
  WAITING
};

PumpState currentState = PRIMING;
unsigned long stateStartTime = 0;
unsigned long noFlowStartTime = 0;

void setup() {
  Serial.begin(9600);
  pinMode(pumpPin, OUTPUT);
  pinMode(flowSensorPin, INPUT_PULLUP);
  stateStartTime = millis();
  Serial.println("Pump Control System Started");
  Serial.println("State: PRIMING");
}

void loop() {
  unsigned long currentTime = millis();
  bool flowDetected = digitalRead(flowSensorPin) == LOW;
  
  // Debug info every 10 seconds
  static unsigned long lastDebug = 0;
  if (currentTime - lastDebug >= 10000) {
    Serial.print("Debug - Current State: ");
    switch (currentState) {
      case PRIMING: Serial.print("PRIMING"); break;
      case PUMPING: Serial.print("PUMPING"); break;
      case STOPPED: Serial.print("STOPPED"); break;
      case WAITING: Serial.print("WAITING"); break;
    }
    Serial.print(", Time in state: ");
    Serial.print((currentTime - stateStartTime) / 1000);
    Serial.print("s, Flow: ");
    Serial.println(flowDetected ? "YES" : "NO");
    lastDebug = currentTime;
  }
  
  switch (currentState) {
    case PRIMING:
      digitalWrite(pumpPin, HIGH);
      
      if (flowDetected) {
        Serial.println("Flow detected! State: PUMPING");
        currentState = PUMPING;
        stateStartTime = currentTime;
      } else if (currentTime - stateStartTime >= PRIME_DURATION) {
        Serial.println("Priming timeout. State: STOPPED");
        currentState = STOPPED;
        stateStartTime = currentTime;
        digitalWrite(pumpPin, LOW);
      }
      break;
      
    case PUMPING:
      digitalWrite(pumpPin, HIGH);
      
      if (!flowDetected) {
        if (noFlowStartTime == 0) {
          Serial.println("No flow detected, starting debounce timer");
          noFlowStartTime = currentTime;
        } else if (currentTime - noFlowStartTime >= NO_FLOW_DEBOUNCE) {
          Serial.println("No flow for 5 seconds. State: STOPPED");
          currentState = STOPPED;
          stateStartTime = currentTime;
          digitalWrite(pumpPin, LOW);
          noFlowStartTime = 0;
        }
      } else {
        if (noFlowStartTime != 0) {
          Serial.println("Flow resumed");
        }
        noFlowStartTime = 0;
      }
      
      if (currentTime - stateStartTime >= MAX_PUMP_TIME) {
        Serial.println("Max pump time reached. State: STOPPED");
        currentState = STOPPED;
        stateStartTime = currentTime;
        digitalWrite(pumpPin, LOW);
        noFlowStartTime = 0;
      }
      break;
      
    case STOPPED:
      digitalWrite(pumpPin, LOW);
      Serial.println("State: WAITING (10 seconds)");
      currentState = WAITING;
      stateStartTime = currentTime;
      noFlowStartTime = 0;
      break;
      
    case WAITING:
      digitalWrite(pumpPin, LOW);
      
      if (currentTime - stateStartTime >= REFILL_WAIT) {
        Serial.print("Wait complete after ");
        Serial.print((currentTime - stateStartTime) / 1000);
        Serial.println(" seconds. State: PRIMING");
        currentState = PRIMING;
        stateStartTime = currentTime;
        noFlowStartTime = 0;
      } else {
        // Show waiting progress every 15 seconds
        static unsigned long lastWaitLog = 0;
        if (currentTime - lastWaitLog >= 15000) {
          unsigned long timeInWait = currentTime - stateStartTime;
          if (timeInWait < REFILL_WAIT) {
            unsigned long remainingTime = (REFILL_WAIT - timeInWait) / 1000;
            Serial.print("Still waiting... ");
            Serial.print(remainingTime);
            Serial.println(" seconds remaining");
          }
          lastWaitLog = currentTime;
        }
      }
      break;
  }
}
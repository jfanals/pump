const int flowSensorPin = 2;
const int pumpPin = 7;
const int ledPin = 13;

const unsigned long PRIME_DURATION = 15000UL; // 15 seconds
// const unsigned long REFILL_WAIT = 60000UL; // 1 minute (for testing)
const unsigned long REFILL_WAIT = 10800000UL; // 3 hour
const unsigned long MAX_PUMP_TIME = 600000UL; // 10 minutes
const unsigned long NO_FLOW_DEBOUNCE = 5000UL; // 5 seconds
const unsigned long BLINK_DURATION = 500UL; // 500ms blink
const unsigned long PAUSE_DURATION = 2000UL; // 2 second pause

enum PumpState {
  PRIMING,
  PUMPING,
  STOPPED,
  WAITING
};

PumpState currentState = PRIMING;
unsigned long stateStartTime = 0;
unsigned long noFlowStartTime = 0;
unsigned long totalPumpTimeMilliseconds = 0;
unsigned long pumpSessionStartTime = 0;

void setup() {
  Serial.begin(9600);
  pinMode(pumpPin, OUTPUT);
  pinMode(flowSensorPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  stateStartTime = millis();
  Serial.println("Pump Control System Started");
  Serial.println("State: PRIMING");
}

void displayPumpMinutes(int totalMinutes) {
  static unsigned long blinkStartTime = 0;
  static int currentBlink = 0;
  static bool ledState = false;
  static bool inPause = false;
  static unsigned long pauseStartTime = 0;
  
  unsigned long currentTime = millis();
  
  // Initialize timing on first call
  if (blinkStartTime == 0) {
    blinkStartTime = currentTime;
    currentBlink = 0;
    ledState = false;
    inPause = false;
  }
  
  // Handle pause period (2 seconds between cycles)
  if (inPause) {
    digitalWrite(ledPin, LOW);
    if (currentTime - pauseStartTime >= PAUSE_DURATION) {
      inPause = false;
      currentBlink = 0;
      blinkStartTime = currentTime;
      ledState = false;
    }
    return;
  }
  
  // Handle blinking
  if (currentBlink < totalMinutes) {
    if (currentTime - blinkStartTime >= BLINK_DURATION) {
      ledState = !ledState;
      digitalWrite(ledPin, ledState ? HIGH : LOW);
      blinkStartTime = currentTime;
      
      // If LED just turned off, we completed one blink
      if (!ledState) {
        currentBlink++;
      }
    }
  } else {
    // All blinks done, start pause
    digitalWrite(ledPin, LOW);
    inPause = true;
    pauseStartTime = currentTime;
  }
}

void loop() {
  unsigned long currentTime = millis();
  bool flowDetected = digitalRead(flowSensorPin) == LOW;
  int totalMinutes = totalPumpTimeMilliseconds / 60000UL; // Convert ms to minutes
  
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
      digitalWrite(pumpPin, LOW);
      
      if (flowDetected) {
        Serial.println("Flow detected! State: PUMPING");
        pumpSessionStartTime = currentTime; // Start timing this pumping session
        currentState = PUMPING;
        stateStartTime = currentTime;
      } else if (currentTime - stateStartTime >= PRIME_DURATION) {
        Serial.println("Priming timeout. State: STOPPED");
        currentState = STOPPED;
        stateStartTime = currentTime;
        digitalWrite(pumpPin, HIGH);
      }
      break;
      
    case PUMPING:
      digitalWrite(pumpPin, LOW);
      
      if (!flowDetected) {
        if (noFlowStartTime == 0) {
          Serial.println("No flow detected, starting debounce timer");
          noFlowStartTime = currentTime;
        } else if (currentTime - noFlowStartTime >= NO_FLOW_DEBOUNCE) {
          // Add pump session time to total
          unsigned long sessionTimeMs = currentTime - pumpSessionStartTime;
          totalPumpTimeMilliseconds += sessionTimeMs;
          Serial.print("No flow for 5 seconds. State: STOPPED. Session pumped for ");
          Serial.print(sessionTimeMs / 1000UL);
          Serial.print(" seconds. Total: ");
          Serial.print(totalPumpTimeMilliseconds / 1000UL);
          Serial.println(" seconds");
          currentState = STOPPED;
          stateStartTime = currentTime;
          digitalWrite(pumpPin, HIGH);
          noFlowStartTime = 0;
        }
      } else {
        if (noFlowStartTime != 0) {
          Serial.println("Flow resumed");
        }
        noFlowStartTime = 0;
      }
      
      if (currentTime - stateStartTime >= MAX_PUMP_TIME) {
        // Add pump session time to total
        unsigned long sessionTimeMs = currentTime - pumpSessionStartTime;
        totalPumpTimeMilliseconds += sessionTimeMs;
        Serial.print("Max pump time reached. State: STOPPED. Session pumped for ");
        Serial.print(sessionTimeMs / 1000UL);
        Serial.print(" seconds. Total: ");
        Serial.print(totalPumpTimeMilliseconds / 1000UL);
        Serial.println(" seconds");
        currentState = STOPPED;
        stateStartTime = currentTime;
        digitalWrite(pumpPin, HIGH);
        noFlowStartTime = 0;
      }
      break;
      
    case STOPPED:
      digitalWrite(pumpPin, HIGH);
      Serial.print("State: WAITING (1 hour) - Total pump time: ");
      Serial.print(totalPumpTimeMilliseconds / 1000UL);
      Serial.print(" seconds (");
      Serial.print(totalMinutes);
      Serial.println(" minutes)");
      currentState = WAITING;
      stateStartTime = currentTime;
      noFlowStartTime = 0;
      break;
      
    case WAITING:
      digitalWrite(pumpPin, HIGH);
      
      // Display total pump minutes via LED blinking
      if (totalMinutes > 0) {
        displayPumpMinutes(totalMinutes);
      }
      
      if (currentTime - stateStartTime >= REFILL_WAIT) {
        Serial.print("Wait complete after ");
        Serial.print((currentTime - stateStartTime) / 1000);
        Serial.println(" seconds. State: PRIMING");
        digitalWrite(ledPin, LOW); // Turn off LED
        currentState = PRIMING;
        stateStartTime = currentTime;
        noFlowStartTime = 0;
      }
      break;
  }
}
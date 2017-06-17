#include <SevenSegmentFun.h>
#include <SevenSegmentExtended.h>
#include <SevenSegmentTM1637.h>

const byte PIN_CLK = 0;   // define CLK pin (any digital pin)
const byte PIN_DIO = 1;   // define DIO pin (any digital pin)
SevenSegmentExtended    display(PIN_CLK, PIN_DIO);

int quizWinner = -1;
int modesIndex = 0; //quiz
String modes[] = {"QUIZ", "DISC", "HORN", "30 S", };
int maxModes = 3;
double timerInitValue = 30;
double timerCounter = timerInitValue;
bool runTimer;
int buttonInterval = 300;
int buzzerInterval = 200;
unsigned long prevMillis = 0;
int timerInterval = 10;
int pinc = 0;
unsigned long currentMillis = 0;
int seconds = 0;
int centisecond = 0;
const int RED_BUTTON_PIN = 8;
const int GREEN_BUTTON_PIN = 9;

const bool DEBUG = false;

// Printing methods
void print(String input) {
  if (DEBUG) {
    Serial.println(input);
  } else {
    display.print(input);
  }
}

void printTime(int seconds, int centisecond) {
  if (DEBUG) {
    // Serial.println(centisecond);
  } else {
    display.printTime(seconds, centisecond, false);
  }
}

void initPrint() {
  if (DEBUG) {
    Serial.begin(9600);
  } else {
    display.begin();            // initializes the display
    display.setBacklight(100);  // set the brightness to 100 %
  }
}

void clearDisplay() {
  if (!DEBUG) {
    display.clear();
  } else {
    Serial.println("Display has been cleared");
  }
}
// End printing methods

void setup()
{
  DDRC =  B11100100;
  DDRD =  B11111111;
  PORTD = B11111100;
  DDRB =  B11111100;

  initPrint();
  print(modes[modesIndex]);
}

unsigned long buttonPressedDuration = 0;
bool buttonPressed(int pin) {
  buttonPressedDuration = 0;
  while (digitalRead(pin) == 1) {
    if (buttonPressedDuration == 0) {
      buttonPressedDuration = millis();
    }
  }

  if (buttonPressedDuration != 0 && (buttonPressedDuration - millis()) > 10) {
    return true;
  }
  return false;
}

void loop()
{
  // copy();
  quiz();
  disco();
  horn();
  thirtySeconds();
  cycleModes();
}

const int MAX_SEQUENCE = 20;
const int MIN_DELAY = 1;
const int SEQUENCE_STEP = 50;
const int DEFAULT_SEQUENCE = 2;
const int DEFAULT_SEQUENCE_DELAY = 1000;
const int LEVEL_STEP = 2;
const int RANDOM_BUTTON_COUNT = 3;
const int USER_FEEDBACK_DELAY = 800;

bool repeat = false;
bool sequenceError = false;
bool sequenceCompleted = true;
int lastButtonPressed = -1;
int sequenceLength = DEFAULT_SEQUENCE;
int sequenceDelay = DEFAULT_SEQUENCE_DELAY;
int sequence[MAX_SEQUENCE];
int lights[4] = {1, 2, 8, 16};
int randomLight = -1;
int previousRandomLight = -1;
int i = 0;
int level = 0;
unsigned long randomSeedValue;

int getRandomLight() {
  return lights[random(0, 4)];
}

void resetCopyGame() {
  repeat = false;
  sequenceLength = DEFAULT_SEQUENCE;
  sequenceDelay = DEFAULT_SEQUENCE_DELAY;
  level = 0;
  lastButtonPressed = -1;
  PORTD = B11111100;
  
  clearDisplay();
  print(modes[modesIndex]);
}

void breakableDelay(int delayTime) {
  prevMillis = millis();
  while (!(buttonPressed(RED_BUTTON_PIN) || buttonPressed(GREEN_BUTTON_PIN))) {
    currentMillis = millis();
    if (currentMillis - prevMillis >= delayTime)
    {
      break;
    }
  }
}

String getLight(int light) {
  switch (light) {
    case 1:
      return "YELLOW";
      break;
    case 2:
      return "BLUE";
      break;
    case 8:
      return "GREEN";
      break;
    case 16:
      return "RED";
      break;
  }
}

void playSequence() {
  // play the sequence
  for (i = 0; i < sequenceLength; i++) {
    if (buttonPressed(RED_BUTTON_PIN) || buttonPressed(GREEN_BUTTON_PIN)) {
      resetCopyGame();
      break;
    }

    switch (sequence[i])
    {
      case 1:
        PORTD = B11101100;
        break;
      case 2:
        PORTD = B11011100;
        break;
      case 8:
        PORTD = B10111100;
        break;
      case 16:
        PORTD = B01111100;
        break;
    }

    // breakableDelay(sequenceDelay);
    delay(1000);
  }

  // sequence has been built we can now go into play mode \o/
  repeat = true;
  PORTD = B11111100;
}

void checkUserSequence() {
  // compare sequence against user input
  i = 0;
  sequenceError = false;
  while (!sequenceError && i != sequenceLength) {
    if (buttonPressed(RED_BUTTON_PIN) || buttonPressed(GREEN_BUTTON_PIN)) {
      resetCopyGame();
      break;
    }

    pinc = PINC;
    if (pinc != lastButtonPressed && pinc != 0) {
      lastButtonPressed = pinc;
      PORTD = getRelatedLight(pinc);
      delay(buttonInterval);
      PORTD = B11111100;

      // we got a button value here yay
      if (pinc != sequence[i]) {
        sequenceError = true;

        PORTD = B11111000;
        delay(buzzerInterval);
        PORTD = B11111100;

        resetCopyGame();
      } else {
        i++;
      }
    }
  }

  if (!sequenceError) {
    levelCopyUp();
  }

  repeat = false;
  lastButtonPressed = -1;
}

void levelCopyUp() {
  // check if sequenceLength is within array bounds
  if ((level % LEVEL_STEP) == 0) {
    if (sequenceLength < MAX_SEQUENCE) {
      sequenceLength++;
    }
  }

  // check if sequenceDelay is not below 0
  if (sequenceDelay > MIN_DELAY + SEQUENCE_STEP) {
    sequenceDelay -= SEQUENCE_STEP;
  }

  level++;

  clearDisplay();
  print("L" + (String)level);
}

void generateSequence() {
  randomSeed(millis());

  // build light sequence
  for (i = 0; i < sequenceLength; i++) {
    randomLight = getRandomLight();
    if (previousRandomLight != -1) {
      while (randomLight == previousRandomLight) {
        randomLight = getRandomLight();
      }
    }

    sequence[i] = randomLight;
    previousRandomLight = randomLight;
  }
}

byte getRelatedLight(int pin) {
  switch (pin)
  {
    case 1:
      return B11101100;
      break;
    case 2:
      return B11011100;
      break;
    case 8:
      return B10111100;
      break;
    case 16:
      return B01111100;
      break;
  }
}

void copy() {
  if (modes[modesIndex] != "COPY")
  {
    return;
  }

  if (!repeat) {
    delay(1000);
    generateSequence();
    playSequence();
  } else {
    checkUserSequence();
  }

  if (buttonPressed(RED_BUTTON_PIN) || buttonPressed(GREEN_BUTTON_PIN)) {
    resetCopyGame();
  }
}

void resetQuiz() {
  quizWinner = -1;
  PORTD = B11111100;
}

void quiz()
{
  if (modes[modesIndex] != "QUIZ")
  {
    return;
  }

  if (quizWinner == -1)
  {
    PORTD = B11111100;

    pinc = PINC;
    if (pinc == 1 || pinc == 2 || pinc == 8 || pinc == 16)
    {
      winner(pinc);
    }
  }

  if (buttonPressed(RED_BUTTON_PIN))
  {
    resetQuiz();
  }

}

void winner(int winner)
{
  quizWinner = winner;
  PORTD = getRelatedLight(winner);

  PORTD = PORTD ^ B00000100;
  delay(buzzerInterval);
  PORTD = PORTD ^ B00000100;
}

void disco()
{
  if (modes[modesIndex] != "DISC")
  {
    return;
  }
  PORTD = B00001100;
}

void horn()
{
  if (modes[modesIndex] != "HORN")
  {
    return;
  }

  pinc = PINC;
  while (buttonPressed(RED_BUTTON_PIN) || pinc != 0)
  {
    switch (pinc)
    {
      case 1:
        PORTD = B11101000;
        break;
      case 2:
        PORTD = B11011000;
        break;
      case 8:
        PORTD = B10111000;
        break;
      case 16:
        PORTD = B01111000;
        break;
      case 0:
        PORTD = B00001000;
        break;
    }
    pinc = PINC;
  }

  PORTD = B11111100;
}

void thirtySeconds()
{
  if (modes[modesIndex] != "30 S")
  {
    return;
  }

  pinc = PINC;
  if (pinc != 0)
  {
    PORTD = B11111100;
    runTimer = true;
    delay(buttonInterval);
  }

  if (runTimer)
  {
    currentMillis = millis();
    if (currentMillis - prevMillis >= timerInterval)
    {
      prevMillis = currentMillis;
      thirtysecondsCountDown();
    }
  }

  if (buttonPressed(RED_BUTTON_PIN))
  {
    resetThirtySeconds();
  }

}

void thirtysecondsCountDown()
{
  seconds = (int)timerCounter;
  centisecond = (timerCounter - seconds) * 100;

  printTime(seconds, centisecond);

  timerCounter = timerCounter - 0.01;
  if (timerCounter < 0)
  {
    resetThirtySeconds();
    thirtySecondsFinished();
  }
}

void resetThirtySeconds()
{
  prevMillis = 0;
  runTimer = false;
  timerCounter = timerInitValue;
  PORTD = B11111100;

  clearDisplay();
  print("30 S");
}

void thirtySecondsFinished()
{
  PORTD = B00001100;
  PORTD = PORTD ^ B00000100;
  delay(buzzerInterval);
  PORTD = PORTD ^ B00000100;
}

void resetGames() {
  resetThirtySeconds();
  resetQuiz();
  // resetCopyGame();
}

void cycleModes()
{
  if (buttonPressed(GREEN_BUTTON_PIN))
  {
    modesIndex++;
    if (modesIndex > maxModes)
    {
      modesIndex = 0;
    }
    
    resetGames();

    clearDisplay();
    print(modes[modesIndex]);
    delay(buttonInterval);
  }
}


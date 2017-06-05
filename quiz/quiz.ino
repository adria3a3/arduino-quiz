//#include <SevenSegmentFun.h>
// #include <SevenSegmentExtended.h>
// #include <SevenSegmentTM1637.h>

const byte PIN_CLK = 0;   // define CLK pin (any digital pin)
const byte PIN_DIO = 1;   // define DIO pin (any digital pin)
// SevenSegmentExtended    display(PIN_CLK, PIN_DIO);

int quizWinner = -1;
int modesIndex = 0; //quiz
String modes[] = {"COPY", "QUIZ", "DISC", "HORN", "30 S"};
int maxModes = 4;
double timerInitValue = 30;
double timerCounter = timerInitValue;
bool runTimer;
int buttonInterval = 300;
int buzzerInterval = 200;
unsigned long prev_millis = 0;
int timerInterval = 10;
int pinc = 0;
unsigned long current_millis = 0;
int seconds = 0;
int centisecond = 0;
const int RED_BUTTON_PIN = 8;
const int GREEN_BUTTON_PIN = 9; 

void setup()
{
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, OUTPUT);
  pinMode(A6, OUTPUT);
  pinMode(A7, OUTPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  
  DDRD = B11111111;
  PORTD = B11111100;
   
  Serial.begin(9600);
  //display.begin();            // initializes the display
  //display.setBacklight(100);  // set the brightness to 100 %
  //display.print(modes[modesIndex]);
}

void loop()
{
  copy();
  quiz();
  disco();
  horn();
  thirtySeconds();
  cycleModes();
}


// amount of lights (in a sequence)
// speed between lights (increased per run)
// if we got an error we sound the horn
// sequence should be unique (so no red, red)
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

void resetCopy() {
  repeat = false;
  sequenceLength = DEFAULT_SEQUENCE;
  sequenceDelay = DEFAULT_SEQUENCE_DELAY;
  level = 0; 
}

void playSequence() {
  // play the sequence
  for (i = 0; i < sequenceLength; i++) {
    // Serial.print("turning on light: ");
    // Serial.println(sequence[i]);
    switch(sequence[i])
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
    delay(sequenceDelay);
  }

  // sequence has been built we can now go into play mode \o/
  repeat = true;

  PORTD = B11111100;
}

void checkUserSequence() {
  // compare sequence against user input
  i = 0;
  while (sequenceError || i != sequenceLength) {
    pinc = PINC;
    delay(buttonInterval);
    if (pinc != 0) {
      PORTD = getRelatedLight(pinc);
      delay(USER_FEEDBACK_DELAY);
      PORTD = B11111100;
      Serial.println(pinc);
      // we got a button value here yay
      if (pinc != sequence[i]) {
        
        Serial.println("Pressed button ");
        Serial.println(pinc);

        Serial.println("Right button ");
        Serial.println(sequence[i]);

        sequenceError = true;

        PORTD = B11111000;
        delay(buzzerInterval);
        PORTD = B11111100;

        resetCopy();
      } else {
        Serial.println("Length is: ");
        Serial.println(sequenceLength);
        
        Serial.println("Current pos: ");
        Serial.println(i);
        i++;
      }
    }
  }

  Serial.println("wooohooo completed \o/");
  repeat = false;
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
}

void generateSequence() {
  randomSeed(millis());
  // build light sequence
  Serial.println("Generating random sequence");
  for (i = 0; i < sequenceLength; i++) {
    randomLight = getRandomLight();
    if (previousRandomLight != -1) {
      while (randomLight == previousRandomLight) {
        randomLight = getRandomLight();
      }
    }

    sequence[i] = randomLight;
    previousRandomLight = randomLight;
    Serial.println(randomLight);
  }
  Serial.println("Done generating random sequence");
}

byte getRelatedLight(int pin) {
  switch(pin)
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
  if(modes[modesIndex] != "COPY")
  {
    return;
  }
  
  if (!repeat) {
    delay(1000);
    generateSequence();
    playSequence();
  } else {
    checkUserSequence();
    levelCopyUp();
  }
}

void quiz()
{
  if(modes[modesIndex] != "QUIZ")
  {
    return;
  }
  if(quizWinner == -1)
  {
    PORTD = B11111100;
        
    pinc = PINC;
    if(pinc == 1 || pinc == 2 || pinc == 8 || pinc == 16)
    {
      winner(pinc);
    }
  }
  if(digitalRead(RED_BUTTON_PIN) == 1 || digitalRead(GREEN_BUTTON_PIN) == 1)
  {
    quizWinner = -1;
    PORTD = B11111100;
  }
  
}

void winner(int winner)
{
  quizWinner = winner;
  switch(winner)
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
  
  PORTD = PORTD ^ B00000100;
  delay(buzzerInterval);
  PORTD = PORTD ^ B00000100;
}

void disco()
{
  if(modes[modesIndex] != "DISC")
  {
    return;
  }
  PORTD = B00001100;
}

void horn()
{
  if(modes[modesIndex] != "HORN")
  {
    return;
  }
  
  pinc = PINC;
  while(digitalRead(RED_BUTTON_PIN) == 1 || pinc != 0)
  {
    switch(pinc)
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
  if(modes[modesIndex] != "30 S")
  {
    return;
  }

  while(true)
  {
    pinc = PINC;
    if(pinc != 0)
    {
      PORTD = B11111100;
      runTimer = !runTimer;
      delay(buttonInterval);
    }
    
    if(runTimer)
    {
      current_millis = millis();
      if(current_millis - prev_millis >= timerInterval)
      {
        prev_millis = current_millis;
        thirtysecondsCountDown();
      }
    }
    
    if(digitalRead(RED_BUTTON_PIN) == 1)
    {
      resetTimer();
    }

    if(digitalRead(GREEN_BUTTON_PIN) == 1)
    {
      resetTimer();
      break;
    }
  }
}

void thirtysecondsCountDown()
{
  seconds = (int)timerCounter;
  centisecond = (timerCounter - seconds)*100;
  //display.printTime(seconds,centisecond, false);
  
  timerCounter = timerCounter - 0.01;
  if(timerCounter < 0)
  {
    resetTimer();
    thirtySecondsFinished();
  }
}

void resetTimer()
{
    runTimer = false;
    timerCounter = timerInitValue;
    PORTD = B11111100;
    //display.clear();
    //display.print("30 S");
}

void thirtySecondsFinished()
{
  PORTD = B00001100;
  PORTD = PORTD ^ B00000100;
  delay(buzzerInterval);
  PORTD = PORTD ^ B00000100;
}

void cycleModes()
{
  if(digitalRead(GREEN_BUTTON_PIN) == 1)
  {
    delay(10);
    if(digitalRead(GREEN_BUTTON_PIN) != 1)
    {
      return;
    }
    modesIndex++;
    if(modesIndex > maxModes)
    {
      modesIndex = 0;
    }
    //display.clear();
    //display.print(modes[modesIndex]);
    delay(buttonInterval);
  }
}


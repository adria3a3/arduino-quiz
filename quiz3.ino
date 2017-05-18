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
unsigned long prev_millis = 0;
int timerInterval = 10;

unsigned long current_millis = 0;
int seconds = 0;
int centisecond = 0;

void setup()
{
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  
  DDRD = B11111111;
  PORTD = B11111100;
   
  display.begin();            // initializes the display
  display.setBacklight(100);  // set the brightness to 100 %
  display.print(modes[modesIndex]);
}

void loop()
{
  quiz();
  disco();
  horn();
  thirtySeconds();
  cycleModes();
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
        
    if(PINC == 1 || PINC == 2 || PINC == 8 || PINC == 16)
    {
      winner(PINC);
    }
  }
  if(digitalRead(8) == 1 || digitalRead(9) == 1)
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
  
  while(digitalRead(8) == 1 || PINC != 0)
  {
    switch(PINC)
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
    if(PINC != 0)
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
    if(digitalRead(8) == 1)
    {
      resetTimer();
    }
    if(digitalRead(9) == 1)
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
  display.printTime(seconds,centisecond, false);
  
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
    display.clear();
    display.print("30 S");
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
  if(digitalRead(9) == 1)
  {
    delay(10);
    if(digitalRead(9) != 1)
    {
      return;
    }
    modesIndex++;
    if(modesIndex > maxModes)
    {
      modesIndex = 0;
    }
    display.clear();
    display.print(modes[modesIndex]);
    delay(buttonInterval);
  }
}


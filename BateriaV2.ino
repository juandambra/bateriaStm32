#include <Arduino.h>

#include <MIDI.h>

// http : //www.robotoid.com/appnotes/sensors-piezo-disc-touch-bar.html

       MIDI_CREATE_DEFAULT_INSTANCE();
const int midichannel = 1;


//*******************************************************************************************************************
// Setup
//*******************************************************************************************************************

class Pad
{
  protected:
    int _pin;
    int _padCutOff;
    int _padNote;
    bool _velocity;
    bool _active;
    int _playtime;
    int _maxPlaytime;
    //bool _hiHat;
    Pad(int pin, int padCutOff, int padNote, bool velocity);
    void basicAct();
    void basicAct2();
  public:
    virtual void act() = 0;
};

Pad::Pad(int pin, int padCutOff, int padNote, bool velocity)
{
  _pin = pin;
  _padCutOff = padCutOff;
  _padNote = padNote;
  _velocity = velocity;
  _active = false;
  _playtime = 0;
  _maxPlaytime = 90;
}

void Pad::basicAct()
{
  int valor = analogRead(_pin);
  if (valor > _padCutOff)
  {
    if(!_active){
      valor = (valor/8) -1;
      MIDI.sendNoteOn(_padNote, valor, midichannel);
      _playtime = 0;
      _active = true;
    }
    else _playtime++;
  }
  else if(_active)
  {
    _playtime++;
    if(_playtime > _maxPlaytime)
    {
      _active = false;
    }
  }
}

// long map(long x, long in_min, long in_max, long out_min, long out_max)
// {
//   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
// }

void Pad::basicAct2()
{
  int newValue = 0;
  int lastValue = 0;   // value read from the pot
  int outputValue = 0; // value output to the PWM
  int count = 0;
  int maxCount = 0;
  newValue = analogRead(_pin);
  if (newValue > _padCutOff)
  {
    if (!_active)
    {
      lastValue = 0;
      count = 0;
      while (count < 60)
      {
        if (newValue > lastValue)
        {
          lastValue = newValue;
          maxCount = count;
        }
        newValue = analogRead(_pin);
        // Serial.print(newValue);
        // Serial.print(",");
        count++;
      }
      //Serial.print("sensor max value = " );
      // Serial.print("\n");
      // Serial.print("Valor Maximo:");
      // Serial.print(lastValue);
      // Serial.print(", en sameple numero :");
      // Serial.println(maxCount);
      // delay(40 + lastValue * 3 / 100);
    
      // valor = (valor / 8) - 1;
      int valor = map(lastValue, 0, 4000, 0, 127);
      MIDI.sendNoteOn(_padNote, valor, midichannel);
      _playtime = 0;
      _active = true;
    }
    else
      _playtime++;
  }
  else if (_active)
  {
    _playtime++;
    if (_playtime > _maxPlaytime)
    {
      _active = false;
    }
  }
}

class AnaloguePad : public Pad
{
  public:
    AnaloguePad(int pin, int padCutOff, int padNote, bool velocity):
    Pad(pin, padCutOff, padNote, velocity)
    {};
    void act();
};

class HiHatPad : public Pad
{
  private:
    bool _hiHat;
    void changeNote(int note);
  public:
    HiHatPad(int pin, int padCutOff, int padNote, bool velocity):
    Pad(pin, padCutOff, padNote, velocity),
    _hiHat(true){};
    void act();
};

void HiHatPad::changeNote(int note)
{
  _padNote = note;
}

void HiHatPad::act()
{
  if(digitalRead(2))
  {
    if (!_hiHat)
    {
      _hiHat = true;
      changeNote(36);
    }
  } else
  {   
    if (_hiHat)
    {
      _hiHat = false;
      MIDI.sendNoteOn(38, 127, midichannel);
      MIDI.sendNoteOn(36, 0, midichannel);
      changeNote(37);
    }
  }
  basicAct2();
}

void AnaloguePad::act()
{
  basicAct2();
}

//HiHatPad pad1(0,500,36,true);
//AnaloguePad pad2(1, 500, 39, true);
//AnaloguePad pad3(2, 500, 40, true);
//AnaloguePad pad4(3, 500, 41, true);
//AnaloguePad pad5(4, 500, 42, true);
//AnaloguePad pad6(5, 500, 43, true);
Pad** arrayPads ; 
int maxPads;
void setup()
{
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  MIDI.begin();
  Serial.begin(115200);                                  // connect to the serial port 115200
  arrayPads = new Pad*[6];
  maxPads = 4;
  arrayPads[0] = new AnaloguePad(PA0, 550, 38, true);        // new HiHatPad(0, 550, 36, true);
  arrayPads[1] = new AnaloguePad(PA1, 550, 39, true);
  arrayPads[2] = new AnaloguePad(PA2, 550, 40, true);
  arrayPads[3] = new AnaloguePad(PA3, 550, 41, true);
  //arrayPads[4] = new AnaloguePad(4, 550, 42, true);
  //arrayPads[5] = new AnaloguePad(5, 550, 43, true);
}

//*******************************************************************************************************************
// Main Program
//*******************************************************************************************************************

void loop()
{
  for (int pin = 0; pin < maxPads; pin++)
    arrayPads[pin]->act();
}


//r variable
//position of loops
const double r_c = 0.12000, r_o = 0.699000;
double r_out, psi_c, psi_o, psi_op, tangent;

//calibration coefficients
const double c_op = -0.0504334;
const double c_c = -0.000919975;

//z variable
#define dim_z 7

double psi[dim_z];
double z_out;
double A, B;

//calibration coefficients
const double c_o[2] = {0.00934779, 0.00952129};
const double c_so[6] = {-0.0116987, -0.0137233, -0.0116096, -0.00803445, 0.0120827, 0.0135117};

//coefficients of fitting (X_T*X)^-1*X_T X=z
const double c[3][7] = {
    {-0.11208726, 0.11096086, 0.30037245, 0.38624873, 0.29337451, 0.14411194, -0.12298114},
    {0.78498737, 0.61369047, 0.37636976, 0.035726801, -0.33225523, -0.58251984, -0.89599924},
    {4.1828246, 0.48285854, -2.6519288, -4.0549021, -2.4798069, 0.024828931, 4.4961242}};

//pin mapping
#define PF 2 //R
#define H1 4 //Z1
#define H2 5 //Z2
#define H3 6 //Z3
#define H4 3 //Z0
#define triggerPin 20

#define D_PF OCR3B
#define D_H2 OCR3A
#define D_H4 OCR3C

// default duty circle 0~255
#define PF_default 255
#define H2_default 255
#define H4_default 255

// target
#define r_t 512;
#define z_t 512;

double error_r;
double error_z;
double cumuError_r = 0;
double cumuError_z = 0;
double lastError_r = 0;
double lastError_z = 0;
double lastDuty = 0;
double duty_z;

//PID coefficient
double Kr_p = 0.01; //50
double Kr_i = 0.01;
//double Kr_d = 0.1;

double Kz_p = 0.01; //50
double Kz_i = 0.01;
//double Kz_d = 0.1;

// preprogrammed waveform
#define pre_num 6
int pre[pre_num] = {1, 1, 1, 1, 1, 1};

const int delta_time = 100;
const int delayfromTrigger = 10000;
bool triggerflag = false;
unsigned long triggerTime;
unsigned long TimefromTrigger;
int cnt = 0;

void triggerISR()
{
  triggerflag = true;
  triggerTime = micros();
}

void setup()
{

  pinMode(PF, OUTPUT);
  pinMode(H1, OUTPUT);
  pinMode(H2, OUTPUT);
  pinMode(H3, OUTPUT);
  pinMode(H4, OUTPUT);
  pinMode(triggerPin, INPUT);

  Serial3.begin(9600);

  // clock = clk_io = 16MHz(default), fast PWM mode, set at match and clear at bottom
  // T = 16 microseconds
  TCCR3A = _BV(COM3A1) | _BV(COM3A0) | _BV(COM3B1) | _BV(COM3B0) | _BV(COM3C0) | _BV(COM3C1) | _BV(WGM30);
  TCCR3B = _BV(WGM32) | _BV(CS30);

  D_PF = PF_default;
  D_H2 = H2_default;
  D_H4 = H4_default;

  //change adc clock, prescaler 16, ADC not working at 4 or 2
  bitClear(ADCSRA, ADPS0);
  bitClear(ADCSRA, ADPS1);
  bitSet(ADCSRA, ADPS2);

  //trigger interrupt
  attachInterrupt(digitalPinToInterrupt(triggerPin), triggerISR, RISING);

  digitalWrite(PF, LOW);
  digitalWrite(H1, LOW);
  digitalWrite(H2, LOW);
  digitalWrite(H3, LOW);
  digitalWrite(H4, LOW);
}

void loop()
{

  if (triggerflag)
  {

    //  /* PWM programmed waveform mode, turn on PWM first, not verified yet */
    //    TimefromTrigger = micros()-triggerTime;
    //    if(TimefromTrigger<2000){
    //      delayMicroseconds(delayfromTrigger-TimefromTrigger-delta_time);
    //      bitSet(TCCR3A,COM3B1);
    //      for (int i = 0; i < pre_num; i++)
    //      {
    //          D_PF = pre[i];
    //          delayMicroseconds(100);
    //      }
    //    }

    /*
     * read psi for z_out
     * arduino read from 0~5V but signal from 2.5 to -2.5V
     * psi[0] are flux loop
     * psi[1] to psi[6] are saddle loop
     */

    TimefromTrigger = micros() - triggerTime;

    if (TimefromTrigger < 2000)
    {
      delayMicroseconds(delayfromTrigger - TimefromTrigger - delta_time);
      for (int i = 0; i < pre_num / 2; i++)
      {
        digitalWrite(PF, HIGH);
        delay(pre[2 * i]);
        digitalWrite(PF, LOW);
        delay(pre[2 * i + 1]);

        // turn on pwm for R control
        bitSet(TCCR3A, COM3B1);
      }
    }

    else
    {
      for (int i = 1; i < dim_z; i++)
      {
        //analogRead optimized from 120us to 5us
        psi[i] = analogRead(A1 + i);
        psi[i] = -(psi[i] * (5.0 / 1023.0)) + 2.5;
      }

      psi[0] = analogRead(A7);
      psi[0] = -(psi[0] * (5.0 / 1023.0)) + 2.5;
      psi[0] = psi[0] * c_o[0];

      for (int i = 1; i < dim_z; i++)
      {
        psi[i] = psi[i] * c_so[i - 1];
        psi[i] = psi[i] + psi[i - 1];
      }

      //read psi_c,psi_o,tangent for r_out
      psi_c = analogRead(A9);
      psi_c = -(psi_c * (5.0 / 1023.0)) + 2.5;
      psi_c = psi_c * c_c;
      psi_o = psi[3]; //3 is outboard center loop
      psi_op = analogRead(A8);
      psi_op = -(psi_op * (5.0 / 1023.0)) + 2.5;
      psi_op = psi_op * c_op;
      tangent = psi_op * PI * 2 * r_o;

      //rout
      r_out = (r_c + r_o + (psi_c - psi_o) / tangent) / 2;

      //zout
      A = 0, B = 0;
      for (int i = 0; i < dim_z; i++)
      {
        A += psi[i] * c[2][i];
        B += psi[i] * c[1][i];
      }

      z_out = -B / (2 * A);

      /* check r_out */
      //    r_out=analogRead(A1);
      error_r = r_out - r_t;
      cumuError_r += error_r;
      //    lastError_r = error_r;
      D_PF += Kr_p * error_r + Kr_i * cumuError_r;
      D_PF = 100;
      delay(2);

      /* check z_out */
      //    z_out=analogRead(A1);
      //    z_out = z_out*(5.0/1023.0);
      //    Serial.println(z_out);
      error_z = z_out - z_t;
      cumuError_z += error_z;
      duty_z = lastDuty + Kz_p * error_z + Kz_i * cumuError_z;
      lastDuty = duty_z;
      if (duty_z > 255)
      {
        duty_z = 255;
      }
      if (duty_z < -255)
      {
        duty_z = -255;
      }

      if (duty_z > 0)
      {
        // 255 is no signal at 0V, 0 is delta signal from 5V
        D_H2 = 255;
        //some delay to make sure the order is correct, because the period of pwm is 16us
        digitalWrite(H3, LOW);
        delayMicroseconds(16);
        digitalWrite(H1, HIGH);
        D_H4 = 255 - duty_z;
      }
      else
      {
        D_H4 = 255;
        digitalWrite(H1, LOW);
        delayMicroseconds(16);
        digitalWrite(H3, HIGH);
        D_H2 = 255 + duty_z;
      }
    }

    /* stop after 100 loop */
    cnt++;
    if (cnt > 100)
    {
      cnt = 0;
      triggerflag = false;
      digitalWrite(PF, LOW);
      digitalWrite(H1, LOW);
      digitalWrite(H2, LOW);
      digitalWrite(H3, LOW);
      digitalWrite(H4, LOW);
    }
  }

  else
  {
    if (Serial3.available())
    {
      for (int i = 0; i < pre_num; i++)
      {
        pre[i] = Serial3.parseInt();
      }
    }
  }
}

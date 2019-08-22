#define dim_z 7
//position of loops
const double r_c = 0.12000, r_o = 0.699000;
double r_out, psi_c, psi_o, psi_op, tangent;
double tf = 0;
double psi[dim_z];
double z_out;
double A, B;

//calibration coefficients
double c_op = -0.0504334;
double c_c = -0.000919975;
double c_o = 0.00934779;
double c_so[6] = {-0.0116987, -0.0137233, -0.0116096, -0.00803445, 0.0120827, 0.0135117};
double c_bt = 0.083102499;

//coefficients of fitting (X_T*X)^-1*X_T X=z
const double c[3][7] = {
    {-0.11208726, 0.11096086, 0.30037245, 0.38624873, 0.29337451, 0.14411194, -0.12298114},
    {0.78498737, 0.61369047, 0.37636976, 0.035726801, -0.33225523, -0.58251984, -0.89599924},
    {4.1828246, 0.48285854, -2.6519288, -4.0549021, -2.4798069, 0.024828931, 4.4961242}};

double tfoff_so[6] = {0.000378215, -0.000524829, -0.00155499, -0.000648422, 0.000982490, 0.00128668};
double tfoff_c = -0.000129345;
double tfoff_op = 0.00415151;

//pin mapping
const int triggerPin = 20;
const int PF = 2; //R
const int H1 = 4; //Z1
const int H2 = 5; //Z2
const int H3 = 6; //Z3
const int H4 = 3; //Z0

#define D_PF OCR3B
#define D_H2 OCR3A
#define D_H4 OCR3C

// default duty circle
int PF_default = 0;
const int H2_default = 0;
const int H4_default = 0;

// target
double r_t = 0;
double z_t = 0;

double error_r;
double error_z;
double cumuError_r = 0;
double cumuError_z = 0;
double lastError_r = 0;
double lastError_z = 0;
int lastDuty_z = 0;
int lastDuty_r = 0;
int duty_z;
int duty_r;

//PID coefficient
double Kr_p = 0.01; //50
double Kr_i = 0.01;
//double Kr_d = 0.1;

double Kz_p = 0.01; //50
double Kz_i = 0.01;
//double Kz_d = 0.1;

// preprogrammed waveform
volatile byte pre_num_r = 0;
int *pre_r = new int(1);
int pre_num_z = 0;
byte num_z_state = 0;
int *pre_z = new int(1);
byte cnt_z = 0;
int *pre_z_state = new int(1);

volatile byte cnt_r = 0;
unsigned long thisTime = 0;
unsigned long lastTime_z = 0;
int preTime_z = 0;
volatile byte state_r = HIGH;
byte state_z = HIGH;

//trigger
const int delta_time = 20;
int delayfromTrigger = 10000;
volatile unsigned long triggerTime;
unsigned long TimefromTrigger;
int cnt = 0;
double val = 0;
char command;

//data sent back
double checkZOut[100];
double checkROut[100];
int checkDutyZ[100];
int checkPF[100];
double checkProbe[10][100];
double checktime[100];

//offset coefficient
double A_offset, B_offset;
double psi_offset, tangent_offset;
double tf_selfoff;
double tf_offset;

const int avg_num = 5;
double A_offsets[avg_num];
double B_offsets[avg_num];
double psi_offsets[avg_num];
double tangent_offsets[avg_num];
double tf_selfoffs[avg_num];
int cnt_offset = 0;

//flag
volatile bool triggerflag = false;
volatile bool initFlag = true;
volatile bool pwmFlag = true;

bool isTFdivided = true;
double c_tfdivide = 2;
double op_limit = 0.0005;

double avg(int num, double *x)
{
  double sum = 0;
  for (int i = 0; i < num; i++)
  {
    sum += x[i];
  }
  return sum / num;
}

void triggerISR()
{
  triggerTime = micros();
  triggerflag = true;
  initFlag = true;
  pwmFlag = true;
}

ISR(TIMER5_COMPA_vect)
{
  cnt_r++;
  if (cnt_r >= pre_num_r)
  {
    bitClear(TIMSK5, OCIE5A);
    digitalWrite(PF, LOW);
  }
  else
  {
    OCR5A = 2 * pre_r[cnt_r];
    digitalWrite(PF, state_r);
    state_r = 1 - state_r;
  }
}

void setup()
{
  noInterrupts();

  pinMode(PF, OUTPUT);
  pinMode(H1, OUTPUT);
  pinMode(H2, OUTPUT);
  pinMode(H3, OUTPUT);
  pinMode(H4, OUTPUT);
  pinMode(triggerPin, INPUT);

  Serial3.begin(9600);

  //timer3 generate pwm for feedback control
  // clock = clk_io = 16MHz(default), fast PWM mode, set at match and clear at bottom, prescarlar 8
  // T = 128 microseconds
  TCCR3A = _BV(COM3A1) | _BV(COM3A0) | _BV(COM3B1) | _BV(COM3B0) | _BV(COM3C0) | _BV(COM3C1) | _BV(WGM30);
  TCCR3B = _BV(WGM32) | _BV(CS31);

  D_PF = 255;
  D_H2 = 255;
  D_H4 = 255;

  //timer5 interrupt to control PF preprogrammed waveform
  bitClear(TIMSK5, OCIE5A);
  TCCR5A = 0;
  TCCR5B = _BV(WGM52) | _BV(CS51);

  //change adc clock to get faster sampling, prescaler 16, ADC not working at 4 or 2
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

  interrupts();
}

void loop()
{

  if (triggerflag)
  {
    if (initFlag)
    {
      //some initiation
      TimefromTrigger = micros() - triggerTime;
      delayMicroseconds(delayfromTrigger - TimefromTrigger - delta_time);

      if (pre_num_r > 0)
      {
        digitalWrite(PF, state_r);
        state_r = 1 - state_r;
        OCR5A = 2 * pre_r[0];
        TCNT5 = 0;
        bitSet(TIFR5, OCF5A);
        bitSet(TIMSK5, OCIE5A);
      }

      lastTime_z = micros();
      preTime_z = 0;

      initFlag = false;
    }

    //polling to control H preprogrammed waveform
    else if (cnt_r < pre_num_r)
    {
      if (cnt_z < pre_num_z)
      {
        thisTime = micros() - lastTime_z;
        if (thisTime > preTime_z)
        {
          switch (pre_z_state[cnt_z])
          {
          case 1:
            digitalWrite(H1, HIGH);
            digitalWrite(H2, LOW);
            digitalWrite(H3, LOW);
            digitalWrite(H4, HIGH);
            break;
          case 2:
            digitalWrite(H1, HIGH);
            digitalWrite(H2, LOW);
            digitalWrite(H3, LOW);
            digitalWrite(H4, LOW);
            break;
          case 3:
            digitalWrite(H1, LOW);
            digitalWrite(H2, HIGH);
            digitalWrite(H3, HIGH);
            digitalWrite(H4, LOW);
            break;
          case 4:
            digitalWrite(H1, LOW);
            digitalWrite(H2, LOW);
            digitalWrite(H3, HIGH);
            digitalWrite(H4, LOW);
            break;
          case 5:
            digitalWrite(H1, LOW);
            digitalWrite(H2, LOW);
            digitalWrite(H3, LOW);
            digitalWrite(H4, LOW);
            break;
          default:
            break;
          }
          preTime_z += pre_z[cnt_z];
          cnt_z++;
        }
      }
    }

    //turn on pwm before feedback begin
    else if (pwmFlag)
    {
      bitSet(TCCR3A, COM3A1);
      bitSet(TCCR3A, COM3B1);
      bitSet(TCCR3A, COM3C1);
      D_PF = 255 - PF_default;
      D_H2 = 255 - H2_default;
      D_H4 = 255 - H4_default;
      pwmFlag = false;
    }

    //feedback control
    else
    {
      /*
     * read psi for z_out
     * arduino read from 0~5V but signal from 2.5 to -2.5V
     * psi[0] are flux loop
     * psi[1] to psi[6] are saddle loop
     */

      tf = analogRead(A10);
      tf = -(tf * (5.0 / 1023.0)) + 2.5;
      if (isTFdivided)
      {
        tf = tf * c_tfdivide;
      }
      tf = c_bt * tf - tf_selfoff;
      checkProbe[9][cnt] = tf;

      //so
      for (int i = 1; i < dim_z; i++)
      {
        psi[i] = analogRead(A1 + i - 1);
        psi[i] = -(psi[i] * (5.0 / 1023.0)) + 2.5;
      }

      //O0
      psi[0] = analogRead(A7);
      psi[0] = -(psi[0] * (5.0 / 1023.0)) + 2.5;
      psi[0] = psi[0] * c_o - tf * tfoff_c;
      checkProbe[0][cnt] = psi[0];

      for (int i = 1; i < dim_z; i++)
      {
        psi[i] = psi[i] * c_so[i - 1] - tf * tfoff_so[i - 1];
        checkProbe[i][cnt] = psi[i];
        psi[i] = psi[i] + psi[i - 1];
      }

      //read C0 and Bp
      psi_c = analogRead(A9);
      psi_c = -(psi_c * (5.0 / 1023.0)) + 2.5;
      psi_c = psi_c * c_c - tf * tfoff_c;
      checkProbe[7][cnt] = psi_c;
      psi_o = psi[3]; //3 is outboard center loop
      psi_op = analogRead(A8);
      psi_op = -(psi_op * (5.0 / 1023.0)) + 2.5;
      psi_op = psi_op * c_op - tf * tfoff_op;
      checkProbe[8][cnt] = psi_op;
      tangent = psi_op * PI * 2 * r_o;

      //rout
      r_out = (r_c + r_o + (psi_c - psi_o - psi_offset) / (tangent - tangent_offset)) / 2;

      //zout
      A = 0, B = 0;
      for (int i = 0; i < dim_z; i++)
      {
        A += psi[i] * c[2][i];
        B += psi[i] * c[1][i];
      }

      z_out = -(B - B_offset) / (2 * (A - A_offset));

      /* check r_out */
      error_r = r_out - r_t;
      cumuError_r += error_r;
      duty_r = int(Kr_p * error_r + Kr_i * cumuError_r + PF_default);
      if (duty_r > 255)
      {
        duty_r = 255;
      }
      if (duty_r < 0)
      {
        duty_r = 0;
      }
      D_PF = 255 - duty_r;
      lastDuty_r = duty_r;

      /* check z_out */
      error_z = z_out - z_t;
      cumuError_z += error_z;
      duty_z = int(Kz_p * error_z + Kz_i * cumuError_z);
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
        if (lastDuty_z < 0)
        {
          // 255 is no signal at 0V, 0 is delta signal from 5V
          D_H2 = 255;
          digitalWrite(H3, LOW);
          // delay to make sure that there is no short happening
          delayMicroseconds(128);
        }
        digitalWrite(H1, HIGH);
        D_H4 = 255 - duty_z;
      }
      else
      {
        if (lastDuty_z > 0)
        {
          D_H4 = 255;
          digitalWrite(H1, LOW);
          delayMicroseconds(128);
        }
        digitalWrite(H3, HIGH);
        D_H2 = 255 + duty_z;
      }

      lastDuty_z = duty_z;
      /* stop after 100 loop */
      checkROut[cnt] = r_out;
      checkPF[cnt] = D_PF;
      checkZOut[cnt] = z_out;
      checkDutyZ[cnt] = duty_z;
      checktime[cnt] = micros() - triggerTime;

      cnt++;

      if (cnt > 99 || psi_op < op_limit)
      {
        cnt_r = 0;
        cnt_z = 0;
        duty_z = 0;
        duty_r = 0;
        lastDuty_z = 0;
        lastDuty_r = 0;
        cumuError_z = 0;
        cumuError_r = 0;
        state_r = HIGH;
        state_z = HIGH;
        triggerflag = false;
        thisTime = 0;
        lastTime_z = 0;
        D_PF = 255;
        D_H2 = 255;
        D_H4 = 255;
        digitalWrite(PF, LOW);
        digitalWrite(H1, LOW);
        digitalWrite(H2, LOW);
        digitalWrite(H3, LOW);
        digitalWrite(H4, LOW);

        Serial3.println("cnt");
        Serial3.println(cnt);
        Serial3.println("ROut");
        for (int i = 0; i < cnt; i++)
        {
          Serial3.println(checkROut[i], 10);
        }
        Serial3.println("D_PF");
        for (int i = 0; i < cnt; i++)
        {
          Serial3.println(checkPF[i]);
        }
        Serial3.println("ZOut");
        for (int i = 0; i < cnt; i++)
        {
          Serial3.println(checkZOut[i], 10);
        }
        Serial3.println("duty_z");
        for (int i = 0; i < cnt; i++)
        {
          Serial3.println(checkDutyZ[i]);
        }
        Serial3.println("probe");
        for (int i = 0; i < 10; i++)
        {
          Serial3.print("p");
          Serial3.println(i);
          for (int j = 0; j < cnt; j++)
          {
            Serial3.println(checkProbe[i][j], 10);
          }
        }
        Serial3.println("time");
        for (int i = 0; i < cnt; i++)
        {
          Serial3.println(checktime[i]);
        }
        cnt = 0;
      }
    }
  }

  else
  {
    tf = analogRead(A10);
    tf = -(tf * (5.0 / 1023.0)) + 2.5;
    if (isTFdivided)
    {
      tf = tf * c_tfdivide;
    }
    tf = c_bt * tf;
    tf_selfoffs[cnt_offset] = tf;

    for (int i = 1; i < dim_z; i++)
    {
      //analogRead optimized from 120us to 5us
      psi[i] = analogRead(A1 + i - 1);
      psi[i] = -(psi[i] * (5.0 / 1023.0)) + 2.5;
    }

    psi[0] = analogRead(A7);
    psi[0] = -(psi[0] * (5.0 / 1023.0)) + 2.5;
    psi[0] = psi[0] * c_o;

    for (int i = 1; i < dim_z; i++)
    {
      psi[i] = psi[i] * c_so[i - 1];
      psi[i] = psi[i] + psi[i - 1];
    }

    psi_c = analogRead(A9);
    psi_c = -(psi_c * (5.0 / 1023.0)) + 2.5;
    psi_c = psi_c * c_c;
    psi_o = psi[3];
    psi_op = analogRead(A8);
    psi_op = -(psi_op * (5.0 / 1023.0)) + 2.5;
    psi_op = psi_op * c_op;
    tangent_offsets[cnt_offset] = psi_op * PI * 2 * r_o;
    psi_offsets[cnt_offset] = psi_c - psi_o;

    A = 0, B = 0;
    for (int i = 0; i < dim_z; i++)
    {
      A += psi[i] * c[2][i];
      B += psi[i] * c[1][i];
    }
    A_offsets[cnt_offset] = A;
    B_offsets[cnt_offset] = B;

    cnt_offset++;
    if (cnt_offset == avg_num)
    {
      cnt_offset = 0;
    }

    tf_selfoff = avg(avg_num, tf_selfoffs);
    psi_offset = avg(avg_num, psi_offsets);
    tangent_offset = avg(avg_num, tangent_offsets);
    A_offset = avg(avg_num, A_offsets);
    B_offset = avg(avg_num, B_offsets);

    if (Serial3.available())
    {
      command = Serial3.read();
      switch (command)
      {
      case 'r': //r target
        val = Serial3.parseFloat();
        r_t = val;
        Serial3.println("r target set!");
        Serial3.println(r_t);
        break;
      case 'z': //z target
        val = Serial3.parseFloat();
        z_t = val;
        Serial3.println("z target set!");
        Serial3.println(z_t);
        break;
      case 's': //delay to start preprogrammed wave
        val = Serial3.parseInt();
        delayfromTrigger = val;
        Serial3.println("delay set!");
        Serial3.println(delayfromTrigger);
      case 'f': //default_pf
        val = Serial3.parseInt();
        PF_default = val;
        Serial3.println("pf default set!");
        Serial3.println(PF_default);
        break;
      case 'p': //Kz_p
        val = Serial3.parseFloat();
        Kz_p = val;
        Serial3.println("Kz_p set!");
        Serial3.println(Kz_p);
        break;
      case 'i': //Kz_i
        val = Serial3.parseFloat();
        Kz_i = val;
        Serial3.println("Kz_i set!");
        Serial3.println(Kz_i);
        break;
      case 'x': //Kr_p
        val = Serial3.parseFloat();
        Kr_p = val;
        Serial3.println("Kr_p set!");
        Serial3.println(Kr_p);
        break;
      case 'y': //Kr_i
        val = Serial3.parseFloat();
        Kr_i = val;
        Serial3.println("Kr_i set!");
        Serial3.println(Kr_i);
        break;
      case 'm': //PF preprogrammed wave
        val = Serial3.parseInt();
        pre_num_r = val;
        delete[] pre_r;
        pre_r = new int[pre_num_r];
        Serial3.println("PF preprogrammed wave set!");
        for (int i = 0; i < pre_num_r; i++)
        {
          val = Serial3.parseInt();
          pre_r[i] = int(val);
          Serial3.println(pre_r[i]);
        }
        break;
      case 'n': //H preprogrammed wave
        val = Serial3.parseInt();
        pre_num_z = val;
        delete[] pre_z;
        pre_z = new int[pre_num_z];
        Serial3.println("H preprogrammed wave set!");
        for (int i = 0; i < pre_num_z; i++)
        {
          val = Serial3.parseInt();
          pre_z[i] = int(val);
          Serial3.println(pre_z[i]);
        }
        break;
      case 'l': //H preprogrammed wave state
        delete[] pre_z_state;
        pre_z_state = new int[pre_num_z];
        for (int i = 0; i < pre_num_z; i++)
        {
          Serial3.println("H wave state set!");
          val = Serial3.parseInt();
          pre_z_state[i] = int(val);
          Serial3.println(pre_z_state[i]);
        }
        break;
      case 'o': //op limitation
        val = Serial3.parseFloat();
        op_limit = val;
        Serial3.println("op limit target set!");
        Serial3.println(op_limit, 10);
        break;
      case 't': //op limitation
        val = Serial3.parseInt();
        if (val == 1)
        {
          isTFdivided = true;
        }
        else if (val == 2)
        {
          isTFdivided = false;
        }
        Serial3.println("TF mode set!");
        Serial3.println(isTFdivided);
        break;
      case 'A':
        val = Serial3.parseFloat();
        c_op = val;
        Serial3.println("c_op set!");
        Serial3.println(c_op);
        break;
      case 'B':
        val = Serial3.parseFloat();
        c_c = val;
        Serial3.println("c_c set!");
        Serial3.println(c_c);
        break;
      case 'C':
        val = Serial3.parseFloat();
        c_o = val;
        Serial3.println("c_o set!");
        Serial3.println(c_o);
        break;
      case 'D': //r target
        Serial3.println("c_so set!");
        for (int i = 0; i < 6; i++)
        {
          val = Serial3.parseFloat();
          c_so[i] = val;
          Serial3.println(c_so[i]);
        }
        break;
      case 'D':
        val = Serial3.parseFloat();
        c_bt = val;
        Serial3.println("c_bt set!");
        Serial3.println(c_bt);
        break;
      case 'E': //r target
        Serial3.println("tfoff_so set!");
        for (int i = 0; i < 6; i++)
        {
          val = Serial3.parseFloat();
          tfoff_so[i] = val;
          Serial3.println(tfoff_so[i]);
        }
        break;
      case 'F':
        val = Serial3.parseFloat();
        tfoff_c = val;
        Serial3.println("tfoff_c set!");
        Serial3.println(tfoff_c);
        break;
      case 'G':
        val = Serial3.parseFloat();
        tfoff_op = val;
        Serial3.println("tfoff_op set!");
        Serial3.println(tfoff_op);
        break;
      default:
        break;
      }
    }
  }
}

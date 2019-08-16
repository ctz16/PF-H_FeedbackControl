#define dim_z 7
//position of loops
const double r_c = 0.12000, r_o = 0.699000;
double r_out, psi_c, psi_o, psi_op, tangent;
double psi[dim_z];
double z_out;
double A, B;

//calibration coefficients
const double c_op = -0.0504334;
const double c_c = -0.000919975;
const double c_o[2] = {0.00934779, 0.00952129};
const double c_so[6] = {-0.0116987, -0.0137233, -0.0116096, -0.00803445, 0.0120827, 0.0135117};
const double c_bt = 0.083102499;

//coefficients of fitting (X_T*X)^-1*X_T X=z
const double c[3][7] = {
    {-0.11208726, 0.11096086, 0.30037245, 0.38624873, 0.29337451, 0.14411194, -0.12298114},
    {0.78498737, 0.61369047, 0.37636976, 0.035726801, -0.33225523, -0.58251984, -0.89599924},
    {4.1828246, 0.48285854, -2.6519288, -4.0549021, -2.4798069, 0.024828931, 4.4961242}};

const double tfoff_so[6] = {0.000378215, -0.000524829,  -0.00155499, -0.000648422,  0.000982490,   0.00128668};
const double tfoff_c[2] = { -0.000129345, -0.000101060};
const double tfoff_op = 0.00415151;

//pin mapping
const int triggerPin = 20;
const int PF = 2; //R
const int H1 = 4;       //Z1
const int H2 = 5;       //Z2
const int H3 = 6;       //Z3
const int H4 = 3;       //Z0

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
int pre_num_r = 0;
int *pre_r = new int(1);
int pre_num_z = 0;
int *pre_z = new int(1);
int cnt_z = 0;
int cnt_r = 0;
int next_r = 0;
int next_z = 0;
unsigned long thisTime;
unsigned long lastTime_r;
unsigned long lastTime_z;
int state_r = HIGH;
int state_z = HIGH;

//trigger
const int delta_time = 100;
int delayfromTrigger = 10000;
bool triggerflag = false;
unsigned long triggerTime;
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

//offset coefficient
double A_offset1,B_offset1;
double psi_offset1,tangent_offset1;
double tf_selfoff;
double tf_offset;
double c_psi_o=0;
double c_tan_o=0;
double c_a_o=0;
double c_b_o=0;
double tf=0;

//H brigde mode
//mode 1 is normal mode, H1H4 is positive
//mode 2 is emergency mode, H2H3 is positive
//in mode 2, H1 change with H2, H3 change with H4
int mode = 1;

void triggerISR()
{
  triggerflag = true;
  triggerTime = micros();
}

ISR(TIM4_COMPA_vect){
  cnt_r++;
  OCR4A = 2*pre_r[cnt_r];
  digitalWrite(PF,state_r);
  state_r=1-state_r;
  if (cnt_r >= pre_num_r-1){
    bitClear(TIMSK4,OCIE4A);
  }
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

  // clock = clk_io = 16MHz(default), fast PWM mode, set at match and clear at bottom, prescarlar 8
  // T = 128 microseconds
  TCCR3A = _BV(COM3A1) | _BV(COM3A0) | _BV(COM3B1) | _BV(COM3B0) | _BV(COM3C0) | _BV(COM3C1) | _BV(WGM30);
  TCCR3B = _BV(WGM32) | _BV(CS31);

  D_PF = 255;
  D_H2 = 255;
  D_H4 = 255;

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

  TCCR4B = _BV(WGM42) | _BV(CS41);


}

void loop()
{

  if (triggerflag)
  {

    TimefromTrigger = micros() - triggerTime;

    if (TimefromTrigger < 5000)
    {
      delayMicroseconds(delayfromTrigger - TimefromTrigger - delta_time);
      OCR4A = 2*pre_r[0];
      bitSet(TIMSK4,OCIE4A);
      //  for (int i = 0; i < pre_num_r / 2; i++)
      //  {
      //    digitalWrite(PF, HIGH);
      //    delayMicroseconds(pre_r[2 * i]);
      //    digitalWrite(PF, LOW);
      //    delayMicroseconds(pre_r[2 * i + 1]);
      //  }

      //  //turn on pwm
      //  bitSet(TCCR3A, COM3A1);
      //  bitSet(TCCR3A, COM3B1);
      //  bitSet(TCCR3A, COM3C1);
      //  D_PF = 255 - PF_default;
      //  D_H2 = 255 - H2_default;
      //  D_H4 = 255 - H4_default;
    }

    else
    {
     if (cnt_r < pre_num_r)
     {
      //  thisTime = micros();
      //  if (thisTime - lastTime_r > pre_r[cnt_r])
      //  {
      //    // high is 1, low is 0
      //    digitalWrite(PF, state_r);
      //    state_r = 1-state_r;
      //    lastTime_r = micros();
      //    cnt_r++;
      //  }

       thisTime = micros();
       if(pre_z[cnt_z]>0)
       {
         if (thisTime - lastTime_z > pre_z[cnt_z])
         {
           digitalWrite(H2,LOW);
           digitalWrite(H3,LOW);
           digitalWrite(H1, state_z);
           digitalWrite(H4, state_z);
           state_z = 1-state_z;
           lastTime_z = micros();
           cnt_z++;
         }
       }
       else
       {
         if (thisTime - lastTime_z > -pre_z[cnt_z])
         {
           digitalWrite(H1,LOW);
           digitalWrite(H4,LOW);
           digitalWrite(H2, (state_z);
           digitalWrite(H3, (state_z);
           state_z = 1-state_z;
           lastTime_z = micros();
           cnt_z++;
         }
       }
     }
     else
     {
       bitSet(TCCR3A, COM3A1);
       bitSet(TCCR3A, COM3B1);
       bitSet(TCCR3A, COM3C1);
       D_PF = 255 - PF_default;
       D_H2 = 255 - H2_default;
       D_H4 = 255 - H4_default;
     }
     
     if (psi_op>0)
      {

    /*
     * read psi for z_out
     * arduino read from 0~5V but signal from 2.5 to -2.5V
     * psi[0] are flux loop
     * psi[1] to psi[6] are saddle loop
     */

        tf = analogRead(A10);
        tf = -(tf * (5.0 / 1023.0)) + 2.5;
        tf = c_bt*tf - tf_selfoff;
        checkProbe[9][cnt] = tf;

        //so
        for (int i = 1; i < dim_z; i++)
        {
          //analogRead optimized from 120us to 5us
          psi[i] = analogRead(A1 + i - 1);
          psi[i] = -(psi[i] * (5.0 / 1023.0)) + 2.5;
        }

        //O0
        psi[0] = analogRead(A7);
        psi[0] = -(psi[0] * (5.0 / 1023.0)) + 2.5;
        psi[0] = psi[0] * c_o[0] - tf*tfoff_c[0];
        checkProbe[0][cnt] = psi[0];

        for (int i = 1; i < dim_z; i++)
        {
          psi[i] = psi[i] * c_so[i - 1]-tf*tfoff_so[i-1];
          checkProbe[i][cnt] = psi[i];
          psi[i] = psi[i] + psi[i - 1];
        }

        //read C0 and Bp
        psi_c = analogRead(A9);
        psi_c = -(psi_c * (5.0 / 1023.0)) + 2.5;
        psi_c = psi_c * c_c - tf*tfoff_c[0];
        checkProbe[7][cnt] = psi_c;
        psi_o = psi[3]; //3 is outboard center loop
        psi_op = analogRead(A8);
        psi_op = -(psi_op * (5.0 / 1023.0)) + 2.5;
        psi_op = psi_op * c_op - tf*tfoff_op;
        checkProbe[8][cnt] = psi_op;
        tangent = psi_op * PI * 2 * r_o;

        //rout
        r_out = (r_c + r_o + (psi_c - psi_o - psi_offset1) / (tangent-tangent_offset1)) / 2;

        //zout
        A = 0, B = 0;
        for (int i = 0; i < dim_z; i++)
        {
          A += psi[i] * c[2][i];
          B += psi[i] * c[1][i];
        }

        z_out = -(B - B_offset1) / (2 * (A-A_offset1));

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
      }

      /* stop after 100 loop */
      checkROut[cnt] = r_out;
      checkPF[cnt] = D_PF;
      checkZOut[cnt] = z_out;
      checkDutyZ[cnt] = duty_z;

      cnt++;

      if (cnt > 99)
      {
        cnt = 0;
        cnt_r = 0;
        cnt_z = 0;
        duty_z = 0;
        duty_r = 0;
        lastDuty_z = 0;
        lastDuty_r = 0;
        cumuError_z = 0;
        cumuError_r = 0;
        triggerflag = false;
        D_PF = 255;
        D_H2 = 255;
        D_H4 = 255;
        digitalWrite(PF, LOW);
        digitalWrite(H1, LOW);
        digitalWrite(H2, LOW);
        digitalWrite(H3, LOW);
        digitalWrite(H4, LOW);

        Serial3.println("ROut");
        for (int i = 0; i < 100; i++)
        {
          Serial3.println(checkROut[i], 10);
        }
        Serial3.println("D_PF");
        for (int i = 0; i < 100; i++)
        {
          Serial3.println(checkPF[i]);
        }
        Serial3.println("ZOut");
        for (int i = 0; i < 100; i++)
        {
          Serial3.println(checkZOut[i], 10);
        }
        Serial3.println("duty_z");
        for (int i = 0; i < 100; i++)
        {
          Serial3.println(checkDutyZ[i]);
        }
        Serial3.println("probe");
        for (int i = 0; i < 10; i++)
        {
          Serial3.print("p");
          Serial3.println(i);
          for (int j = 0; j < 100; j++)
          {
            Serial3.println(checkProbe[i][j], 10);
          }
        }
      }
    }
  }

  else
  {
    tf = analogRead(A10);
    tf = -(tf * (5.0 / 1023.0)) + 2.5;
    tf = c_bt*tf;
    tf_selfoff = tf;

    for (int i = 1; i < dim_z; i++)
        {
          //analogRead optimized from 120us to 5us
          psi[i] = analogRead(A1 + i - 1);
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

        psi_c = analogRead(A9);
        psi_c = -(psi_c * (5.0 / 1023.0)) + 2.5;
        psi_c = psi_c * c_c;
        psi_o = psi[3]; 
        psi_op = analogRead(A8);
        psi_op = -(psi_op * (5.0 / 1023.0)) + 2.5;
        psi_op = psi_op * c_op;
        tangent_offset1 = psi_op * PI * 2 * r_o;
        psi_offset1 = psi_c-psi_o;

        A = 0, B = 0;
        for (int i = 0; i < dim_z; i++)
        {
          A += psi[i] * c[2][i];
          B += psi[i] * c[1][i];
        }
        A_offset1=A;
        B_offset1=B;


        

    if (Serial3.available())
    {
      command = Serial3.read();
      switch (command)
      {
      case 'r': //r target
        val = Serial3.parseFloat();
        if (val > 0)
        {
          r_t = val;
          Serial3.println("r target set!");
          Serial3.println(r_t);
        }
        break;
      case 'z': //z target
        val = Serial3.parseFloat();
        if (val > 0.000001 || val < -0.000001)
        {
          z_t = val;
          Serial3.println("z target set!");
          Serial3.println(z_t);
        }
        break;
      case 's': //delay to start preprogrammed wave
        val = Serial3.parseInt();
        if (val > 0)
        {
          delayfromTrigger = val;
          Serial3.println("delay set!");
          Serial3.println(delayfromTrigger);
        }
        break;
      // case 'a': //z offset coefficient
      //   val = Serial3.parseFloat();
      //   if (val > 0.000001 || val < -0.000001)
      //   {
      //     c_a_o = val;
      //     Serial3.println("A offset coefficient set!");
      //     Serial3.println(c_a_o,5);
      //   }
      //   break;
      // case 'b': //psi offset coefficient
      //   val = Serial3.parseFloat();
      //   if (val > 0.000001 || val < -0.000001)
      //   {
      //     c_b_o = val;
      //     Serial3.println("B offset coefficient set!");
      //     Serial3.println(c_b_o,5);
      //   }
      //   break;
      // case 'c': //tangent offset coefficient
      //   val = Serial3.parseFloat();
      //   if (val > 0.000001 || val < -0.000001)
      //   {
      //     c_psi_o = val;
      //     Serial3.println("psi offset coefficient set!");
      //     Serial3.println(c_psi_o,5);
      //   }
      //   break;
      // case 'd': //tangent offset coefficient
      //   val = Serial3.parseFloat();
      //   if (val > 0.000001 || val < -0.000001)
      //   {
      //     c_tan_o = val;
      //     Serial3.println("tangent offset coefficient set!");
      //     Serial3.println(c_tan_o,5);
      //   }
      //   break;
      case 'f': //default_pf
        val = Serial3.parseInt();
        if (val > 0)
        {
          PF_default = val;
          Serial3.println("pf default set!");
          Serial3.println(PF_default);
        }
        break;
      case 'p': //Kz_p
        val = Serial3.parseFloat();
        if (val > 0.000001 || val < -0.000001)
        {
          Kz_p = val;
          Serial3.println("Kz_p set!");
          Serial3.println(Kz_p);
        }
        break;
      case 'i': //Kz_i
        val = Serial3.parseFloat();
        if (val > 0.0000001 || val < -0.0000001)
        {
          Kz_i = val;
          Serial3.println("Kz_i set!");
          Serial3.println(Kz_i);
        }
        break;
      case 'x': //Kr_p
        val = Serial3.parseFloat();
        if (val > 0)
        {
          Kr_p = val;
          Serial3.println("Kr_p set!");
          Serial3.println(Kr_p);
        }
        break;
      case 'y': //Kr_i
        val = Serial3.parseFloat();
        if (val > 0)
        {
          Kr_i = val;
          Serial3.println("Kr_i set!");
          Serial3.println(Kr_i);
        }
        break;
      case 'm': //PF preprogrammed wave
        val = Serial3.parseInt();
        if (val > 0)
        {
          pre_num_r = val;
          delete[] pre_r;
          pre_r = new int[pre_num_r];
        }
        for (int i = 0; i < pre_num_r; i++)
        {
          val = Serial3.parseInt();
          if (val > 0)
          {
            pre_r[i] = int(val);
          }
          Serial3.println(pre_r[i]);
        }
        break;
      case 'n': //H preprogrammed wave
        val = Serial3.parseInt();
        if (val > 0)
        {
          pre_num_z = val;
          delete[] pre_z;
          pre_z = new int[pre_num_z];
        }
        for (int i = 0; i < pre_num_z; i++)
        {
          val = Serial3.parseInt();
          if (val > 0.000001 || val < -0.000001)
          {
            pre_z[i] = int(val);
          }
          Serial3.println(pre_z[i]);
        }
        break;
      default:
        break;
      }
    }
  }
}

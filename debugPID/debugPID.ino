//r variable
//position of loops
const double r_c=0.12000, r_o=0.699000;
double r_out,psi_c,psi_o,psi_op,tangent;

//calibration coefficients
const double c_op=-0.0504334;
const double c_c=-0.000919975;

//z variable
#define dim_z 8

double psi[dim_z];
double z_out;
double A,B;

//calibration coefficients
const double c_o[2]={   0.00934779,   0.00952129};
const double c_so[6]={   -0.0116987,   -0.0137233,   -0.0116096,  -0.00803445,    0.0120827,    0.0135117};

//coefficients of fitting (X_T*X)^-1*X_T X=z
const double c[3][8]={
{     -0.10157325,      0.10741101,      0.28746355,      0.37565219,      0.30099177,     0.17140523,    -0.064258453,    -0.077091960},
{      0.85464802,      0.59017087,      0.29084170,    -0.034480695,     -0.28178700,     -0.40168786,     -0.50693151,     -0.51077351},
{       3.8203913,      0.60522721,      -2.2069399,      -3.6896237,      -2.7423851,     -0.91601104,       2.4718668,       2.6574733}};

//pin mapping
#define PF 2 //R
#define H1 4 //Z1
#define H2 5 //Z2
#define H3 6 //Z3
#define H4 3 //Z0

#define D_PF OCR3B
#define D_H2 OCR3A
#define D_H4 OCR3C

// default duty circle 0~255
#define PF_default 100
#define H2_default 100
#define H4_default 100

// timePeriod measured
#define timePeriod 0.83 //ms

// target
#define r_t 0;
#define z_t 0;
double error_r;
double error_z;
double cumuError_r = 0;
double cumuError_z = 0;
double lastError_r = 0;
double lastError_z = 0;
double lastDuty = 0;

//PID coefficient
#define Kr_p 0.2
#define Kr_i 0.1
#define Kr_d 0.1

#define Kz_p 0.2
#define Kz_i 0.1
#define Kz_d 0.1

void setup(){

    pinMode(PF,OUTPUT);
    pinMode(H1,OUTPUT);
    pinMode(H2,OUTPUT);
    pinMode(H3,OUTPUT);
    pinMode(H4,OUTPUT);
    pinMode(11,OUTPUT);

    // clock = clk_io = 16MHz(default), fast PWM mode, set at match and clear at bottom
    // T = 16 microseconds
    TCCR3A = _BV(COM3A1) | _BV(COM3A0) | _BV(COM3B1) | _BV(COM3B0) | _BV(COM3C0) | _BV(COM3C1) | _BV(WGM30);
    TCCR3B = _BV(WGM32) | _BV(CS30);
//    OCR3AL = 255;
//    OCR3BL = 255;
//    OCR3CL = 255;

    D_PF = PF_default;
    D_H2 = H2_default;
    D_H4 = H4_default;

    ADCSRA = _BV(ADPS2);
//    Serial.begin(230400);
}

void loop(){
//    digitalWrite(11,HIGH);
//    for (int i=0; i<10; i++){
//      digitalWrite(11,HIGH);
//    }
//    for (int i=0; i<10; i++){
//      psi[0]=analogRead(A0);
//    }
    //read psi for z_out
    //arduino read from 0~5V but signal from 2.5 to -2.5V
    //psi[0] and psi[7] are flux loop
    //psi[1] to psi[6] are saddle loop
    for (int i = 0; i < dim_z; i++)
    {
        psi[i]=analogRead(A1+i);
        psi[i]=-(psi[i] * (5.0 / 1023.0))+2.5;
    }
    psi[0]=psi[0]*c_o[0];
    psi[dim_z-1]=psi[dim_z-1]*c_o[1];
    
    for (int i = 1; i < dim_z-1; i++)
    {
        psi[i]=psi[i]*c_so[i-1];
        psi[i]=psi[i]+psi[i-1];
    }
    
    //read psi_c,psi_o,tangent for r_out
    
    psi_c=analogRead(A9);
    psi_c=-(psi_c * (5.0 / 1023.0))+2.5;
    psi_c=psi_c*c_c;
    psi_o=psi[3]; //3 is outboard center loop
    psi_op=analogRead(A10);
    psi_op=-(psi_op * (5.0 / 1023.0))+2.5;
    psi_op=psi_op*c_op;
    tangent=psi_op*PI*2*r_o;
    
    
    //rout
    r_out = (r_c+r_o+(psi_c-psi_o)/tangent)/2;

    //zout
    A=0,B=0;
    for (int i = 0; i < dim_z; i++)
    {
        A+=psi[i]*c[2][i];
        B+=psi[i]*c[1][i];
    }
    
    z_out=-B/(2*A);
    
    //check r_out
//    r_out=analogRead(A1);
    error_r = r_out - r_t;
    cumuError_r += error_r;
    D_PF += Kr_p*error_r + Kr_i*cumuError_r + Kr_d*(error_r-lastError_r);
    lastError_r = error_r;
    
    //check z_out
//    z_out=analogRead(A1);
    error_z = z_out - z_t;
    cumuError_z += error_z;
    lastError_z = error_z;
    if(error_z>0){
        digitalWrite(H3,LOW);
        D_H2 = 255;
        digitalWrite(H1,HIGH);
        D_H4 = lastDuty + Kz_p*error_z + Kz_i*cumuError_z + Kz_d*(error_z-lastError_z);
        lastDuty = D_H4;
    }
    else{
        digitalWrite(H1,LOW);
        D_H4 = 255;
        digitalWrite(H3,HIGH);
        D_H2 = lastDuty + Kz_p*error_z + Kz_i*cumuError_z + Kz_d*(error_z-lastError_z);
        lastDuty = D_H2;
    }
    
//    digitalWrite(11,LOW);
}

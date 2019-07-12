//PID coefficient
#define prop 0.2

//pin map
//OCR2A:23, OCR2B:18 
#define PF 2
#define H1 4
#define H2 5
#define H3 6
#define H4 3

#define D_PF OCR3B
#define D_H2 OCR3A
#define D_H4 OCR3C

// default duty circle 0~255
#define PF_default 100
#define H2_default 100
#define H4_default 100

const double r_t=512;
const double z_t=512;
double delta_d;

double r_out;
double z_out;

void setup(){

    pinMode(PF,OUTPUT);
    pinMode(H1,OUTPUT);
    pinMode(H2,OUTPUT);
    pinMode(H3,OUTPUT);
    pinMode(H4,OUTPUT);

    // clock = clk_io = 16MHz(default), fast PWM mode, set at match and clear at bottom
    // T = 16 microseconds
    TCCR3A = _BV(COM3A1) |_BV(COM3A0) |_BV(COM3B1) |_BV(COM3B0) | _BV(COM3C0) | _BV(COM3C1) | _BV(WGM30);
    TCCR3B = _BV(WGM32)| _BV(CS30);
    OCR3AL = 255;
    OCR3BL = 255;
    OCR3CL = 255;

//    Serial.begin(230400);
}
void loop(){
//    bitClear(PORTB,4);
    //check r_out
    r_out=analogRead(A1);
    D_PF = PF_default + prop*(r_out-r_t);
//    Serial.print("rout ");
//    Serial.print(r_out);
//    Serial.print("V: ");
//    Serial.println(r_out*5.0/1023.0);
//    Serial.print("D_PF ");
//    Serial.println(D_PF);
    
    //check z_out
    z_out=analogRead(A1);
    delta_d=prop*(z_out-z_t);
    if(delta_d>0){
        digitalWrite(H3,LOW);
        D_H2=255;
        digitalWrite(H1,HIGH);
        D_H4 = H4_default + delta_d;
    }
    else{
        digitalWrite(H1,LOW);
        D_H4=255;
        digitalWrite(H3,HIGH);
        D_H2 = H2_default + delta_d;
    }
    bitSet(PORTB,4);
}

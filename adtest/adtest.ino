void setup() {
  //change adc clock, prescaler 16, ADC not working at 4 or 2
  bitClear(ADCSRA, ADPS0);
  bitClear(ADCSRA, ADPS1);
  bitSet(ADCSRA, ADPS2);
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int value = analogRead(A9);
  double voltage = value * (5.0 / 1023.0);
  Serial.println(voltage);
}

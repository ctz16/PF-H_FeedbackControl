void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int value=analogRead(A1);
  float voltage = value * (5.0 / 1023.0);
  Serial.println(voltage);
}

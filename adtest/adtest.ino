void setup() {
  Serial.begin(9600);
}

void loop() {
  double tf = analogRead(A10);
  tf = -(tf * (5.0 / 1023.0)) + 2.5;
  Serial.println(tf,7);
}

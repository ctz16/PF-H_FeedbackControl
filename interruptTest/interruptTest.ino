const int PF =2;

ISR(TIMER4_COMPA_vect)
{
  cnt_r++;
  OCR4A = 2 * pre_r[cnt_r];
  digitalWrite(PF, state_r);
  state_r = 1 - state_r;
  if (cnt_r >= pre_num_r - 1)
  {
    cnt_r++;
    bitClear(TIMSK4, OCIE4A);
  }
}

void setup(){
    noInterrupts();
    bitClear(TIMSK4, OCIE4A);
  TCCR4A = 0;
  TCCR4B = _BV(WGM42) | _BV(CS41);

  interrupts();
}

void loop() {
  
}
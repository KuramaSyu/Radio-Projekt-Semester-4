int PIN_ANALOG_IN = 19;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  int x_value = analogRead(PIN_ANALOG_IN);
  delay(100);
  float channel = get_channel(x_value);
  Serial.print("Frequency: ");
  Serial.print(channel, 2);   // 2 decimal places
  Serial.println(" MHz");
}

float get_frequency(int analog_value) {
  const float f_min = 88.0;   // MHz
  const float f_max = 108.0;  // MHz

  return f_min + (analog_value / 4095.0) * (f_max - f_min);
}

float get_channel(int analog_value) {
  float frequencies[] = {89.2, 100.2, 102.4, 103.5, 105.2, 106.1};
  int len = sizeof(frequencies) / sizeof(frequencies[0]);
  float stepsize = 4096 / len;
  // stepsize * idx = analog_value -> analog_value / stepsize = x
  int index = (int)(analog_value / stepsize);
  float channel = frequencies[index];
  return channel;

}
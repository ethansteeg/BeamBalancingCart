void setup() {
  Serial.begin(9600);
  Serial.println("GoalRPM:,RPM:");//,V_out:");

  attachInterrupt(digitalPinToInterrupt(2), myFunction, FALLING);
  pinMode(3, INPUT);

  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);

  digitalWrite(6, LOW);
  analogWrite(5, 0);
}

volatile long int counter = 0;
int v_out = 80;
double error = 0;
double goalRPM = 0;
int activepin = 5;
int groundpin = 6;
double vmax = 0.551;
double amax = 0.2;//0.8175;
double radius = 0.0444;
double multiplier = 0.25;

double dist = 1.65;

double wmax = (vmax/radius)/(2*3.1416)*60;

double deltat = (vmax/amax)+(dist/vmax);
double deltatc = (-vmax/amax)+(dist/vmax);;

double tslope = 0.5*(deltat-deltatc)*1000000.;
double tflat = deltatc*1000000.;

int choice = 0; //0 means controlling amax; 1 means controlling vmax

if(choice == 0){
  if(tflat < 0){
    tflat = 0;

    vmax = sqrt(amax * dist);
  }
}
else{
  amax = vmax * vmax / dist;
}

bool firstTime = true;
bool bypass = false;

void loop() {
  unsigned long currentTime = micros();

  //NEED FOR SPEED
  if(currentTime < tslope){
    goalRPM = (wmax/tslope)*currentTime;
  }
  else if(currentTime < tslope + tflat){
    goalRPM = wmax;
  }
  else if(currentTime < 2*tslope + tflat){
    goalRPM = -(wmax/tslope)*(currentTime-(2*tslope + tflat));
  }
  else if(currentTime < 3*tslope + tflat){
    //Serial.println("switch1");
    goalRPM = -(wmax/tslope)*(currentTime-(2*tslope + tflat));
    activepin = 6;
    groundpin = 5;

    if(firstTime){
      bypass = true;
      firstTime = false;
    }
  }
  else if(currentTime < 3*tslope + 2*tflat){
    //Serial.println("switch2");
    goalRPM = -wmax;
  }
  else if(currentTime < 4*tslope + 2*tflat){
    //Serial.println("switch3");
    goalRPM = (wmax/tslope)*(currentTime-(4*tslope + 2*tflat));
  }
  else{
    goalRPM = 0;
    v_out = 0;
  }

  error = multiplier*(goalRPM - rpm());

  Serial.print(goalRPM);Serial.print(",");Serial.println(rpm());

  if(bypass){
    v_out = -80;
    bypass = false;
  }
  else if (v_out+error > 255) {
    v_out = 255;
  }
  else if (v_out+error < -255) {
    v_out = -255;
  }
  else{
    v_out = v_out+error;
  }

  digitalWrite(groundpin,LOW);
  analogWrite(activepin, abs(v_out));
}

void myFunction() {
  if (digitalRead(3) == LOW) {
    counter++;
  }
  else {
    counter--;
  }
}

double angle() {
  return counter * 1.5;
}

double rpm(){
  double th0 = 0, th1 = 0;
  th0 = angle();
  delay(50);
  th1 = angle();
  return((th1-th0)/0.05/360*60);
}
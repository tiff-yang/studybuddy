#include "MPU6050.h"

MPU6050 mpu;

void setup(){
  // Setup connection on port 9600
  Serial.begin(9600);
  Serial.println("STARTING UP");

 // MPU
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_16G))
  {
    delay(500);
  }
  _configureMPU();
}


void loop(){
    Serial.write(mpu.readActivites().isActivity);
    delay(200);
}

void _configureMPU(){
  mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);

  mpu.setIntFreeFallEnabled(false);  
  mpu.setIntZeroMotionEnabled(false);
  mpu.setIntMotionEnabled(false);
  
  mpu.setDHPFMode(MPU6050_DHPF_5HZ);

  mpu.setMotionDetectionThreshold(2);
  mpu.setMotionDetectionDuration(5);

  mpu.setZeroMotionDetectionThreshold(4);
  mpu.setZeroMotionDetectionDuration(2); 
}

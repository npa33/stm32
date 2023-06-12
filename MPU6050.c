#include "MPU6050.h"
#include "I2C.h"
#include "LCD_I2C.h"
#include <stdio.h>

extern LCD_Name LCD1;

int16_t Accel_Z_RAW = 0;

#define WINDOW_SIZE 5
#define threshold 1.05
uint16_t stepCount = 0;
uint8_t thresholdCount = 0;
double curAccelZ = 0, preAccelZ = 0;
double dataAccelZ[WINDOW_SIZE] = {};
uint8_t currentIndex = 0, preIndex = WINDOW_SIZE - 1;

void MPU_Write (uint8_t Address, uint8_t Reg, uint8_t Data)
{
	I2C_Start ();
	I2C_Address (Address);
	I2C_Write (Reg);
	I2C_Write (Data);
	I2C_Stop ();
}

void MPU_Read (uint8_t Address, uint8_t Reg, uint8_t *buffer, uint8_t size)
{
	I2C_Start ();
	I2C_Address (Address);
	I2C_Write (Reg);
	I2C_Start ();  // Repeated start
	I2C_Read (Address+0x01, buffer, size);
	I2C_Stop ();
}

void MPU6050_Init (void)
{
	uint8_t check;
	uint8_t data;

	// Check device
	MPU_Read (MPU6050_ADDR, WHO_AM_I_REG, &check, 1);

	if (check == 104)  // 0x68 will be returned by the sensor if everything goes well
	{
		// Power management register 0X6B we should write all 0's to wake the sensor up
		data = 0;
		MPU_Write (MPU6050_ADDR, PWR_MGMT_1_REG, data);

		// Set DATA RATE of 1KHz by writing SMPLRT_DIV register
		data = 0x07;
		MPU_Write(MPU6050_ADDR, SMPLRT_DIV_REG, data);

		// Set accelerometer configuration in ACCEL_CONFIG Register
		// XA_ST=0,YA_ST=0,ZA_ST=0, FS_SEL=0 -> ± 2g
		data = 0x00;
		MPU_Write(MPU6050_ADDR, ACCEL_CONFIG_REG, data);
	}

}

double MPU6050_Read_AccelZ(void)
{
	uint8_t Recceived_Data[2];

	// Read 2 BYTES of data starting from ACCEL_ZOUT_H register
	MPU_Read (MPU6050_ADDR, ACCEL_ZOUT_H_REG, Recceived_Data, 2);

	Accel_Z_RAW = (int16_t)(Recceived_Data[0] << 8 | Recceived_Data [1]);

	return Accel_Z_RAW/16384.0; // Chuyen doi tu RAW value thanh gia tri gia toc(g): (Accel_Z_RAW / Sensitivity)
}

// Ham dem buoc chan
int MPU6050_Counter(void) 
{
	char buf[4];
	curAccelZ = MPU6050_Read_AccelZ();
	
	dataAccelZ[currentIndex] = curAccelZ;
	currentIndex = (currentIndex + 1) % WINDOW_SIZE;
	
	double filterAccelZ = 0;
  for(int i = 0; i < WINDOW_SIZE; i++) {
     filterAccelZ += dataAccelZ[i];
  }
  filterAccelZ /= WINDOW_SIZE;
	
	preAccelZ = dataAccelZ[preIndex];
	if(filterAccelZ > threshold && curAccelZ > preAccelZ && filterAccelZ < preAccelZ) {
		thresholdCount++;
	}
	
	if(thresholdCount > 1) {
		stepCount++; 							//Dem buoc chan
		
		//In so buoc chan ra man hinh
		sprintf(buf, "%d", stepCount);
		LCD_Cursor(&LCD1,6,1);
		LCD_WriteString(&LCD1, buf);
		
		thresholdCount = 0;
	}
	
	if(currentIndex == 0) {
		preIndex = WINDOW_SIZE - 1;
	} else {
		preIndex = currentIndex - 1;
	}
	
	return stepCount;
}

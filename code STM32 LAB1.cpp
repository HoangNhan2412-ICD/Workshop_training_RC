/* USER CODE BEGIN PFP */

void Motor_SetSpeed(int16_t speed);
void Servo_SetAngle(uint8_t angle);
void RC_Drive(int16_t speed, uint8_t angle);
void RC_Lab1_AutoTest(void);

/* USER CODE END PFP */
/* USER CODE BEGIN 0 */

void Motor_SetSpeed(int16_t speed)
{
  if (speed > 255)
  {
    speed = 255;
  }

  if (speed < -255)
  {
    speed = -255;
  }

  if (speed > 0)
  {
    // Tien
    HAL_GPIO_WritePin(GPIOA, IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, IN2_Pin, GPIO_PIN_RESET);
  }
  else if (speed < 0)
  {
    // Lui
    HAL_GPIO_WritePin(GPIOA, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, IN2_Pin, GPIO_PIN_SET);

    speed = -speed;
  }
  else
  {
    // Dung
    HAL_GPIO_WritePin(GPIOA, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, IN2_Pin, GPIO_PIN_RESET);
  }

  // TIM3 ARR = 999, map speed 0-255 sang CCR 0-999
  uint32_t ccr = (uint32_t)speed * 999 / 255;

  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ccr);
}

void Servo_SetAngle(uint8_t angle)
{
  if (angle > 180)
  {
    angle = 180;
  }

  // TIM4 servo 50Hz
  // CCR 1000 = 1ms
  // CCR 2000 = 2ms
  uint32_t ccr = 1000 + (uint32_t)angle * 1000 / 180;

  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, ccr);
}

void RC_Drive(int16_t speed, uint8_t angle)
{
  Motor_SetSpeed(speed);
  Servo_SetAngle(angle);
}

void RC_Lab1_AutoTest(void)
{
  // Tien 2 giay
  RC_Drive(200, 90);
  HAL_Delay(2000);

  // Re phai
  Servo_SetAngle(135);
  HAL_Delay(1000);

  // Lui 1 giay
  RC_Drive(-200, 90);
  HAL_Delay(1000);

  // Dung
  RC_Drive(0, 90);
  HAL_Delay(1000);

  // Toc do 30%
  RC_Drive(77, 90);
  HAL_Delay(2000);

  // Toc do 60%
  RC_Drive(153, 90);
  HAL_Delay(2000);

  // Toc do 100%
  RC_Drive(255, 90);
  HAL_Delay(2000);

  // Dung truoc khi lap lai
  RC_Drive(0, 90);
  HAL_Delay(3000);
}

/* USER CODE END 0 */

/* USER CODE BEGIN 2 */

HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);

Motor_SetSpeed(0);
Servo_SetAngle(90);

/* USER CODE END 2 */
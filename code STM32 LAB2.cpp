/* USER CODE BEGIN PV */

uint8_t rf_rx_byte;
uint8_t rf_buf[5];
uint8_t rf_idx = 0;

uint32_t last_cmd_time = 0;
uint8_t watchdog_stopped = 0;

uint8_t current_flags = 0;

/* USER CODE END PV */
/* USER CODE BEGIN PFP */

void Motor_SetSpeed(int16_t speed);
void Servo_SetAngle(uint8_t angle);
void RF_ParseByte(uint8_t byte);
void RF_ProcessFrame(void);
void Check_Watchdog(void);

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
    HAL_GPIO_WritePin(GPIOA, IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, IN2_Pin, GPIO_PIN_RESET);
  }
  else if (speed < 0)
  {
    HAL_GPIO_WritePin(GPIOA, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, IN2_Pin, GPIO_PIN_SET);
    speed = -speed;
  }
  else
  {
    HAL_GPIO_WritePin(GPIOA, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, IN2_Pin, GPIO_PIN_RESET);
  }

  uint32_t ccr = (uint32_t)speed * 999 / 255;
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ccr);
}

void Servo_SetAngle(uint8_t angle)
{
  if (angle > 180)
  {
    angle = 180;
  }

  uint32_t ccr = 1000 + (uint32_t)angle * 1000 / 180;
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, ccr);
}

void RF_ProcessFrame(void)
{
  uint8_t cs = rf_buf[0] ^ rf_buf[1] ^ rf_buf[2] ^ rf_buf[3];

  if (cs != rf_buf[4])
  {
    return;
  }

  int16_t speed = (int16_t)rf_buf[1] - 128;
  uint8_t angle = rf_buf[2];
  current_flags = rf_buf[3];

  speed = speed * 2;

  if (speed > 255)
  {
    speed = 255;
  }

  if (speed < -255)
  {
    speed = -255;
  }

  Motor_SetSpeed(speed);
  Servo_SetAngle(angle);

  last_cmd_time = HAL_GetTick();
  watchdog_stopped = 0;
}

void RF_ParseByte(uint8_t byte)
{
  if (rf_idx == 0 && byte != 0xAA)
  {
    return;
  }

  rf_buf[rf_idx++] = byte;

  if (rf_idx >= 5)
  {
    rf_idx = 0;
    RF_ProcessFrame();
  }
}

void Check_Watchdog(void)
{
  if ((HAL_GetTick() - last_cmd_time > 500) && watchdog_stopped == 0)
  {
    Motor_SetSpeed(0);
    watchdog_stopped = 1;
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    RF_ParseByte(rf_rx_byte);

    HAL_UART_Receive_IT(&huart2, &rf_rx_byte, 1);
  }
}

/* USER CODE END 0 */
/* USER CODE END 0 */


/* USER CODE BEGIN 2 */

HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);

Motor_SetSpeed(0);
Servo_SetAngle(90);

last_cmd_time = HAL_GetTick();

HAL_UART_Receive_IT(&huart2, &rf_rx_byte, 1);

/* USER CODE END 2 */
/* USER CODE END 2 */

while (1)
{
  Check_Watchdog();
}

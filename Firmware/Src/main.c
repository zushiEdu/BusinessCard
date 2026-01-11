#include "main.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

#define ROWS 12
#define COLS 12

//		{0,0,0,0,0,0,0,0,0,0,0,0},
//		{0,0,0,0,0,0,0,0,0,0,0,0},
//		{0,0,0,0,0,0,0,0,0,0,0,0},
//		{0,0,0,0,0,0,0,0,0,0,0,0},
//		{0,0,0,0,0,0,0,0,0,0,0,0},
//		{0,0,0,0,0,0,0,0,0,0,0,0},
//		{0,0,0,0,0,0,0,0,0,0,0,0},
//		{0,0,0,0,0,0,0,0,0,0,0,0},
//		{0,0,0,0,0,0,0,0,0,0,0,0},
//		{0,0,0,0,0,0,0,0,0,0,0,0},
//		{0,0,0,0,0,0,0,0,0,0,0,0},
//		{0,0,0,0,0,0,0,0,0,0,0,0}

uint8_t initialScreen[ROWS][COLS] =
{
	    {0,0,0,0,0,0,0,0,0,0,0,0},
	    {0,0,0,0,0,0,1,0,0,0,0,0},
	    {0,0,0,0,0,0,1,0,0,0,0,0},
	    {0,0,0,0,0,1,1,0,0,0,0,0},
	    {0,0,0,0,1,0,0,0,0,0,0,0},
	    {0,0,0,1,0,1,0,0,1,1,1,0},
	    {0,0,1,1,1,0,1,0,1,0,0,0},
	    {0,0,1,0,0,1,0,1,0,0,0,0},
	    {0,0,0,0,0,1,1,0,0,0,0,0},
	    {0,0,0,0,1,1,0,0,0,0,0,0},
	    {0,0,0,0,0,0,0,0,0,0,0,0},
	    {0,0,0,0,0,0,0,0,0,0,0,0}
};

uint8_t offScreen[ROWS][COLS] =
{
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0}
};


uint8_t display[ROWS][COLS];

uint16_t rowPins[ROWS] =
{
    GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3,
    GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7,
    GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11
};

uint16_t colPins[COLS] =
{
    GPIO_PIN_10, GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_15,
    GPIO_PIN_0,  GPIO_PIN_1,  GPIO_PIN_2,  GPIO_PIN_7,
    GPIO_PIN_6,  GPIO_PIN_5,  GPIO_PIN_4,  GPIO_PIN_3
};

uint8_t screenPower = 1;

void matrixDrawRow(uint8_t row);
void generateNewGen(uint8_t board[][COLS]);
void copyFrame(uint8_t target[][COLS], uint8_t source[][COLS]);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  copyFrame(display, initialScreen);

  while (1)
  {
	  // Update board
	  if (screenPower){
		  generateNewGen(display);
	  }

	  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) == GPIO_PIN_SET){
		  // Reset screen on button press A
		   copyFrame(display, initialScreen);
	  } else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == GPIO_PIN_SET){
		  // Trigger power on button press B

		  if (screenPower == 1){
			  copyFrame(display, offScreen);
			  screenPower = 0;
		  } else {
			   copyFrame(display, initialScreen);
			   screenPower = 1;
		  }

		  HAL_Delay(50);
	  } else if (screenPower){
		  // Refresh screen if screenPower is on
		  for (uint16_t i = 0; i < 6; i++){
			  for (uint8_t r = 0; r < ROWS; r++)
			  {
				  matrixDrawRow(r);
				  HAL_Delay(1);
			  }
		  }
	  }
  }
}

void copyFrame(uint8_t target[][COLS], uint8_t source[][COLS]){
    for (uint8_t y = 0; y < ROWS; y++) {
        for (uint8_t x = 0; x < COLS; x++) {
            target[y][x] = source[y][x];
        }
    }
}

void generateNewGen(uint8_t board[][COLS]){
	uint8_t newGen[ROWS][COLS];

	uint8_t xDirs[8] = {0, 1, 0, -1, 1, -1, 1, -1};
	uint8_t yDirs[8] = {1, 0, -1, 0, 1, -1, -1, 1};

	for (uint8_t y = 0; y < ROWS; y++){
		for (uint8_t x = 0; x < COLS; x++){
			int alive = 0;

			for(uint8_t i = 0; i < 8; i++){
				int8_t xN = x + xDirs[i];
				int8_t yN = y + yDirs[i];

				if (xN >= 0 && xN < COLS && yN >= 0 && yN < ROWS && (board[yN][xN] == 1)){
					alive++;
				}
			}

			if (board[y][x] == 1 && (alive < 2 || alive > 3)){
				newGen[y][x] = 0;
			} else if (board[y][x] == 0 && alive == 3){
				newGen[y][x] = 1;
			} else {
				newGen[y][x] = board[y][x];
			}
		}
	}

    copyFrame(display,newGen);
}

void matrixDrawRow(uint8_t row)
{
    for (int r = 0; r < ROWS; r++)
        HAL_GPIO_WritePin(GPIOB, rowPins[r], GPIO_PIN_RESET);

    for (int c = 0; c < COLS; c++)
    {
        if (display[row][c])
            HAL_GPIO_WritePin(GPIOA, colPins[c], GPIO_PIN_RESET); // LED ON
        else
            HAL_GPIO_WritePin(GPIOA, colPins[c], GPIO_PIN_SET);   // LED OFF
    }

    HAL_GPIO_WritePin(GPIOB, rowPins[row], GPIO_PIN_SET);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3
                           PA4 PA5 PA6 PA7
                           PA10 PA11 PA12 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10
                           PB11 PB3 PB4 PB5
                           PB6 PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

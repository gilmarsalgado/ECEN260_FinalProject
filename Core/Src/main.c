/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include "font.h"
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */


// Display dimensions
#define NUMROWS 160
#define NUMCOLS 128
// Common 16-bit RGB colors: bbbbb gggggg rrrrr
#define BLACK 0x0000 // 00000 000000 00000
#define WHITE 0xFFFF // 11111 111111 11111 (red + green + blue)
#define RED 0x001F // 00000 000000 11111
#define GREEN 0x07E0 // 00000 111111 00000
#define BLUE 0xF800 // 11111 000000 00000
#define CYAN 0xFFE0 // 11111 111111 00000 (green + blue)
#define MAGENTA 0xF81F // 11111 000000 11111 (red + blue)
#define YELLOW 0x07FF // 00000 111111 11111 (red + green)
#define ORANGE 0x041F // 00000 100000 11111 (red + 50% green)
#define PURPLE 0x8010 // 10000 000000 10000 (50% red + 50% blue)
#define GRAY 0x8410 // 10000 100000 10000 (50% red + 50% green + 50% blue)
#define LIGHTGRAY 0xC618 // 11000 110000 11000 (75% red + 75% green + 75% blue)
#define DARKGRAY 0x4208 // 01000 010000 01000 (25% red + 25% green + 25% blue)

// ST7735 command codes
#define SWRESET 0x01
#define SLPOUT 0x11
#define COLMOD 0x3A
#define MADCTL 0x36
#define DISPON 0x29
#define CASET 0x2A
#define RASET 0x2B
#define RAMWR 0x2C
#define COLOR_MODE_16BIT 0x05
#define MADCTL_DEFAULT 0xC8


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */

#define TRIG_PIN  US_TRIG_Pin
#define TRIG_PORT US_TRIG_GPIO_Port

#define ECHO_PIN  US_ECHO_Pin
#define ECHO_PORT US_ECHO_GPIO_Port

uint16_t Distance = 0;  // cm
uint16_t Ultrasonic_ReadDistanceCm(void);
uint16_t cursor_x = 0;
uint16_t cursor_y = 0;

volatile uint8_t record_request = 0;

uint16_t recorded_distance = 0;

char record_buffer[16];

char buffer[16];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM4_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

void ST7735_WriteData(uint8_t data[], uint16_t size);
void ST7735_WriteCommand(uint8_t cmd);
void ST7735_Init(void);
void ST7735_DrawPixel(uint8_t x, uint8_t y, uint16_t color);
void ST7735_DrawScreen(uint16_t frame[NUMROWS][NUMCOLS]);
void ST7735_UpdateCharCursor(uint16_t x, uint16_t y);
void ST7735_PrintChar(char character);
void ST7735_PrintString(const char *string);
void ST7735_SetAddressWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);
void ST7735_FillScreen(uint16_t color);

void ST7735_DrawCharScaled(char character, uint16_t x, uint16_t y, uint8_t scale);
void ST7735_PrintStringScaled(const char *string, uint16_t x, uint16_t y, uint8_t scale);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM4_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  ST7735_Init();
  ST7735_FillScreen(BLACK);

  for (uint16_t x = 0; x < NUMCOLS; x++)
  {
      ST7735_DrawPixel((uint8_t)x, 106, WHITE);
  }

  HAL_TIM_Base_Start(&htim4);


  HAL_GPIO_WritePin(TRIG_PORT,
                    TRIG_PIN,
                    GPIO_PIN_RESET);



//	HAL_TIM_Base_Start(&htim4);
//	HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET); // pull the TRIG pin low
//	HAL_Delay(2000);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  Distance = Ultrasonic_ReadDistanceCm();

	     /*
	      * Clear the entire large-number area.
	      * Four characters at 4× scale fill the 128-pixel width.
	      */
	     ST7735_PrintStringScaled("    ",
	                              0,
	                              25,
	                              4);

	     if (Distance == 0)
	     {
	         snprintf(buffer,
	                  sizeof(buffer),
	                  "---");
	     }
	     else
	     {
	         snprintf(buffer,
	                  sizeof(buffer),
	                  "%u",
	                  (unsigned int)Distance);
	     }

	     /*
	      * Center the number.
	      * Each 4× character is 32 pixels wide.
	      */
	     uint16_t number_width =
	         strlen(buffer) * 32;

	     uint16_t number_x =
	         (NUMCOLS - number_width) / 2;

	     ST7735_PrintStringScaled(buffer,
	                              number_x,
	                              25,
	                              4);

	     /*
	      * Display the unit below the large number.
	      * "cm" at 2× scale is 32 pixels wide.
	      */
	     ST7735_PrintStringScaled("cm",
	                              48,
	                              65,
	                              2);

	     if (record_request == 1)
	     {
	         record_request = 0;

	         recorded_distance = Distance;

	         if (recorded_distance == 0)
	         {
	             snprintf(record_buffer,
	                      sizeof(record_buffer),
	                      "--- cm");
	         }
	         else
	         {
	             snprintf(record_buffer,
	                      sizeof(record_buffer),
	                      "%3u cm",
	                      (unsigned int)recorded_distance);
	         }

	         /*
	          * Clear the saved-measurement area.
	          */
	         ST7735_PrintStringScaled("      ",
	                                  16,
	                                  125,
	                                  2);

	         /*
	          * Display the saved measurement.
	          */
	         ST7735_PrintStringScaled(record_buffer,
	                                  16,
	                                  125,
	                                  2);
	     }

	     HAL_Delay(100);




//	  Distance = Ultrasonic_ReadDistanceCm();
//
//	     LCD_SetCursor(0, 0);
//	     LCD_Print("                ");
//
//	     LCD_SetCursor(0, 0);
//
//	     if (Distance == 0)
//	     {
//	         LCD_Print("No echo");
//	     }
//	     else
//	     {
//	         snprintf(buffer,
//	                  sizeof(buffer),
//	                  "%u cm",
//	                  Distance);
//
//	         LCD_Print(buffer);
//	     }

	     HAL_Delay(100);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10D19CE4;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 79;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

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
  HAL_GPIO_WritePin(GPIOA, RES_Pin|CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, DC_Pin|US_TRIG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RES_Pin DC_Pin CS_Pin US_TRIG_Pin */
  GPIO_InitStruct.Pin = RES_Pin|DC_Pin|CS_Pin|US_TRIG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : RECORD_BTN_Pin */
  GPIO_InitStruct.Pin = RECORD_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(RECORD_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : US_ECHO_Pin */
  GPIO_InitStruct.Pin = US_ECHO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(US_ECHO_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void ST7735_WriteData(uint8_t data[], uint16_t size)
{
    HAL_GPIO_WritePin(DC_GPIO_Port,
                      DC_Pin,
                      GPIO_PIN_SET);

    HAL_GPIO_WritePin(CS_GPIO_Port,
                      CS_Pin,
                      GPIO_PIN_RESET);

    HAL_SPI_Transmit(&hspi1,
                     data,
                     size,
                     HAL_MAX_DELAY);

    HAL_GPIO_WritePin(CS_GPIO_Port,
                      CS_Pin,
                      GPIO_PIN_SET);
}

void ST7735_WriteCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(DC_GPIO_Port,
                      DC_Pin,
                      GPIO_PIN_RESET);

    HAL_GPIO_WritePin(CS_GPIO_Port,
                      CS_Pin,
                      GPIO_PIN_RESET);

    HAL_SPI_Transmit(&hspi1,
                     &cmd,
                     1,
                     HAL_MAX_DELAY);

    HAL_GPIO_WritePin(CS_GPIO_Port,
                      CS_Pin,
                      GPIO_PIN_SET);
}

void ST7735_Init(void)
{
    uint8_t data;

    HAL_GPIO_WritePin(CS_GPIO_Port,
                      CS_Pin,
                      GPIO_PIN_SET);

    HAL_GPIO_WritePin(RES_GPIO_Port,
                      RES_Pin,
                      GPIO_PIN_RESET);

    HAL_Delay(5);

    HAL_GPIO_WritePin(RES_GPIO_Port,
                      RES_Pin,
                      GPIO_PIN_SET);

    HAL_Delay(5);

    ST7735_WriteCommand(SWRESET);
    HAL_Delay(150);

    ST7735_WriteCommand(SLPOUT);
    HAL_Delay(150);

    ST7735_WriteCommand(COLMOD);

    data = COLOR_MODE_16BIT;
    ST7735_WriteData(&data, 1);

    ST7735_WriteCommand(MADCTL);

    data = MADCTL_DEFAULT;
    ST7735_WriteData(&data, 1);

    ST7735_WriteCommand(DISPON);
    HAL_Delay(10);
}

void ST7735_SetAddressWindow(uint16_t xStart,
                             uint16_t yStart,
                             uint16_t xEnd,
                             uint16_t yEnd)
{
    uint8_t address[4];

    if (xStart > xEnd ||
        yStart > yEnd ||
        xEnd >= NUMCOLS ||
        yEnd >= NUMROWS)
    {
        return;
    }

    xStart += 2;
    xEnd   += 2;
    yStart += 1;
    yEnd   += 1;

    ST7735_WriteCommand(CASET);

    address[0] = xStart >> 8;
    address[1] = xStart & 0xFF;
    address[2] = xEnd >> 8;
    address[3] = xEnd & 0xFF;

    ST7735_WriteData(address, 4);

    ST7735_WriteCommand(RASET);

    address[0] = yStart >> 8;
    address[1] = yStart & 0xFF;
    address[2] = yEnd >> 8;
    address[3] = yEnd & 0xFF;

    ST7735_WriteData(address, 4);

    ST7735_WriteCommand(RAMWR);
}

void ST7735_DrawPixel(uint8_t x,
                      uint8_t y,
                      uint16_t color)
{
    uint8_t pixelData[2];

    if (x >= NUMCOLS || y >= NUMROWS)
    {
        return;
    }

    ST7735_SetAddressWindow(x, y, x, y);

    pixelData[0] = color >> 8;
    pixelData[1] = color & 0xFF;

    ST7735_WriteData(pixelData, 2);
}

void ST7735_FillScreen(uint16_t color)
{
    uint8_t row_data[NUMCOLS * 2];

    for (uint16_t x = 0; x < NUMCOLS; x++)
    {
        row_data[2 * x]     = color >> 8;
        row_data[2 * x + 1] = color & 0xFF;
    }

    ST7735_SetAddressWindow(0, 0, NUMCOLS - 1, NUMROWS - 1);

    HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);

    for (uint16_t y = 0; y < NUMROWS; y++)
    {
        HAL_SPI_Transmit(&hspi1,
                         row_data,
                         sizeof(row_data),
                         HAL_MAX_DELAY);
    }

    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
}

void ST7735_UpdateCharCursor(uint16_t x,
                             uint16_t y)
{
    cursor_x = x;
    cursor_y = y;

    if (cursor_x > NUMCOLS - 8)
    {
        cursor_x = 0;
        cursor_y += 8;
    }

    if (cursor_y > NUMROWS - 9)
    {
        cursor_x = 0;
        cursor_y = 0;
    }
}

void ST7735_PrintChar(char character)
{
    if (character < ' ' || character > '~')
    {
        return;
    }

    int font_index = character - 0x20;
    const uint8_t *character_data = fontTable[font_index];

    uint8_t pixel_data[8 * 8 * 2];
    uint16_t index = 0;

    for (int row = 0; row < 8; row++)
    {
        uint8_t row_data = character_data[row];

        for (int col = 7; col >= 0; col--)
        {
            uint16_t color;

            if (row_data & (1 << col))
            {
                color = WHITE;
            }
            else
            {
                color = BLACK;
            }

            pixel_data[index++] = color >> 8;
            pixel_data[index++] = color & 0xFF;
        }
    }

    ST7735_SetAddressWindow(cursor_x,
                            cursor_y,
                            cursor_x + 7,
                            cursor_y + 7);

    ST7735_WriteData(pixel_data, sizeof(pixel_data));

    ST7735_UpdateCharCursor(cursor_x + 8, cursor_y);
}

void ST7735_PrintString(const char *string)
{
    while (*string != '\0')
    {
        ST7735_PrintChar(*string);
        string++;
    }
}

void ST7735_DrawCharScaled(char character,
                           uint16_t x,
                           uint16_t y,
                           uint8_t scale)
{
    if (character < ' ' || character > '~')
    {
        return;
    }

    if (scale == 0)
    {
        return;
    }

    if (scale > 4)
    {
        scale = 4;
    }

    uint16_t width = 8 * scale;
    uint16_t height = 8 * scale;

    if ((x + width > NUMCOLS) ||
        (y + height > NUMROWS))
    {
        return;
    }

    const uint8_t *character_data =
        fontTable[character - 0x20];

    /*
     * Maximum row size:
     * 8 source pixels × 4 scale × 2 color bytes
     */
    uint8_t row_buffer[8 * 4 * 2];

    ST7735_SetAddressWindow(x,
                            y,
                            x + width - 1,
                            y + height - 1);

    /*
     * Keep CS low while sending the entire character.
     */
    HAL_GPIO_WritePin(DC_GPIO_Port,
                      DC_Pin,
                      GPIO_PIN_SET);

    HAL_GPIO_WritePin(CS_GPIO_Port,
                      CS_Pin,
                      GPIO_PIN_RESET);

    for (int row = 0; row < 8; row++)
    {
        uint8_t source_row = character_data[row];

        for (int repeat_y = 0; repeat_y < scale; repeat_y++)
        {
            uint16_t index = 0;

            for (int col = 7; col >= 0; col--)
            {
                uint16_t color;

                if (source_row & (1U << col))
                {
                    color = WHITE;
                }
                else
                {
                    color = BLACK;
                }

                for (int repeat_x = 0; repeat_x < scale; repeat_x++)
                {
                    row_buffer[index++] = color >> 8;
                    row_buffer[index++] = color & 0xFF;
                }
            }

            HAL_SPI_Transmit(&hspi1,
                             row_buffer,
                             index,
                             HAL_MAX_DELAY);
        }
    }

    HAL_GPIO_WritePin(CS_GPIO_Port,
                      CS_Pin,
                      GPIO_PIN_SET);
}


void ST7735_PrintStringScaled(const char *string,
                              uint16_t x,
                              uint16_t y,
                              uint8_t scale)
{
    if (scale == 0)
    {
        return;
    }

    if (scale > 4)
    {
        scale = 4;
    }

    while ((*string != '\0') &&
           (x + (8 * scale) <= NUMCOLS))
    {
        ST7735_DrawCharScaled(*string,
                              x,
                              y,
                              scale);

        x += 8 * scale;
        string++;
    }
}


uint16_t Ultrasonic_ReadDistanceCm(void)
{
    uint32_t wait_start;
    uint32_t echo_start;
    uint32_t echo_time;

    /*
     * Send a 10-microsecond trigger pulse.
     */
    HAL_GPIO_WritePin(TRIG_PORT,
                      TRIG_PIN,
                      GPIO_PIN_RESET);

    __HAL_TIM_SET_COUNTER(&htim4, 0);

    HAL_GPIO_WritePin(TRIG_PORT,
                      TRIG_PIN,
                      GPIO_PIN_SET);

    while (__HAL_TIM_GET_COUNTER(&htim4) < 10)
    {
        /* Wait 10 microseconds */
    }

    HAL_GPIO_WritePin(TRIG_PORT,
                      TRIG_PIN,
                      GPIO_PIN_RESET);

    /*
     * Wait for Echo to become high.
     */
    wait_start = HAL_GetTick();

    while (HAL_GPIO_ReadPin(ECHO_PORT,
                            ECHO_PIN) == GPIO_PIN_RESET)
    {
        if ((HAL_GetTick() - wait_start) >= 50)
        {
            return 0;
        }
    }

    /*
     * Measure how long Echo remains high.
     */
    echo_start = __HAL_TIM_GET_COUNTER(&htim4);

    wait_start = HAL_GetTick();

    while (HAL_GPIO_ReadPin(ECHO_PORT,
                            ECHO_PIN) == GPIO_PIN_SET)
    {
        if ((HAL_GetTick() - wait_start) >= 50)
        {
            return 0;
        }
    }

    echo_time =
        __HAL_TIM_GET_COUNTER(&htim4) - echo_start;

    /*
     * Timer is running at 1 microsecond per count.
     * Distance in centimeters = echo time / 58.
     */
    return (uint16_t)((echo_time + 29) / 58);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint32_t last_button_tick = 0;
    uint32_t current_tick = HAL_GetTick();

    if (GPIO_Pin == RECORD_BTN_Pin)
    {
        /*
         * Ignore button bounce for 200 ms.
         */
        if ((current_tick - last_button_tick) >= 200)
        {
            record_request = 1;
            last_button_tick = current_tick;
        }
    }
}
//
//
//void LCD_WriteNibble(uint8_t nibble, uint8_t rs)
//{
//    uint8_t data;
//
//    data = (nibble & 0x0F) << LCD_D4_BIT;
//    data |= (rs & 0x01) << LCD_RS_BIT;
//    data |= 1 << LCD_BL_BIT;
//    data |= 1 << LCD_EN_BIT;
//
//    HAL_I2C_Master_Transmit(&hi2c1,
//                            I2C_ADDR << 1,
//                            &data,
//                            1,
//                            100);
//
//    HAL_Delay(1);
//
//    data &= ~(1 << LCD_EN_BIT);
//
//    HAL_I2C_Master_Transmit(&hi2c1,
//                            I2C_ADDR << 1,
//                            &data,
//                            1,
//                            100);
//}
//
//void LCD_SendCommand(uint8_t command)
//{
//    LCD_WriteNibble(command >> 4, 0);
//    LCD_WriteNibble(command & 0x0F, 0);
//
//    if (command == 0x01 || command == 0x02)
//    {
//        HAL_Delay(2);
//    }
//}
//
//void LCD_SendData(uint8_t data)
//{
//    LCD_WriteNibble(data >> 4, 1);
//    LCD_WriteNibble(data & 0x0F, 1);
//}
//
//void LCD_Init(void)
//{
//    HAL_Delay(50);
//
//    LCD_WriteNibble(0x03, 0);
//    HAL_Delay(5);
//
//    LCD_WriteNibble(0x03, 0);
//    HAL_Delay(1);
//
//    LCD_WriteNibble(0x03, 0);
//    HAL_Delay(1);
//
//    LCD_WriteNibble(0x02, 0);
//
//    LCD_SendCommand(0x28);
//    LCD_SendCommand(0x0C);
//    LCD_SendCommand(0x06);
//    LCD_SendCommand(0x01);
//
//    HAL_Delay(2);
//}
//
//void LCD_SetCursor(uint8_t row, uint8_t column)
//{
//    uint8_t address;
//
//    if (row == 0)
//        address = 0x00;
//    else
//        address = 0x40;
//
//    LCD_SendCommand(0x80 | (address + column));
//}
//
//void LCD_Print(const char *text)
//{
//    while (*text != '\0')
//    {
//        LCD_SendData(*text);
//        text++;
//    }
//}
//
//void LCD_Clear(void)
//{
//    LCD_SendCommand(0x01);
//    HAL_Delay(2);
//}

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
	while (1) {
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

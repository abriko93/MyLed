#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_dac.h"
#include "stm32f4xx_dma.h"

#include "main.h"

/*******************************************************************/
// Объявляем переменные

#define BUFFERSIZE                       24
#define LEDSNUMBER                       12

GPIO_InitTypeDef GPIO_InitStructure;
DMA_InitTypeDef DMA_InitStructure; 
SPI_InitTypeDef SPI_InitStructure;

uint8_t aTxBuffer[BUFFERSIZE];
uint8_t Byte1 = 0xFC;
uint8_t Byte0 = 0xC0;
uint8_t ByteRes = 0x00;
uint8_t GREEN[LEDSNUMBER];
uint8_t RED[LEDSNUMBER];
uint8_t BLUE[LEDSNUMBER];
uint8_t LED_count = 0;
uint32_t random32bit = 0;

static void SPI_Config(void);
static void New_Led_Data(uint8_t LED_count);
static void RNG_Config(void);
static void TIM4_Config(void);

  int main()
{
  /* RNG configuration */
  RNG_Config();
  /* SPI configuration */
  SPI_Config();
  
  TIM4_Config();
          
    while(1)
    {
    // А тут мы ничего не делаем, вся работа у нас в прерывании
        __NOP();
    } 
}
static void New_Led_Data(uint8_t LED_count)
{      
  //Подготавливаем массив для отправки в стветодиоды
  uint8_t i = 0;
  for (uint8_t G_count = 0; G_count < 8; G_count++)
  {
    if ((GREEN[LED_count]<<G_count) & 0x80)
    {  
      aTxBuffer[i]=Byte1;
    }
    else
    {
      aTxBuffer[i]=Byte0;
    }
    i++;
  }
  for (uint8_t R_count = 0; R_count < 8; R_count++)
  {
    if ((RED[LED_count]<<R_count) & 0x80)
    {  
      aTxBuffer[i]=Byte1;
    }
    else
    {
      aTxBuffer[i]=Byte0;
    }
    i++;
  }
  for (uint8_t B_count = 0; B_count < 8; B_count++)
  {
    if ((BLUE[LED_count]<<B_count) & 0x80)
    {  
      aTxBuffer[i]=Byte1;
    }
    else
    {
      aTxBuffer[i]=Byte0;
    }
    i++;
  }
}


static void RNG_Config(void)
{  
 /* Enable RNG clock source */
  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);

  /* RNG Peripheral enable */
  RNG_Cmd(ENABLE);
}

        

static void SPI_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;

  /* Peripheral Clock Enable -------------------------------------------------*/
  /* Enable the SPI clock */
  SPIx_CLK_INIT(SPIx_CLK, ENABLE);
  
  /* Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(SPIx_SCK_GPIO_CLK | SPIx_MISO_GPIO_CLK | SPIx_MOSI_GPIO_CLK, ENABLE);
  
  /* Enable DMA clock */
  RCC_AHB1PeriphClockCmd(SPIx_DMA_CLK, ENABLE);

  /* SPI GPIO Configuration --------------------------------------------------*/
  /* GPIO Deinitialisation */
  GPIO_DeInit(SPIx_SCK_GPIO_PORT);
  GPIO_DeInit(SPIx_MISO_GPIO_PORT);
  GPIO_DeInit(SPIx_MOSI_GPIO_PORT);
  
  /* Connect SPI pins to AF5 */  
  GPIO_PinAFConfig(SPIx_SCK_GPIO_PORT, SPIx_SCK_SOURCE, SPIx_SCK_AF);
  GPIO_PinAFConfig(SPIx_MISO_GPIO_PORT, SPIx_MISO_SOURCE, SPIx_MISO_AF);    
  GPIO_PinAFConfig(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_SOURCE, SPIx_MOSI_AF);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

  /* SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = SPIx_SCK_PIN;
  GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStructure);
  
  /* SPI  MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin =  SPIx_MISO_PIN;
  GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStructure);  

  /* SPI  MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  SPIx_MOSI_PIN;
  GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStructure);
 
  /* SPI configuration -------------------------------------------------------*/
  SPI_I2S_DeInit(SPIx);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  
  /* DMA configuration -------------------------------------------------------*/
  /* Deinitialize DMA Streams */
  DMA_DeInit(SPIx_TX_DMA_STREAM);
  
  /* Configure DMA Initialization Structure */
  DMA_InitStructure.DMA_BufferSize = BUFFERSIZE+1;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(SPIx->DR)) ;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  /* Configure TX DMA */
  DMA_InitStructure.DMA_Channel = SPIx_TX_DMA_CHANNEL ;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
  DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)aTxBuffer ;
  DMA_Init(SPIx_TX_DMA_STREAM, &DMA_InitStructure);
  
  /* Initializes the SPI communication */
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_Init(SPIx, &SPI_InitStructure);
  
  /* The Data transfer is performed in the SPI using Direct Memory Access */
  
      /* Enable DMA SPI TX Stream */
  DMA_Cmd(SPIx_TX_DMA_STREAM,ENABLE);
  
  /* Enable SPI DMA TX Requsts */
  SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Tx, ENABLE);
  
  /* Разрешаем прерывание по окончанию передачи данных */
  DMA_ITConfig(SPIx_TX_DMA_STREAM, DMA_IT_TC, ENABLE);
    
  NVIC_EnableIRQ(DMA1_Stream4_IRQn);
 
    /* Enable the SPI peripheral */
  SPI_Cmd(SPIx, ENABLE); 
} 


static void TIM4_Config(void)
{
    TIM_TimeBaseInitTypeDef timer;
    //Включаем тактирование таймера TIM4
    //Таймер 4 у нас висит на шине APB1
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    //Заполняем поля структуры дефолтными значениями
    TIM_TimeBaseStructInit(&timer);
    //Выставляем предделитель
    timer.TIM_Prescaler = 500-1;
    //Тут значение, досчитав до которого таймер сгенерирует прерывание
    //Кстати это значение мы будем менять в самом прерывании
    timer.TIM_Period = 5000-1;
    //Инициализируем TIM4 нашими значениями
    TIM_TimeBaseInit(TIM4, &timer);	
    //Настраиваем таймер для генерации прерывания по обновлению (переполнению)
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
    //Запускаем таймер 
    TIM_Cmd(TIM4, ENABLE);
    //Разрешаем соответствующее прерывание
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
   
}

void TIM4_IRQHandler()
{	
     TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

     //Подготавливаем значения цветов
     for (uint8_t ii = 0; ii < LEDSNUMBER; ii++)
    {
    /* Wait until one RNG number is ready */
      while(RNG_GetFlagStatus(RNG_FLAG_DRDY)== RESET)
      {
      }
      /* Get a 32bit Random number */       
    random32bit = RNG_GetRandomNumber();     
    
    if (random32bit & 0x1)
    {
      GREEN[ii] = 1; //Значение бита цвета
    }
    else
    {
      GREEN[ii] = 0; //Значение бита цвета
    }
    if (random32bit & 0x2)
    {
      RED[ii] = 1; //Значение бита цвета
    }
    else
    {
      RED[ii] = 0; //Значение бита цвета
    }
    if (random32bit & 0x4)
    {
      BLUE[ii] = 1; //Значение бита цвета
    }
    else
    {
      BLUE[ii] = 0; //Значение бита цвета
    }
    }
}

void DMA1_Stream4_IRQHandler()
{
    /* Waiting the end of Data transfer */
  if (DMA_GetFlagStatus(SPIx_TX_DMA_STREAM,SPIx_TX_DMA_FLAG_TCIF)==SET)
  {  
      if (LED_count < LEDSNUMBER)
      {
        New_Led_Data(LED_count);
        LED_count++;
        /* Clear DMA Transfer Complete Flags */
        DMA_ClearFlag(SPIx_TX_DMA_STREAM,SPIx_TX_DMA_FLAG_TCIF);
        /* Enable DMA SPI TX Stream */
        DMA_Cmd(SPIx_TX_DMA_STREAM,ENABLE);
      }
      else
      { 
          for (uint32_t i = 0; i < 12000; i++) //Задержка > 50мс
          {     
          }
          LED_count=0;
      }
  }
}

/*******************************************************************/


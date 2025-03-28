#include "gpio.h"
#include "mik32_memory_map.h"
#include "pad_config.h"
#include "power_manager.h"
#include "scr1_timer.h"
#include "wakeup.h"

uint8_t dig[] = {0b11111100,
               0b01100000,
               0b11011010,
               0b11110010,
               0b01100110,
               0b10110110,
               0b10111110,
               0b11100000,
               0b11111110, 
               0b11110110,
               };

#define DATA_PIN (8) 
#define DATA_PIN_PORT GPIO_0

#define STCP_PIN (0) // STORE
#define STCP_PIN_PORT GPIO_0 

#define SHCP_PIN (10) //SHIFT
#define SHCP_PIN_PORT GPIO_0 

#define SCR1_TIMER_GET_TIME()                                                  \
 (((uint64_t)(SCR1_TIMER->MTIMEH) << 32) | (SCR1_TIMER->MTIME))
#define SYSTEM_FREQ_HZ 32000000UL

#define SET_TWO_BIT(REG, NUM, TWO_BITS) (REG = (REG & ~PAD_CONFIG_PIN_M(NUM)) | PAD_CONFIG_PIN(NUM, TWO_BITS))

#define GPIO0_SET ((volatile uint32_t* )(GPIO_0_BASE_ADDRESS + GPIO_SET))
#define GPIO0_CLR ((volatile uint32_t* )(GPIO_0_BASE_ADDRESS + GPIO_CLEAR))

void SystemClock_Config();
void GPIO_Init();
void delay(uint32_t ms);

void shift_c(uint8_t symbol);

// print является прослойкой между while из main и shift_c, потому что после отработки shift_c в переменной indx гарантированно оказывается мусор
// другие обходы этой проблемы либо не принесли результата, либо были слишком костыльными
void print(uint8_t sign) {
 STCP_PIN_PORT-> CLEAR = 1 <<STCP_PIN;
 shift_c(dig[sign]);
 STCP_PIN_PORT-> SET = 1 <<STCP_PIN;
}

int main() {
 SystemClock_Config();
 GPIO_Init();

 uint8_t indx = 0;
 while (1) {
   print(indx);
   ++indx;
   if (indx == 10) {
     indx = 0;
   }
   delay(500);
}
return 0;
}

void SystemClock_Config(void) {
  WU->CLOCKS_SYS &=
      ~(0b11 << WU_CLOCKS_SYS_OSC32M_EN_S); // Включить OSC32M и HSI32M
  WU->CLOCKS_BU &=
      ~(0b11 << WU_CLOCKS_BU_OSC32K_EN_S); // Включить OSC32K и LSI32K

  // Поправочный коэффициент HSI32M
  WU->CLOCKS_SYS = (WU->CLOCKS_SYS & (~WU_CLOCKS_SYS_ADJ_HSI32M_M)) |
                   WU_CLOCKS_SYS_ADJ_HSI32M(128);
  // Поправочный коэффициент LSI32K
  WU->CLOCKS_BU = (WU->CLOCKS_BU & (~WU_CLOCKS_BU_ADJ_LSI32K_M)) |
                  WU_CLOCKS_BU_ADJ_LSI32K(8);

  // Автоматический выбор источника опорного тактирования
  WU->CLOCKS_SYS &= ~WU_CLOCKS_SYS_FORCE_32K_CLK_M;

  // ожидание готовности
  while (!(PM->FREQ_STATUS & PM_FREQ_STATUS_OSC32M_M))
    ;

  // переключение на тактирование от OSC32M
  PM->AHB_CLK_MUX = PM_AHB_CLK_MUX_OSC32M_M | PM_AHB_FORCE_MUX_UNFIXED;
  PM->DIV_AHB = 0;   // Задать делитель шины AHB.
  PM->DIV_APB_M = 0; // Задать делитель шины APB_M.
  PM->DIV_APB_P = 0; // Задать делитель шины APB_P.
}

void GPIO_Init() {

  /**< Включить  тактирование GPIO_0 */
  PM->CLK_APB_P_SET = PM_CLOCK_APB_P_GPIO_0_M;

  /**< Включить  тактирование GPIO_1 */
  PM->CLK_APB_P_SET = PM_CLOCK_APB_P_GPIO_1_M;

  /**< Включить  тактирование GPIO_2 */
  PM->CLK_APB_P_SET = PM_CLOCK_APB_P_GPIO_2_M;

  /**< Включить  тактирование схемы формирования прерываний GPIO */
  PM->CLK_APB_P_SET = PM_CLOCK_APB_P_GPIO_IRQ_M;

  DATA_PIN_PORT->DIRECTION_OUT = (1 << DATA_PIN) |
                                 (1 << STCP_PIN) | 
                                 (1 << SHCP_PIN) ;

}

void delay(uint32_t ms) {
  uint64_t end_mtimer = SCR1_TIMER_GET_TIME() + ms * (SYSTEM_FREQ_HZ / 1000);
  while (SCR1_TIMER_GET_TIME() < end_mtimer)
    ;
}
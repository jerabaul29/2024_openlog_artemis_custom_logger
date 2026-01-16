#include <Arduino.h>

/*

A self contained example of how to use the Artemis CTIMER to generate periodic interrupts.
For more details, see the datasheet and Apollo3 SDK documentation.
In particular: https://github.com/sparkfun/Arduino_Apollo3/blob/v1/cores/arduino/am_sdk_ap3/mcu/apollo3/hal/am_hal_ctimer.c

*/

// Timer configuration
static constexpr int TIMER_NUM = 2;
static constexpr uint32_t TIMER_FREQ_HZ = 100;  // 100 Hz interrupt rate

// Counter incremented by ISR
volatile uint32_t timer_tick_count = 0;

extern "C" void am_ctimer_isr(void);

// ISR handler for CTIMER interrupts
extern "C" void am_ctimer_isr(void)
{
  // Get interrupt status and clear
  uint32_t ui32Status = am_hal_ctimer_int_status_get(true);
  am_hal_ctimer_int_clear(ui32Status);
  
  // Check if it's timer 2A interrupt (reload/overflow)
  if (ui32Status & AM_HAL_CTIMER_INT_TIMERA2)
  {
    // do our work here - increment the tick count
    timer_tick_count++;
  }
}

void setup() {
  // Initialize serial over USB
  Serial.begin(1000000);
  while (!Serial && millis() < 5000);

  Serial.println();
  Serial.println(F("Artemis Timer ISR Example"));
  Serial.println(F("Setting up 100Hz timer interrupt..."));
  
  // Power up the clock
  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);
  
  // Enable global interrupts
  am_hal_interrupt_master_enable();
  
  // Stop timer
  am_hal_ctimer_stop(TIMER_NUM, AM_HAL_CTIMER_TIMERA);
  
  // Clear timer
  am_hal_ctimer_clear(TIMER_NUM, AM_HAL_CTIMER_TIMERA);
  
  // Configure timer in REPEAT mode with 3MHz clock
  am_hal_ctimer_config_single(TIMER_NUM, AM_HAL_CTIMER_TIMERA,
                               (AM_HAL_CTIMER_FN_REPEAT | 
                                AM_HAL_CTIMER_HFRC_3MHZ |
                                AM_HAL_CTIMER_INT_ENABLE));
  
  // Set the period for the timer
  uint32_t period = 3000000 / TIMER_FREQ_HZ;
  am_hal_ctimer_period_set(TIMER_NUM, AM_HAL_CTIMER_TIMERA, period, 0);
  
  // Clear any pending interrupts
  am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA2);
  
  // Enable the timer interrupt in main CTIMER register
  am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA2);
  
  // Enable interrupt at NVIC level
  NVIC_EnableIRQ(CTIMER_IRQn);
  
  // Start the timer
  am_hal_ctimer_start(TIMER_NUM, AM_HAL_CTIMER_TIMERA);
  Serial.println(F("Timer started!"));
  
  // to debug, one could read the timer value after a short delay
  // to check if the timer is running
  if (false){
    delay(100);
    uint32_t timer_val = am_hal_ctimer_read(TIMER_NUM, AM_HAL_CTIMER_TIMERA);
    Serial.print(F("Timer value after 100ms: "));
    Serial.println(timer_val);
    Serial.println();
  }

  // Print counter at 1 Hz
  static uint32_t last_print_time = 0;
  timer_tick_count = 0;
  uint32_t current_time = millis();
  
  // keep track of how many seconds have passed
  static uint32_t seconds_passed = 0;

  while (true){
    current_time = millis();
    if (current_time - last_print_time >= 1000) {
      last_print_time = current_time;
      seconds_passed++;
      
      // Read the volatile variable atomically
      uint32_t count = timer_tick_count;
      
      Serial.print(F("Seconds passed: "));
      Serial.println(seconds_passed);
      Serial.print(F("Timer tick count: "));
      Serial.println(count);
      Serial.println();
    }
  }
}

void loop() {
}
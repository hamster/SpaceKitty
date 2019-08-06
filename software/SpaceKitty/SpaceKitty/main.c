
#define F_CPU 20000000

#include <atmel_start.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <tca.h>

extern volatile uint8_t measurement_done_touch;
extern volatile uint8_t LED_PWM[5];
extern volatile uint16_t timeTicks;

// Logarithmic brightness levels
const uint8_t pwm_log[] = {30,43,59,78,102,137,196,255};

typedef enum {
	MODE_TWINKLE,
	MODE_BOUNCE,
	MODE_OFF
} RUNMODE;


int main(void){
	
	uint8_t key_status = 0;

	system_init();
	touch_init();
	
	cpu_irq_enable(); /* Global Interrupt Enable */

	LED1_set_level(true);
	LED5_set_level(true);
	_delay_ms(250);
	LED2_set_level(true);
	LED4_set_level(true);
	_delay_ms(250);
	LED3_set_level(true);
	_delay_ms(750);
	
	LED3_set_level(false);
	_delay_ms(100);
	LED2_set_level(false);
	LED4_set_level(false);
	_delay_ms(100);
	LED1_set_level(false);
	LED5_set_level(false);
	_delay_ms(750);
	
	TIMER_0_init();
	
	LED_PWM[0] = 14;
	LED_PWM[1] = 14;
	LED_PWM[2] = 14;
	LED_PWM[3] = 14;
	LED_PWM[4] = 14;
	
	uint8_t brightnessPosition = 0;
	uint8_t twinkleLED = 0;
	bool directionUp = true;
	bool twinkling = false;
	
	uint8_t counter = 0;
	
	RUNMODE mode = MODE_TWINKLE;
	
	bool touched = false;

	while (1) {
		
		if(!touched){
		
			if(mode == MODE_TWINKLE){
				if(!twinkling){
					// Not twinkling, wait a random time before starting a twinkle
					if((rand() % 500) == 0){
						// Time to twinkle
						twinkling = true;
						// Pick a new LED
						twinkleLED = rand() % 5;
					}
				}
		
				if(((timeTicks % 50) < 2) && twinkling){
					if(directionUp){
						// Go up in brightness
						LED_PWM[twinkleLED] = pwm_log[brightnessPosition++];
						if(brightnessPosition == 8){
							directionUp = false;
						}
					}
					else{
						// Go down in brightness
						LED_PWM[twinkleLED] = pwm_log[--brightnessPosition];
						if(brightnessPosition == 0){
							directionUp = true;
							twinkling = false;
							LED_PWM[0] = 14;
							LED_PWM[1] = 14;
							LED_PWM[2] = 14;
							LED_PWM[3] = 14;
							LED_PWM[4] = 14;
						}
					}
				}
			}
		
			if(mode == MODE_BOUNCE){
				if(counter == 75){
					counter = 0;
				
					for(uint8_t i = 0; i < 5; i++){
						if(i == twinkleLED){
							LED_PWM[i] = 128;
						}
						else{
							LED_PWM[i] = 14;
						}
					
					}	
					
					if(directionUp){
						if(twinkleLED == 4){
							twinkleLED = 3;
							directionUp = false;
						}
						else{
							twinkleLED++;
						}
					}
					else{
						twinkleLED--;
						if(twinkleLED == 0){
							directionUp = true;
						}
					}			
				}
				else{
					counter++;
				}
			}
		
			if(mode == MODE_OFF){
				if(counter == 150){
					counter = 0;
					
					LED_PWM[0] = rand() % 128;
					LED_PWM[1] = rand() % 128;
					LED_PWM[2] = rand() % 128;
					LED_PWM[3] = rand() % 128;
					LED_PWM[4] = rand() % 128;
				}
				else{
					counter++;
				}
				
			}
		
		}
		else{
			
			// Mode is about to change, light up all the LEDs
			for(uint8_t i = 0; i < 5; i++){
				LED_PWM[i] = 184;
			}
			
		}
		
		touch_process();
		if (measurement_done_touch == 1) {
			key_status = get_sensor_state(0) & KEY_TOUCHED_MASK;
			if (0u != key_status) {
				touched = true;
				_delay_ms(5);
			}
			else{
				
				if(touched){
					switch(mode){
						case MODE_TWINKLE:
							// Move to bounce mode
							twinkleLED = 0;
							directionUp = true;
							mode = MODE_BOUNCE;
							break;
						case MODE_BOUNCE:
							// Move to off mode
							mode = MODE_OFF;
							break;
						case MODE_OFF:
							// Move to twinkle mode
							LED_PWM[0] = 14;
							LED_PWM[1] = 14;
							LED_PWM[2] = 14;
							LED_PWM[3] = 14;
							LED_PWM[4] = 14;
							twinkleLED = 0;
							directionUp = true;
							brightnessPosition = 0;
							mode = MODE_TWINKLE;
							break;
					}

					touched = false;
				}
			}
			
		}
		
		_delay_ms(1);
		
	}
}

#include "user_buttons.h"
#include "gpio.h"

// About volatile -> http://www.pic24.ru/doku.php/osa/articles/volatile_for_chainiks
extern volatile uint32_t PIN_IN;
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

static volatile os_timer_t rotary_debounce_timer;
static volatile os_timer_t button_timer;
static volatile uint8 button_last_state = 0;

void ICACHE_FLASH_ATTR
button_push(uint8 debounce)
{
    if (debounce == 1)
    {
        os_timer_disarm(&button_timer);
        os_timer_setfn(&button_timer, (os_timer_func_t *)button_push, 0);
        os_timer_arm(&button_timer, 100, 1);
    }

    if (CHECK_BIT(PIN_IN,0) == 0 && button_last_state == 0)
    {
        os_printf("Button press\n");
        os_timer_disarm(&button_timer);
        os_timer_setfn(&button_timer, (os_timer_func_t *)button_push, 1);
        os_timer_arm(&button_timer, 1000, 0);
        button_last_state = 1; 
    }

    if (CHECK_BIT(PIN_IN,0) == 1 && button_last_state == 1)
        button_last_state = 0;
}

void ICACHE_FLASH_ATTR
rotary_debounce(uint8 direction)
{
    gpio_pin_intr_state_set(GPIO_ID_PIN(12), GPIO_PIN_INTR_ANYEDGE);

    if (direction == 1)
    {
        os_printf("Up");
        display_next_page();
    }
    else
    {
        os_printf("Down");
        display_prev_page();
    }
}


LOCAL void
rotary_intr_handler(int8_t key)
{
    uint8 direction = 0;
    uint32 inputs;
    
    inputs = PIN_IN; //Read input register

    //Read rotary state
    if (CHECK_BIT(inputs,13) == CHECK_BIT(inputs,12))
        direction = 1;

    //Not that sure what this does yet and where the register is used for
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

    //disable interrupt
    gpio_pin_intr_state_set(GPIO_ID_PIN(12), GPIO_PIN_INTR_DISABLE);

    //clear interrupt status
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(12));

    //Disarm timer
    os_timer_disarm(&rotary_debounce_timer);

    //Setup timer
    os_timer_setfn(&rotary_debounce_timer, (os_timer_func_t *)rotary_debounce, direction);

    //Arm the debounce timer
    os_timer_arm(&rotary_debounce_timer, 350, 0);
}

void ICACHE_FLASH_ATTR
buttons_init()
{
    //Attach the interrupt thing
    ETS_GPIO_INTR_ATTACH(rotary_intr_handler,12);

    //Disable interrupts
    ETS_GPIO_INTR_DISABLE();

    //Set GPIO to IO
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);

    //Set the GPIO to input
    gpio_output_set(0, 0, 0, GPIO_ID_PIN(12));
    gpio_output_set(0, 0, 0, GPIO_ID_PIN(13));
    gpio_output_set(0, 0, 0, GPIO_ID_PIN(0));

    //Not sure what this does
    gpio_register_set(GPIO_PIN_ADDR(12), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
          | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
          | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

    gpio_register_set(GPIO_PIN_ADDR(13), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
          | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
          | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

    gpio_register_set(GPIO_PIN_ADDR(0), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
          | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
          | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

    //clear gpio status
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(12));
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(13));
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(0));

    //re-enable gpio0 interrupt
    gpio_pin_intr_state_set(GPIO_ID_PIN(12), GPIO_PIN_INTR_ANYEDGE);

    //Global re-enable interrupts
    ETS_GPIO_INTR_ENABLE();

    os_timer_disarm(&button_timer);
    os_timer_setfn(&button_timer, (os_timer_func_t *)button_push, 1);
    os_timer_arm(&button_timer, 50, 1);
}

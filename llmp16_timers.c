#include "llmp16.h"

void llmp16_timer_init(llmp16_timer_t *timer, uint8_t PSC, uint16_t value, uint16_t init_value)
{
    timer->count = init_value;
    timer->PSC = PSC + 1;
    timer->status = 0x0000;
    timer->value = value;
    timer->init_value = init_value;
}

void llmp16_timer_count(llmp16_timer_t *timer, uint8_t clk_counter)
{
    if(clk_counter % timer->PSC == 0) timer->count++;
    if((timer->status & 0x01) == 0 && timer->count >= timer->value)
    {
        timer->count = timer->init_value;
        return; // TODO : activer une intérruption
    }

    if((timer->status & 0x01) == 1 && timer->count <= timer->value)
    {
        timer->count = timer->init_value;
        return; // TODO : activer une intérruption
    }
}

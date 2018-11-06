#ifndef MAIN_H_
#define MAIN_H_

void exti_button_handler(uint8_t irq __attribute__((unused)),
                         uint32_t sr __attribute__((unused)),
                         uint32_t dr __attribute__((unused)));


#endif

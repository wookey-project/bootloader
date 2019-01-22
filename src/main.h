#ifndef MAIN_H_
#define MAIN_H_

#define LOADER_DEBUG 1

void exti_button_handler(uint8_t irq __attribute__((unused)),
                         uint32_t sr __attribute__((unused)),
                         uint32_t dr __attribute__((unused)));

void hexdump(const uint8_t *bin, uint32_t len);

#endif

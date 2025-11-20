#pragma once
#include <stdbool.h>
bool Button_init(void);
bool Button_read(bool *pressed);
void Button_close(void);
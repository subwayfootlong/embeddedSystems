#ifndef EMA_FILTER_H
#define EMA_FILTER_H

void ema_init(float alpha);
float ema_process(float new_value);

#endif

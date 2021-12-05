#ifndef __tools_shared_serial_h_INCLUDED__
#define __tools_shared_serial_h_INCLUDED__



bool serial_init();
void serial_stats(uint32_t* r, uint32_t* w);
bool serial_open(const char* port, int baud, bool isFile=false, bool logToFile=false);
bool serial_close();

int serial_read(uint8_t* destination, int size);
int serial_write(uint8_t* source, int size);

int serial_available();
bool serial_is_open();

#endif
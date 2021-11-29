#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Linux headers
#include <unistd.h>  	// UNIX standard function definitions
#include <fcntl.h>   	// File control definitions
#include <errno.h>   	// Error number definitions
// #include <termios.h> 	// POSIX terminal control definitions (struct termios)
#include <sys/ioctl.h> // Used for TCGETS2, which is required for custom baud rates
// #include <asm/termios.h> // Terminal control definitions (struct termios)
#include <asm/ioctls.h>
#include <asm/termbits.h>

typedef struct
{
  int fd;
  bool open;
  const char* port;
  int baud;
} SerialState_t;

static SerialState_t s;

// ---------------------------------------------------------------
bool serial_init()
{
  

}

void serial_configure()
{
  struct termios2 tty;
 
  ioctl(s.fd, TCGETS2, &tty);

		

  tty.c_cflag &= ~CBAUD;
  tty.c_cflag |= CBAUDEX;
  // tty.c_cflag |= BOTHER;
  tty.c_ispeed = s.baud;
  tty.c_ospeed = s.baud;


  tty.c_cflag     &=  ~PARENB;       	// No parity bit is added to the output characters
  tty.c_cflag     &=  ~CSTOPB;		// Only one stop-bit is used
  tty.c_cflag     &=  ~CSIZE;			// CSIZE is a mask for the number of bits per character
  tty.c_cflag     |=  CS8;			// Set to 8 bits per character
  tty.c_cflag     &=  ~CRTSCTS;       // Disable hadrware flow control (RTS/CTS)
  tty.c_cflag     |=  CREAD | CLOCAL;     				// Turn on READ & ignore ctrl lines (CLOCAL = 1)

  //===================== (.c_oflag) =================//

	tty.c_oflag     =   0;              // No remapping, no delays
	tty.c_oflag     &=  ~OPOST;			// Make raw

		//================= CONTROL CHARACTERS (.c_cc[]) ==================//
  // No timeout (non-blocking)
  tty.c_cc[VTIME] = 0;
  tty.c_cc[VMIN] = 0;

  //=========================== LOCAL MODES (c_lflag) =======================//

  // Canonical input is when read waits for EOL or EOF characters before returning. In non-canonical mode, the rate at which
  // read() returns is instead controlled by c_cc[VMIN] and c_cc[VTIME]
  tty.c_lflag		&= ~ICANON;	
  tty.c_lflag &= ~(ECHO);
  tty.c_lflag		&= ~ECHOE;								// Turn off echo erase (echo erase only relevant if canonical input is active)
  tty.c_lflag		&= ~ECHONL;								//
  tty.c_lflag		&= ~ISIG;

  tty.c_iflag &= (tcflag_t) ~(INLCR | IGNCR | ICRNL | IGNBRK | IXON | IXOFF);

  ioctl(s.fd, TCSETS2, &tty);
}

// ---------------------------------------------------------------
bool serial_open(const char* port, int baud)
{
  s.fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);

  if (s.fd == -1)
    return false;

  s.port = port;
  s.baud = baud;
  s.open = true;

  serial_configure();
}


// ---------------------------------------------------------------
bool serial_close()
{
  if(s.fd <= 0)
    return false;
  
  close(s.fd);
  s.fd = 0;

  s.open = false;
  s.port = NULL;
  s.baud = 0;
  return true;
}

// ---------------------------------------------------------------
int serial_read(uint8_t* destination, int size)
{
  if(!s.open)
    return 0;

  int r = read(s.fd, destination, size);

  return r;
}

// ---------------------------------------------------------------
int serial_write(uint8_t* source, int size)
{
  if(!s.open)
    return 0;

  int r = write(s.fd, source, size);

  return 0;
}

// ---------------------------------------------------------------
int serial_available()
{
  if(!s.open)
    return 0;

  int count = 0;
  ioctl (s.fd, TIOCINQ, &count);
  return count;
}

// ---------------------------------------------------------------
bool serial_is_open()
{
  return s.open;
}






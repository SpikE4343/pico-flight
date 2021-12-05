#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

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
  uint32_t readBytes;
  uint32_t writeBytes;
  bool logToFile;
  FILE* logFD;

  bool isFile;
  uint32_t fileSize;
} SerialState_t;

static SerialState_t s;

// ---------------------------------------------------------------
bool serial_init()
{
  s.isFile = false;
  s.readBytes = 0;
  s.writeBytes = 0;
  s.logToFile = false;
  s.fileSize = 0;
}

void serial_stats(uint32_t* r, uint32_t* w)
{
  *r = s.readBytes;
  *w = s.writeBytes;
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
bool serial_open(const char* port, int baud, bool isFile, bool logToFile)
{
  int flags = O_RDWR | O_NOCTTY | O_NONBLOCK;
  
  if(isFile) 
    flags = O_RDONLY;

  
  s.fd = open(port, flags);

  if (s.fd == -1)
    return false;
  
  
  if(logToFile)
  {
    s.logToFile = true;
    char buf[256];
    sprintf(buf, "serial-in-%d-%u.raw", baud, (uint32_t)time(0));
    s.logFD = fopen(buf, "wb");
  }

  s.port = port;
  s.baud = baud;
  s.open = true;
  s.isFile = isFile;
  
  if(!isFile)
  {
    serial_configure();
  }
  else
  {
    s.fileSize = lseek(s.fd, 0, SEEK_END);
    lseek(s.fd, 0, SEEK_SET);
  }
}


// ---------------------------------------------------------------
bool serial_close()
{
  if(s.fd <= 0)
    return false;
  
  close(s.fd);
  s.fd = 0;

  if(s.logFD)
    fclose(s.logFD);

  s.open = false;
  s.port = NULL;
  s.baud = 0;
  s.isFile = false;
  s.logToFile = false;
  return true;
}

// ---------------------------------------------------------------
int serial_read(uint8_t* destination, int size)
{
  if(!s.open)
    return 0;

  // if( s.fileSize s.readBytes + size)
  int r = read(s.fd, destination, size);
  s.readBytes += r;

  int remain = size - r;
  if(s.isFile && remain > 0)
  {
    // probably reached the end of the file so jump back to the start
    // and read the rest
    lseek(s.fd, 0, SEEK_SET);
    int r2 = read(s.fd, destination+r, size);
    s.readBytes += r2;
  }

  if(s.logToFile && s.logFD)
  {
    fwrite(destination,1, r, s.logFD);
  }

  return r;
}

// ---------------------------------------------------------------
int serial_write(uint8_t* source, int size)
{
  if(!s.open)
    return 0;

  if(s.isFile)
    return size;

  int w = write(s.fd, source, size);
  s.writeBytes += w;
  return w;
}

// ---------------------------------------------------------------
int serial_available()
{
  if(!s.open)
    return 0;

  if(s.isFile)
    return 1;

  int count = 0;
  ioctl (s.fd, TIOCINQ, &count);
  return count;
}

// ---------------------------------------------------------------
bool serial_is_open()
{
  return s.open;
}






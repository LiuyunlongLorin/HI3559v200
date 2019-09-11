#include <termios.h>

int tcgetattr(int fd, struct termios *s)
{
    return ioctl(fd, TCGETS, s);
}

int tcsetattr(int fd, int __opt, const struct termios *s)
{
    return ioctl(fd, __opt, (void *)s);
}

int tcflow(int fd, int action)
{
    return ioctl(fd, TCXONC, (void *)(intptr_t)action);
}

int tcflush(int fd, int __queue)
{
    return ioctl(fd, TCFLSH, (void *)(intptr_t)__queue);
}

pid_t tcgetsid(int fd)
{
    pid_t _pid;
    return ioctl(fd, TIOCGSID, &_pid) ? (pid_t)-1 : _pid;
}

int tcsendbreak(int fd, int __duration)
{
    return ioctl(fd, TCSBRKP, (void *)(uintptr_t)__duration);
}

speed_t cfgetospeed(const struct termios *s)
{
    return (speed_t)(s->c_cflag & CBAUD);
}

int cfsetospeed(struct termios *s, speed_t  speed)
{
    s->c_cflag = (s->c_cflag & ~CBAUD) | (speed & CBAUD);
    return 0;
}

speed_t cfgetispeed(const struct termios *s)
{
    return (speed_t)(s->c_cflag & CBAUD);
}

int cfsetispeed(struct termios *s, speed_t  speed)
{
    s->c_cflag = (s->c_cflag & ~CBAUD) | (speed & CBAUD);
  return 0;
}

void cfmakeraw(struct termios *s)
{
    s->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    s->c_oflag &= ~OPOST;
    s->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    s->c_cflag &= ~(CSIZE|PARENB);
    s->c_cflag |= CS8;
}


#include "types.h"
#include "cpu.h"
#include "string.h"
#include "spin_lock.h"
#include "io.h"

static struct spinlock lock;
static int serial_iobase = 0x3f8;
static int serial_inited = 0;

static void serial_outb(char ch)
{
        uint8_t lsr;

        do {
                lsr = inb(serial_iobase + 0x05);
        } while (!(lsr & 0x20));

        outb(serial_iobase + 0x00, ch);
}

static void serial_put(char ch)
{
        /* Force carriage return to be performed on \n */
        if (ch == '\n')
                serial_outb('\r');
        serial_outb(ch);
}

static void serial_init(void)
{
        uint8_t lcr;

        /* set DLAB */
        lcr = inb(serial_iobase + 0x03);
        lcr |= 0x80;
        outb(serial_iobase + 0x03, lcr);

        /* set baud rate to 115200 */
        outb(serial_iobase + 0x00, 0x01);
        outb(serial_iobase + 0x01, 0x00);

        /* clear DLAB */
        lcr = inb(serial_iobase + 0x03);
        lcr &= ~0x80;
        outb(serial_iobase + 0x03, lcr);

        /* IER: disable interrupts */
        outb(serial_iobase + 0x01, 0x00);
        /* LCR: 8 bits, no parity, one stop bit */
        outb(serial_iobase + 0x03, 0x03);
        /* FCR: disable FIFO queues */
        outb(serial_iobase + 0x02, 0x00);
        /* MCR: RTS, DTR on */
        outb(serial_iobase + 0x04, 0x03);
}

static void print_serial(const char *buf)
{
	unsigned long len = strlen(buf);

    unsigned long i;
    if (!serial_inited) {
        serial_init();
        serial_inited = 1;
    }

    for (i = 0; i < len; i++) {
        serial_put(buf[i]);
    }

}

void k_puts(const char *s)
{
	// spin_lock(&lock);
	print_serial(s);
	// spin_unlock(&lock);
}

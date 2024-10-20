#include <libpynq.h>
int main (void)
{
  // initialise all I/O
  pynq_init();
  iic_init(IIC0);
  uint32_t i;
  // you can use multiple slaves, here only one is shown
  uint32_t slave_address = 0x70;
  // read out all registers of the slave
  for (int reg=0; reg < 32; reg++) {
    if (iic_read_register(IIC0, slave_address, reg, (uint8_t *) &i, 4)) { // 4 bytes
      printf("register[%d]=error\n",reg);
    } else {
      printf("register[%d]=%d\n",reg,i);
    }
  }
  // clean up after use
  pynq_destroy();
  return EXIT_SUCCESS;
}

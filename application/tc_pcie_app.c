#include <time.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include "../driver/tc_pcie_cmd.h"

#define DMA_PENDING_TIMEOUT_COUNT 1000

#define ERR_OUT(_x_) do {printf(_x_); return -1;} while(0)

void **mem_info;
struct tc_ioc_data_struct *param;
char *char_buffer;
int fd;
struct timeval dma0Time, dma1Time;
unsigned long dma0Bytes, dma1Bytes;


int GetDec(char *prompt)
{
  int ret=0;
  printf("%s", prompt);
  scanf("%d", &ret);
  return ret;
}

int GetHex(char *prompt)
{
  int ret=0;
  printf("%s", prompt);
  scanf("%x", &ret);
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
int Bar0Write(int offset, int length, unsigned int *data)
{
  int err, cnt;
  param->index = 0; // BAR0
  param->offset = offset;
  param->length = length;
  for(cnt=0; cnt<length; cnt++)
    param->data[cnt] = data[cnt];
  err = ioctl(fd, TC_IOC_BAR0_WR, param);
  if (err)
  {
    printf("TC_IOC_BAR0_WR failed\n");
    return -1;
  }
  return 0;
}

int Bar0Read(int offset, int length, unsigned int *data)
{
  int err, cnt;
  param->index = 0; // BAR0
  param->offset = offset;
  param->length = length;
  err = ioctl(fd, TC_IOC_BAR0_RD, param);
  if (err)
  {
    printf("TC_IOC_BAR0_RD failed\n");
    return -1;
  }
  for(cnt=0; cnt<length; cnt++)
    data[cnt] = param->data[cnt];
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// DMA functions

static int StatusRegWaitForBit(int flagBit) // this would be in ISR
{
  int cnt, err, temp;

  // wait for bit to be set
  for(cnt=0; cnt<DMA_PENDING_TIMEOUT_COUNT; cnt++)
  {
    err = Bar0Read(BAR0_STATUS_REG, 1, &temp);
    if (err) ERR_OUT("Could not read BAR0_STATUS_REG\n");
    if (temp & (1<<flagBit))
      break;
//    printf(".");
  }
//  printf("\n");
  if (cnt == DMA_PENDING_TIMEOUT_COUNT)
    ERR_OUT("flagBit was still not done in DMA_PENDING_TIMEOUT_COUNT\n");

  // clear bit
  temp = (1 << flagBit);
  err = Bar0Write(BAR0_STATUS_REG, 1, &temp);
  if (err) ERR_OUT("Could not write to BAR0_STATUS_REG to clear flagBit\n");
//  err = Bar0Read(BAR0_STATUS_REG, 1, &temp);
//  if (err) ERR_OUT("Could not read BAR0_STATUS_REG to check flagBit\n");
//  if (temp & (1 << flagBit))
//    ERR_OUT("flagBit is still set after clearing\n");
  return 0;
}

void RecordTime(struct timeval *tv, struct timeval t0, struct timeval t1)
{
  if ( (t0.tv_usec == t1.tv_usec) && (t0.tv_sec == t1.tv_sec) )
    return;
    
  if (t0.tv_usec > t1.tv_usec)
  {
    tv->tv_usec += t0.tv_usec-t1.tv_usec;
    tv->tv_sec++;
  }else tv->tv_usec += t1.tv_usec-t0.tv_usec;
  
  tv->tv_sec += t1.tv_sec-t0.tv_sec;
}

int DMAWriteToDev(int size_in_bytes)  // data must already be in mem_info[0]
{
  int err, size_in_words, temp, cnt;
  unsigned int data[3];
  struct timeval t0, t1;  
  gettimeofday(&t0, NULL); 
  
  if (size_in_bytes > TC_KMEM_LENGTH_BYTES)
    ERR_OUT("size_in_bytes > TC_KMEM_LENGTH_BYTES\n");

  // check if DMA was pending
  err = Bar0Read(BAR0_DMA0_ADDR_LO, 1, &temp);
  if (err) ERR_OUT("Could not read from BAR0_DMA0_ADDR_LO to determine DMA in progress bit\n");
  if (temp & 1)
    ERR_OUT("DMA0 is still in progress\n");

  data[0] = size_in_bytes;
  if (sizeof (int) == 4) 
  {
    data[1] = 0;
    data[2] = (((unsigned int)mem_info[0]) & 0xFFFFFFFC) | 1;
  }
//  else  // 64 bit
//  {
//    data[1] = ((unsigned int)mem_info[0]) >> 32;
//    data[2] = (((unsigned int)mem_info[0]) & 0xFFFFFFFC) | 3;
//  }
  // set up DMA addr/lenstat
  err = Bar0Write(BAR0_DMA0_LENSTAT, 3, data);
  if (err) ERR_OUT("Could not write to BAR0_DMA0 registers\n");
  
  // wait for DMA0done; in real use, this should be in ISR
  gettimeofday(&t0, NULL); 
  err = StatusRegWaitForBit(BITPOS_DMA0_DONE);
  gettimeofday(&t1, NULL); 
  if (err) ERR_OUT("Failed StatusRegWaitForBit(BITPOS_DMA0_DONE)\n");
  
  RecordTime(&dma0Time, t0, t1);  
  dma0Bytes += size_in_bytes;

  return 0;
}

int DMAReadFromDev(int *size_in_bytes)  // data will be in mem_info[1]
{
  int err, temp;
  unsigned int data[3];
  struct timeval t0, t1;  

  // check if there's data to be read
  err = StatusRegWaitForBit(BITPOS_RX_READY);
  if (err) ERR_OUT("\nFailed StatusRegWaitForBit(BITPOS_RX_READY)\n");

  // check if DMA was pending
  err = Bar0Read(BAR0_DMA1_ADDR_LO, 1, &temp);
  if (err) ERR_OUT("Could not read from BAR0_DMA1_ADDR_LO to determine DMA in progress bit\n");
  if (temp & 1)
    ERR_OUT("DMA1 is still in progress\n");
  // clear BITPOS_RX_READY
  temp = 1<<BITPOS_DMA1_DONE;
  err = Bar0Write(BAR0_STATUS_REG, 1, &temp);
  if (err) ERR_OUT("Could not write BAR0_STATUS_REG to clear BITPOS_DMA1_DONE\n");

  // get length
  err = Bar0Read(BAR0_DMA1_LENSTAT, 1, size_in_bytes);
  if (err) ERR_OUT("Could not read BAR0_DMA1_LENSTAT\n");
  *size_in_bytes &= 0xFFFF;
  if (*size_in_bytes > TC_KMEM_LENGTH_BYTES)
    ERR_OUT("size_in_bytes > TC_KMEM_LENGTH_BYTES\n");

  if (sizeof (int) == 4) 
  {
    data[0] = 0;
    data[1] = (((unsigned int)mem_info[1]) & 0xFFFFFFFC) | 1;
  }
//  else  // 64 bit
//  {
//    data[0] = ((unsigned int)mem_info[1]) >> 32;
//    data[1] = (((unsigned int)mem_info[1]) & 0xFFFFFFFC) | 3;
//  }
  // set up DMA addr
  err = Bar0Write(BAR0_DMA1_ADDR_HI, 2, data);
  if (err) ERR_OUT("Could not write to BAR0_DMA0 registers\n");

  // wait for DMA1 done
  gettimeofday(&t0, NULL); 
  err = StatusRegWaitForBit(BITPOS_DMA1_DONE);
  gettimeofday(&t1, NULL); 
  if (err) ERR_OUT("\nFailed StatusRegWaitForBit(BITPOS_DMA1_DONE)\n");

  RecordTime(&dma1Time, t0, t1);  
  dma1Bytes += *size_in_bytes;
  
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// kernel memory functions

int CopyFromKmem(int index, int size_in_words, void *buffer)
{
  int err;
  if (index > TC_KMEM_COUNT)
    ERR_OUT("index > TC_KMEM_COUNT\n");
  if (size_in_words > TC_KMEM_LENGTH_BYTES/4)
    ERR_OUT("size_in_words > TC_KMEM_LENGTH_BYTES/4\n");

  param->index = index;
  param->offset = 0;
  param->length = size_in_words;
  err = ioctl(fd, TC_IOC_KMEM_RD, param);
  if (err)
  {
    printf("TC_IOC_KMEM_RD failed\n");
    return -1;
  }
  memcpy(buffer, &param->data[0], size_in_words*4);
  return 0;
}

int CopyToKmem(int index, int size_in_words, void *buffer)
{
  int err;
  if (index > TC_KMEM_COUNT)
    ERR_OUT("index > TC_KMEM_COUNT\n");
  if (size_in_words > TC_KMEM_LENGTH_BYTES/4)
    ERR_OUT("size_in_words > TC_KMEM_LENGTH_BYTES/4\n");

  memcpy(&param->data[0], buffer, size_in_words*4);

  param->index = index;
  param->offset = 0;
  param->length = size_in_words;
  err = ioctl(fd, TC_IOC_KMEM_WR, param);
  if (err)
  {
    printf("TC_IOC_KMEM_WR failed\n");
    return -1;
  }
  return 0;
}
/*
int CompareKmem01(int size_in_words) // compares KMEM 0 and 1
{
  int cnt, err;

  void *buf0, *buf1;
  buf0 = malloc(TC_KMEM_LENGTH_BYTES);
  if (!buf0) ERR_OUT("Could not allocate buf0\n");
  buf1 = malloc(TC_KMEM_LENGTH_BYTES);
  if (!buf1)
  {
    free(buf0);
    ERR_OUT("Could not allocate buf0\n");
  }

  err = CopyFromKmem(0, size_in_words, buf0);
  if (err) goto errOut;
  err = CopyFromKmem(1, size_in_words, buf1);
  if (err) goto errOut;

  for(cnt=0; cnt<size_in_words; cnt++)
  {
    if (buf0[cnt] != buf1[cnt])
      printf("NOT EQUAL: %4.4x: 0x%8.8x != 0x%8.8x\n", cnt, buf0[cnt], buf1[cnt]);
  }
errOut:
  free(buf0); free(buf1); return err;
}
*/

int DMAWriteReadCompare(int size_in_bytes)
{
  int *buf0, *buf1;
  int cnt, err, size_in_words;
  if (size_in_bytes > TC_KMEM_LENGTH_BYTES)
    ERR_OUT("size_in_bytes > TC_KMEM_LENGTH_BYTES\n");
  size_in_words = size_in_bytes / 4;
  if (size_in_bytes & 3) size_in_words++;
  
  // allocate user buffers
  buf0 = malloc(TC_KMEM_LENGTH_BYTES);
  if (!buf0) ERR_OUT("Could not allocate buf0\n");
  buf1 = malloc(TC_KMEM_LENGTH_BYTES);
  if (!buf1) 
  { 
    free(buf0);
    ERR_OUT("Could not allocate buf1\n");
  }
  
  // setup data to kmem0
  for(cnt=0; cnt<size_in_words; cnt++)
    buf0[cnt] = cnt;
  err = CopyToKmem(0, size_in_words, buf0);
  if (err) goto errOut;
  
  // clear kmem1; this will be overwritten by DMA
  for(cnt=0; cnt<size_in_words; cnt++)
    buf1[cnt] = 0;
  err = CopyToKmem(1, size_in_words, buf1);
  if (err) goto errOut;
  
  // DMA from kmem0 to device
  err = DMAWriteToDev(size_in_bytes);
  if (err) goto errOut;
  
  // DMA from device to kmem1; this waits for rxready 
  err = DMAReadFromDev(&cnt);
  if (err) goto errOut;
  if (cnt!= size_in_bytes)
  {
    printf("DMA to Dev size=%x, DMA from Dev size = %d\n", size_in_bytes, cnt);
    goto errOut;
  }

  // clear kmem0 and buf1, just in case SW error
  for(cnt=0; cnt<size_in_words; cnt++)
    buf1[cnt] = 0;
  err = CopyToKmem(0, size_in_words, buf1);
  if (err) goto errOut;
  
  // read data from kmem1 to local buf1
  err = CopyFromKmem(1, size_in_words, buf1);
  if (err) goto errOut;

  // compare what was wrote (buf0) to what was read (buf1)
  for(cnt=0; cnt<size_in_words; cnt++)
  {
    if (buf0[cnt] != buf1[cnt])
      printf("NOT EQUAL: %4.4x: 0x%8.8x != 0x%8.8x\n", cnt, buf0[cnt], buf1[cnt]);
  }
  
errOut:
  free(buf1);  
  free(buf0);  
}


////////////////////////////////////////////////////////////////////////////////

void FillParamData()
{
  int cnt, input;
  param->length = GetDec("Length: ");
  if (!param->length) return;
  input = GetDec("0=all 0, 1=all 0xff, 2=counter");
  for (cnt=0; cnt<param->length; cnt++)
  {
    switch(input)
    {
      case 0: param->data[cnt] = 0; break;
      case 1: param->data[cnt] = 0xFFFFFFFF; break;
      case 2: param->data[cnt] = cnt; break;
    }
  }
}

void PrintParamData(int count)
{
  int cnt;
  for (cnt=0; cnt<count; cnt++)
  {
    if (cnt%8 == 0) printf("%4.4x: ", cnt);
    printf("%8.8x", param->data[cnt]);
    if ((cnt+1)%8 == 0) printf("\n"); else printf(" ");
  }
}

void PrintSpeedStat()
{
  unsigned long t0, t1;
  double megaBytesPerSec;
  
  t0 = dma0Time.tv_sec * 1000000;
  t0 += dma0Time.tv_usec;
  megaBytesPerSec = dma0Bytes / (double)t0;
  printf("DMA0=%4.4f Mb/sec, ", 8*megaBytesPerSec);
  
  t0 = dma1Time.tv_sec * 1000000;
  t0 += dma1Time.tv_usec;
  megaBytesPerSec = dma1Bytes / (double)t0;
  printf("DMA1=%4.4f Mb/sec, ", 8*megaBytesPerSec);
  printf("              \r");
}

////////////////////////////////////////////////////////////////////////////////

int main()
{
  int err, cnt, offset, data, size_in_bytes;
  char input;
/*
  struct timeval t0, t1;  
  while(1)
  {
    gettimeofday(&t0, NULL); 
    gettimeofday(&t1, NULL); 
    RecordTime(&dma0Time, t0, t1);  
    dma0Bytes += 1024;
    PrintSpeedStat();
  }
  return 0;
*/

  fd = open("/dev/tc_pcie_driver", O_RDWR);
  if (!fd)
  {
    printf("Failed to open device\n");
    return -1;
  }
  param = (struct tc_ioc_data_struct *)malloc(sizeof(struct tc_ioc_data_struct) + TC_KMEM_LENGTH_BYTES);
  mem_info = (void **)malloc(sizeof (void*) * TC_KMEM_COUNT);
  err = ioctl(fd, TC_IOC_DRIVER_INFO, param);
  if (err)
  {
    printf("TC_IOC_DRIVER_INFO failed\n");
    goto exit;
  }
  char_buffer = (char *)param;
  printf("%s : %s", (char *)char_buffer, (char *)&char_buffer[strlen(char_buffer)+1]);

  err = ioctl(fd, TC_IOC_KMEM_INFO, mem_info);
  if (err)
  {
    printf("TC_IOC_KMEM_INFO failed\n");
    goto exit;
  }

  switch(sizeof(int))
  {
    case 4: printf("\nOS=32bit\n"); break;
    case 8: printf("\nOS=64bit\n"); break;
    default: printf("unsupported OS; not 32 or 64 bits"); return;
  }

  // main loop
  while(1)
  {
    printf("\n");
    printf(" 0. Exit\n");
    printf(" 1. TC_IOC_DRIVER_INFO\n");
    printf(" 2. TC_IOC_KMEM_INFO\n");
    printf(" 3. TC_IOC_KMEM_WR\n");
    printf(" 4. TC_IOC_KMEM_RD\n");
    printf(" 5. TC_IOC_BAR0_WR\n");
    printf(" 6. TC_IOC_BAR0_RD\n");
    printf(" 7. Write to dev using app\n");
    printf(" 8. Read from dev using app\n");
    printf(" 9. DMAWriteReadCompare\n");
    printf(" a. multiple DMAWriteReadCompare speed test\n");
    printf("Hello darling! "); 
    do
    {
      input = getchar();
    } while  (input == 13 || input == 10);
    printf("Thanks love.\n"); 
    if (input == '0') break;
    switch(input)
    {
      case '1': //  TC_IOC_DRIVER_INFO
        err = ioctl(fd, TC_IOC_DRIVER_INFO, param);
        if (err)
        {
          printf("TC_IOC_DRIVER_INFO failed\n");
          break;
        }
        char_buffer = (char *)param;
        printf("%s : %s", char_buffer, &char_buffer[strlen(char_buffer)+1]);
        break;

      case '2': // TC_IOC_KMEM_INFO
        err = ioctl(fd, TC_IOC_KMEM_INFO, mem_info);
        if (err)
        {
          printf("TC_IOC_KMEM_INFO failed\n");
          break;
        }
        for (cnt=0; cnt<TC_KMEM_COUNT; cnt++)
          printf("%d %16.16lx\n", cnt, (unsigned long)mem_info[cnt]);
        break;

      case '3': // TC_IOC_KMEM_WR
        param->index = GetDec("Index: ");
        param->offset = GetDec("Offset: ");
        FillParamData();  if (!param->length) break;
        err = ioctl(fd, TC_IOC_KMEM_WR, param);
        if (err)
        {
          printf("TC_IOC_KMEM_WR failed\n");
          break;
        }
        break;

      case '4': // TC_IOC_KMEM_RD
        param->index = GetDec("Index: ");
        param->offset = GetDec("Offset: ");
        param->length = GetDec("Length: ");
        cnt=param->length;
        err = ioctl(fd, TC_IOC_KMEM_RD, param);
        if (err)
        {
          printf("TC_IOC_KMEM_RD failed\n");
          break;
        }
        PrintParamData(cnt);
        break;

      case '5': // TC_IOC_BAR0_WR
        offset = GetDec("Offset: ");
        data = GetHex("Data 0x");
        err = Bar0Write(offset, 1, &data);
        break;

      case '6': // TC_IOC_BAR0_RD
        err=Bar0Read(GetDec("Offset: "), 1, &cnt);
        if (!err) printf("0x%8.8x", cnt);
        break;

      case '7': // Write to dev using app
        FillParamData();  if (!param->length) break;
        DMAWriteToDev(param->length*4);
        break;

      case '8': // Read from dev using app
        err = DMAReadFromDev(&size_in_bytes);
        if (err) break;
        PrintParamData(size_in_bytes/4);
        break;
        
      case '9': // DMA write/read/compare
        size_in_bytes = GetDec("size_in_bytes: ");
        DMAWriteReadCompare(size_in_bytes);
        PrintParamData(size_in_bytes/4);
        break;

      case 'a': case 'A': // multiple dma write/read/compare and speed test
        data = GetDec("number of times to run: ");
        size_in_bytes = GetDec("size_in_bytes: "); if (!size_in_bytes) break;
        dma0Time.tv_sec = 0; dma0Time.tv_usec = 0; dma0Bytes = 0;
        dma1Time.tv_sec = 0; dma1Time.tv_usec = 0; dma1Bytes = 0;
        for(cnt=0; cnt<data; cnt++)
        {
          DMAWriteReadCompare(size_in_bytes);
          printf("%d: ", cnt);
          PrintSpeedStat();
        }
        printf("\n");
        break;
      default:
        printf("%c Unsupported\n", input);
        break;

    }
  }  
exit:
  free(param);
  free(mem_info);
  close(fd);
  return 0;
}

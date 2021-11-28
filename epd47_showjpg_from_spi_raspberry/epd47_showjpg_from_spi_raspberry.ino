#include <ESP32DMASPISlave.h>
#include "Jpglib.h"

uint8_t *fb;

ESP32DMASPI::Slave slave;

// Reference: https://rabbit-note.com/2019/01/20/esp32-arduino-spi-slave/

/*
  编译大小 403KB
  本版本用了重启esp32技巧，避开spi-slave模式墨水屏不能显示位图的问题

  HSPI = CS: 15, CLK: 14, MOSI: 13, MISO: 12

  树莓派 <-> ESP32
  VCC      VCC
  GND      GND
  MISO     12
  MOSI     13
  SCLK     14
  CE1      15
*/

//ESP32 解析的jpg不能是8b色深，必须24b色深，所以jpg预留缓存要大一些，约200KB
uint32_t jpg_max_size = 200 * 1024; //200KB
//树莓派单次SPI传输数据受限4096
static const uint32_t BUFFER_SIZE = 4096;

char * last_jpg_fn = "/last.jpg";
uint8_t* spi_slave_tx_buf;
uint8_t* spi_slave_rx_buf;
uint8_t* jpg_buf;
uint32_t jpg_len;

void rebootESP() {
  Serial.print("Rebooting ESP32: ");
  //delay(100);
  Serial.flush();
  //ESP.restart();
  esp_restart();
}

void dump_buffer(uint32_t cnt) {
  Serial.print("slave receive : ");
  //uint8_t * p1;

  for (size_t i = 0; i < cnt; i++) {
    //p1 = spi_slave_rx_buf +  i
    // Serial.print(*p1, HEX);

    Serial.print(spi_slave_rx_buf[i], HEX);

    Serial.print(" ");
  }
  Serial.print("\n");
}

void dump_buffer_jpg(uint32_t cnt) {
  Serial.print("dump_buffer_jpg: ");
  for (size_t i = 0; i < cnt; i++) {
    Serial.print(jpg_buf[i], HEX);
    Serial.print(" ");
  }
  Serial.print("\n");
}

//保存jpg缓存数据流至spiffs
bool save_buff_spiffs()
{
  if (SPIFFS.exists(last_jpg_fn))
  {
    SPIFFS.remove(last_jpg_fn);
  }


  File  file = SPIFFS.open(last_jpg_fn, FILE_WRITE);
  if (!file)
  {
    Serial.println("write file error!");
    return false;
  }
  uint32_t writenum = 0;
  uint32_t write_now_num = 0;
  while (writenum < jpg_len)
  {
    if (jpg_len - writenum >= 1024)
      write_now_num = 1024;
    else
      write_now_num = jpg_len - writenum;
    file.write(&jpg_buf[writenum] , write_now_num);
    writenum = writenum + write_now_num;

  }
  file.close();
  Serial.println("save jpg_len=" + String(jpg_len) + ",writenum" + String(writenum));
}

void setup() {
  Serial.begin(115200);

  jpg_len = 0;
  jpg_buf = (byte*)ps_malloc(jpg_max_size);
  //jpg_buf = (byte*)malloc(jpg_max_size);
  //jpg_buf = (uint8_t*)heap_caps_malloc(jpg_max_size, MALLOC_CAP_DMA);


  if (!SPIFFS.begin(true))
  {
    ESP_LOGE("main", "SPIFFS begin failed!");
    return;
  }

  //如果存在上一次留下的jpg,进行显示
  //用来避开spi-slave模式下墨水屏显示不出位图的问题，可能是底层dma, spi资源 冲突
  if (SPIFFS.exists(last_jpg_fn))
  {
    epd_init();

    fb = (uint8_t *)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT / 2, MALLOC_CAP_SPIRAM);
    memset(fb, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

    init_jpglib(fb);

    memset(fb, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
    show_jpg_from_spiffs(last_jpg_fn); // 平均需要2.3秒

    //没初始化spi-slave前显示出jpg
    SPIFFS.remove(last_jpg_fn);

    //重启，切换回spi-slave模式
    rebootESP();
  }

  // to use DMA buffer, use these methods to allocate buffer
  spi_slave_tx_buf = slave.allocDMABuffer(BUFFER_SIZE);
  spi_slave_rx_buf = slave.allocDMABuffer(BUFFER_SIZE);

  jpg_len = 0;
  delay(500);

  slave.setDataMode(SPI_MODE0);
  slave.setMaxTransferSize(BUFFER_SIZE);
  slave.setDMAChannel(1);  // 1 or 2 only
  slave.setQueueSize(1);   // transaction queue size
  // begin() after setting
  // HSPI = CS: 15, CLK: 14, MOSI: 13, MISO: 12
  slave.begin(HSPI);

  // connect same name pins each other
  // CS - CS, CLK - CLK, MOSI - MOSI, MISO - MISO
  Serial.print("\nslave begin...\n");
}

void loop() {

  slave.wait(spi_slave_rx_buf, spi_slave_tx_buf, BUFFER_SIZE);


  uint8_t * p1;
  uint8_t * p2;
  uint8_t * p3;
  uint8_t * p4;

  int slave_size = 0;
  // if slave has received transaction data, available() returns size of received transactions
  while (slave.available()) {
    slave_size = slave.size();
    printf("slave received size = %d\n", slave_size);
    //dump_buffer(slave.size());
    p1 = spi_slave_rx_buf;
    p2 = spi_slave_rx_buf + 1;
    p3 = spi_slave_rx_buf + 2;
    p4 = spi_slave_rx_buf + 3;

    if (slave_size == 4 && (char)*p1 == '>' && (char)*p2 == '>' && (char)*p3 == '>' && (char)*p4 == '>')
    {
      printf("begin file\n");
      jpg_len = 0;
      //dump_buffer(slave.size());
    }
    //
    else if (slave_size == 4 &&  (char)*p1 == '<' && (char)*p2 == '<' && (char)*p3 == '<' && (char)*p4 == '<')
    {
      printf("end file\n");

      //将buff转存到spiffs,重启
      //避开spi-slave模式下墨水屏模式无法显示位图的资源冲突问题
      save_buff_spiffs();
      //重启，切换回墨水屏初始模式
      rebootESP();

      jpg_len = 0;
    }
    else
    {
      //printf(".");
      //以下两种写法效果一样
      memcpy(&jpg_buf[jpg_len], spi_slave_rx_buf, slave_size);
      //memcpy(jpg_buf+jpg_len, spi_slave_rx_buf, slave_size);
      jpg_len = jpg_len + slave_size;
    }


    slave.pop();
  }


}

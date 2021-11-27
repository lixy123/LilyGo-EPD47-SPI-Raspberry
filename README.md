# LilyGo-EPD47-SPI-Raspberry <br/>
树莓派SPI 方式把LilyGo EPD47 4.7寸墨水屏当下位机显示位图 <br/>

<b>一.功能：</b> <br/>
   树莓派当上位机，通过SPI方式将JPG图像传给ESP32主控的LilyGo EPD47 4.7寸墨水屏显示. <br/>
   满屏图SPI传输时间<1秒， 显示时间<3秒，合计约3-4秒左右.  <br/>
   速度提升余地有限. <br/>
   
   最终演化：<br/>
   1.树莓派与墨水屏SPI连接，树莓派与此墨水屏整合成一个整体: <br/>
   利用树莓派的丰富功能和存储能力显示天气，日历安排，TODO, 照片墙等。也可以运行微信公众号后台，手机通过微信公众号将手写图片通过手机远程写到墨水屏上图像留言等等. <br/>
   2.墨水屏独立运行，无需树莓派: <br/>
   wifi打开一个端口文件传图并显示。单独使用。功能不如上一方案丰富<br/>


<b>二.硬件引脚连接:</b>  <br/>
  树莓派 <-> ESP32 <br/>
  VCC      VCC <br/>
  GND      GND <br/>
  MISO     12 <br/>
  MOSI     13 <br/>
  SCLK     14 <br/>
  CE1      15 <br/>
   
  <img src= 'https://github.com/lixy123/LilyGo-EPD47-SPI-Raspberry/blob/main/main.jpg?raw=true' /> <br/>
   
<b>三.源码说明:</b>  <br/>
<b>1.raspberry</b>  <br/>
   A.文件拷到树莓派上 <br/>
   B.运行：python3 sendjpg_epd47_by_spi.py <br/>
   
<b>2.epd47_showjpg_from_spi_raspberry</b>  <br/>
  烧录到LilyGo EPD47 4.7寸墨水屏 <br/>
  A.arduino 版本 1.8.13 <br/>
  B.使用库文件: <br/>
  ESP32库 arduino-esp32 版本 1.0.6 <br/>
  墨水屏库 https://github.com/Xinyuan-LilyGO/LilyGo-EPD47 <br/>
  SPI库 https://github.com/hideakitai/ESP32SPISlave <br/>
     

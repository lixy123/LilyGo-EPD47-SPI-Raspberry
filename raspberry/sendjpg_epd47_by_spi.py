import spidev
import time

#https://blog.csdn.net/qq_42842335/article/details/85853682

'''
// 80M极限  
//32000000 32k 10ms 不稳定
// 8000000 32k 33ms 稳定性好
注意：
esp32当spi下位机，每次发送长度必须是4的整数倍，且不可以小于4,
否则本次末位几个字符，下次发送的字符会混乱
'''
 
def append_byte(msg, div_cnt):
    ret_msg=msg
    p1= len(msg) % div_cnt
    if p1 !=0:
        for i in range(div_cnt-p1):
            ret_msg=ret_msg + b' ' 
    return ret_msg            
    
bus = 0    #supporyed values:0,1
#0 cs接CE0
#1 cs接CE1
device = 1   #supported values:0,1   default: 0

spi = spidev.SpiDev()
spi.open(bus,device)    #连接到指定的spi设备     /dev/spidev<bus>.<device>
#spi.mode = 00  默认，不用设置
spi.max_speed_hz = 4000000  #还能加大？待测试

#注意：ESP32 JPG图片要求:
#色深24b, 对于8b ESP32的库不支持显示
def send_jpg(fn_jpg):
    timestamp = time.time() 
    
    #1.发送起始标志
    spi.writebytes(b'>>>>')
    time.sleep(0.05) #时间还能缩小？待测试
    fn_size=0
    
    #2.发送文件内容，拆成4094字节分批发送

    with open(fn_jpg, 'rb') as fp:
        while True: 
            
            content=fp.read(4096)
            if len(content)==0:
                break
            print("",len(content))
            fn_size=fn_size+len(content)
            #此函数最多一次4096字节，否则异常
            spi.writebytes(append_byte(content,4))
            time.sleep(0.05)
    print("fn_size=",fn_size)  
    
    #发送结束标志
    spi.writebytes(b'<<<<') 
    print("send_jpg time :",time.time()-timestamp,"ms")
  
while True:  
    send_jpg('/home/pi/1.jpg') 
    time.sleep(5)
    send_jpg('/home/pi/2.jpg')   
    time.sleep(5)
    send_jpg('/home/pi/3.jpg')  
    time.sleep(5)
spi.close() 




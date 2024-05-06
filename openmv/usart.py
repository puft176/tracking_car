import time,sensor,image,display
from pyb import UART
from pyb import LED

sensor.reset()                       #初始化摄像头
#sensor.set_vflip(True)               #图像倒置
#sensor.set_hmirror(True)
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)    #320x240 160x120
sensor.skip_frames(time=2000)        #等待设置生效
sensor.set_auto_gain(False) # 颜色跟踪必须关闭自动增益
sensor.set_auto_whitebal(False) # 颜色跟踪必须关闭白平衡clock = time.clock()
lcd = display.SPIDisplay()           #初始化LCD屏幕

Range = [0,0,160,120]

#黑白色块初始阈值
white = [65, 100, -43, 40, -50, 7]
black = [0, 34, -32, 23, -35, 23]
thr_3 = [50, 93, -12, 80, 30, 83] #黄色

thrchoose = [white,black]

#阈值调节函数
def thrset():
    global thr_enable
    if(command == 1):
        thr_enable = 1    #阈值调节使能
        thr = bytearray([0xF1,0xF2,
                        abs(thr_3[0]),abs(thr_3[1]),abs(thr_3[2]),
                        abs(thr_3[3]),abs(thr_3[4]),abs(thr_3[5]),
                        0xF3])

        uart.write(thr)

    if(command == 2): thr_enable = 0    #阈值调节失能

    if(thr_enable):
        if(3 <= command and command <= 14):         #command 3~14 为对阈值1进行调节
            if(command % 2 == 0):
                thr_3[command // 2 - 2] += 5
            else:
                thr_3[command // 2 - 1] -= 5
            thr = bytearray([0xFF,
                            abs(thr_3[0]),abs(thr_3[1]),abs(thr_3[2]),
                            abs(thr_3[3]),abs(thr_3[4]),abs(thr_3[5]),
                            0xFE])
            uart.write(thr)
            print(thr)
        img.binary([thr_3])        #根据阈值进行图像二值化
        lcd.write(img,  roi = (96,40,128,160))  # Take a picture and display the image.

    return

#寻找最大色块
def find_max(blobs):
    max_size=0
    if(len(blobs) ==0):
        return 0
    for blob in blobs:
        if blob[2]*blob[3] > max_size:
            max_blob = blob
            max_size = blob[2]*blob[3]
    return max_blob

#小球云台追踪函数
def balltrack():
    global balltrack_en
    if(command == 39): balltrack_en = 1 #小球追踪使能
    if(command == 40): balltrack_en = 0 #小球追踪失能
    if(balltrack_en == 1):
        #识别小球
            #pixels_threshold=100:滤掉像素少于100的色块，merge=True:合并未被滤掉的色块，margin=10:增大或减小色块边界矩形的大小
        blobs = img.find_blobs([thr_3])
        blob = find_max(blobs)
        if(blob):           #找到色块
            img.draw_rectangle(blob.rect())
            img.draw_cross(blob.cx(), blob.cy())
            img.draw_cross(160,120)
            #小球中心坐标,方框面积返回
            rectarea = blob[2] * blob[3]
            ballpos = bytearray([0xFF,
                                int(blob.cx()),int(blob.cy()),
                                rectarea // 255,rectarea % 255,0x00,0x00,
                                0xFE])
            uart.write(ballpos)
            print(ballpos)
        else:               #视野内无色块
            ballpos = bytearray([0xFF,
                                0x00,0x01,0x00,0x00,0x00,0x00,
                                0xFE])
            uart.write(ballpos)
            print(ballpos)
    return

#巡线函数
def linetrack():

    global linetrack_en
    average_x = 0
    list_x = []
    if(command == 46):
        linetrack_en = 1
        LED(1).on()
        LED(2).on()
        LED(3).on()
    if(command == 47):
        linetrack_en = 0
        LED(1).off()
        LED(2).off()
        LED(3).off()

    if(linetrack_en == 1):
        #识别黑白色块

        blobs = img.find_blobs([white,black],pixels_threshold=400,x_stride = 10,y_stride = 10)
        blobs = blob_filter(blobs)
        for blob in blobs:
            img.draw_rectangle(blob.rect())
            img.draw_cross(blob.cx(), blob.cy())
            if(average_x == 0):
                average_x += blob.cx()
                continue
            average_x += blob.cx()
            average_x /= 2
        uart.write(bytearray([int(average_x/2)]))
        print(average_x)
        img.draw_cross(160,120)
    return

#色块筛选函数
def blob_filter(blobs):
    blobs1 = []
    #剔除面积过大色块
    for blob in blobs:
        if(blob[2]*blob[3] <= 1700):
            blobs1.append(blob)
    return blobs1
#    if len(blobs1)==0: return []   #若没有符合要求的色块，则跳过以下全部



#LCD复位函数

#指令获取函数
def cmdget():
    global command
    if(uart.any()):
        command = uart.readchar()           #接收串口数据
        print(command)
    else:
        command = 0
    return

uart = UART(3, 9600)        #使用usart3，波特率9600

command = 0                 #命令储存变量
thr_enable = 0              #阈值调节模式使能标志位
balltrack_en = 0            #小球追踪模式使能标志位
linetrack_en = 0            #巡线模式使能标志位

lcd = display.SPIDisplay()  # Initialize the lcd screen.
while(True):
    img = sensor.snapshot()#.binary([thr_3])  #摄像头截取图片
    cmdget()
    thrset()                 #检测是否进入阈值调节模式
    linetrack()
    balltrack()              #检测是否进入云台追踪
#    lcd.write(img,  roi = (96,40,128,160))  # Take a picture and display the image.
#    img.binary([thr_3])        #根据阈值进行图像二值化
#    lcd.write(img)



#    uart.write('A')
#    time.sleep_ms(1000)


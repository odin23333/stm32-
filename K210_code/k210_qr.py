import sensor, image, time, lcd
from modules import ybserial

#==========================================================================
# K210程序：二维码识别模式
# 功能：识别二维码并发送内容
# 协议：$ + 二维码内容 + #
#==========================================================================

# 初始化串口
ser = ybserial()

# 初始化摄像头和LCD
lcd.init()
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)  # 320x240
sensor.skip_frames(time=100)
clock = time.clock()

#==========================================================================
# 串口发送函数
#==========================================================================

def send_qr_data(msg):
    """发送二维码数据: $ + 内容 + #"""
    if msg:
        send_buf = '$' + msg + '#'
        ser.send(send_buf)

#==========================================================================
# 主循环
#==========================================================================

print("K210 QR Code Detection Started!")
print("Detecting QR codes...")

while True:
    clock.tick()
    img = sensor.snapshot()

    # 查找二维码
    qr_codes = img.find_qrcodes()

    if qr_codes:
        for code in qr_codes:
            # 绘制矩形框
            img.draw_rectangle(code.rect(), color=(255, 0, 0), thickness=3)
            img.draw_string(code.x(), code.y() - 20, code.payload(), color=(255, 0, 0), scale=2)

            # 发送二维码数据
            send_qr_data(code.payload())

            # 显示信息
            print("QR Code:", code.payload())
    else:
        # 没有检测到二维码
        send_qr_data("")

    # 显示FPS
    fps = clock.fps()
    img.draw_string(0, 0, "FPS:%2.1f" % fps, color=(255, 255, 255), scale=2.0)
    img.draw_string(0, 30, "QR Code Mode", color=(255, 255, 255), scale=1.5)

    lcd.display(img)

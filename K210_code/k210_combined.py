import sensor, image, time, lcd
from modules import ybserial

#==========================================================================
# K210合并程序：颜色识别 + 二维码识别
# 同时运行两个功能，STM32根据模式处理不同数据
#==========================================================================

# 初始化串口
ser = ybserial()

# 初始化摄像头和LCD
lcd.init()
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)  # 320x240
sensor.skip_frames(time=100)
sensor.set_auto_gain(False)
sensor.set_auto_whitebal(False)
clock = time.clock()

#==========================================================================
# 颜色阈值定义 (LAB色彩空间)
#==========================================================================

# 红色阈值
red_threshold = (30, 70, 40, 80, 20, 70)

# 绿色阈值
green_threshold = (30, 70, -70, -20, 10, 60)

# 黄色阈值
yellow_threshold = (50, 90, -20, 20, 50, 90)

# 颜色阈值列表
color_thresholds = [
    (red_threshold, "RED", (255, 0, 0)),
    (green_threshold, "GREEN", (0, 255, 0)),
    (yellow_threshold, "YELLOW", (255, 255, 0))
]

#==========================================================================
# 串口发送函数
#==========================================================================

def send_color_data(r, g, y):
    """发送颜色计数数据: $C + R + G + Y + #"""
    r = min(r, 9)
    g = min(g, 9)
    y = min(y, 9)
    send_buf = '$C' + str(r) + str(g) + str(y) + '#'
    ser.send(send_buf)

def send_qr_data(msg):
    """发送二维码数据: $ + 内容 + #"""
    if msg:
        send_buf = '$' + msg + '#'
        ser.send(send_buf)

#==========================================================================
# 主循环 - 同时运行两个功能
#==========================================================================

print("K210 Combined Program Started!")
print("Running: Color Detection + QR Code Detection")

while True:
    clock.tick()
    img = sensor.snapshot()

    # ========== 二维码识别 ==========
    qr_codes = img.find_qrcodes()
    qr_found = False

    if qr_codes:
        for code in qr_codes:
            # 绘制矩形框
            img.draw_rectangle(code.rect(), color=(255, 0, 0), thickness=3)
            img.draw_string(code.x(), code.y() - 20, code.payload(), color=(255, 0, 0), scale=2)

            # 发送二维码数据
            send_qr_data(code.payload())
            qr_found = True

    # ========== 颜色识别 ==========
    red_count = 0
    green_count = 0
    yellow_count = 0

    for threshold, color_name, draw_color in color_thresholds:
        blobs = img.find_blobs([threshold],
                               pixels_threshold=200,
                               area_threshold=200,
                               merge=True,
                               margin=10)

        for blob in blobs:
            img.draw_rectangle(blob.rect(), color=draw_color)
            img.draw_cross(blob.cx(), blob.cy(), color=draw_color)

            if color_name == "RED":
                red_count += 1
            elif color_name == "GREEN":
                green_count += 1
            elif color_name == "YELLOW":
                yellow_count += 1

            img.draw_string(blob.x(), blob.y() - 15, color_name, color=draw_color, scale=1.5)

    # 发送颜色数据
    send_color_data(red_count, green_count, yellow_count)

    # ========== 显示信息 ==========
    fps = clock.fps()
    img.draw_string(0, 0, "FPS:%2.1f" % fps, color=(255, 255, 255), scale=2.0)

    if qr_found:
        img.draw_string(0, 25, "QR:Detected", color=(255, 0, 0), scale=1.5)
    else:
        img.draw_string(0, 25, "QR:None", color=(128, 128, 128), scale=1.5)

    img.draw_string(0, 50, "R:%d G:%d Y:%d" % (red_count, green_count, yellow_count), color=(255, 255, 255), scale=1.5)

    lcd.display(img)

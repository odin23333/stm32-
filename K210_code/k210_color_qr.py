import sensor, image, time, lcd
from modules import ybserial

#==========================================================================
# K210合并程序：颜色识别 + 二维码识别
# 功能1：识别红、绿、黄三种颜色小球并计数
# 功能2：识别二维码并发送内容
# 默认运行颜色识别模式
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
# 功能1：颜色识别
#==========================================================================

def color_detect(img):
    """颜色识别：识别红、绿、黄三种颜色小球并计数"""
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

    # 显示统计信息
    fps = clock.fps()
    img.draw_string(0, 0, "FPS:%2.1f" % fps, color=(255, 255, 255), scale=2.0)
    img.draw_string(0, 25, "MODE:COLOR", color=(255, 255, 255), scale=1.5)
    img.draw_string(0, 50, "R:%d G:%d Y:%d" % (red_count, green_count, yellow_count), color=(255, 255, 255), scale=1.5)

    total = red_count + green_count + yellow_count
    img.draw_string(0, 75, "Total:%d" % total, color=(255, 255, 255), scale=1.5)

    # 发送数据
    send_color_data(red_count, green_count, yellow_count)

#==========================================================================
# 功能2：二维码识别
#==========================================================================

def qr_detect(img):
    """二维码识别：识别二维码并发送内容"""
    qr_codes = img.find_qrcodes()

    if qr_codes:
        for code in qr_codes:
            img.draw_rectangle(code.rect(), color=(255, 0, 0), thickness=3)
            img.draw_string(code.x(), code.y() - 20, code.payload(), color=(255, 0, 0), scale=2)

            # 显示信息
            fps = clock.fps()
            img.draw_string(0, 0, "FPS:%2.1f" % fps, color=(255, 255, 255), scale=2.0)
            img.draw_string(0, 25, "MODE:QR", color=(255, 255, 255), scale=1.5)
            img.draw_string(0, 50, "QR:" + code.payload(), color=(255, 255, 255), scale=1.5)

            # 发送二维码数据
            send_qr_data(code.payload())
    else:
        # 显示信息
        fps = clock.fps()
        img.draw_string(0, 0, "FPS:%2.1f" % fps, color=(255, 255, 255), scale=2.0)
        img.draw_string(0, 25, "MODE:QR", color=(255, 255, 255), scale=1.5)
        img.draw_string(0, 50, "No QR", color=(128, 128, 128), scale=1.5)

        # 发送空数据
        send_qr_data("")

#==========================================================================
# 主循环 - 默认运行颜色识别
#==========================================================================

print("K210 Color + QR Detection Started!")
print("Default Mode: Color Detection")

while True:
    clock.tick()
    img = sensor.snapshot()

    # 默认运行颜色识别
    color_detect(img)

    lcd.display(img)

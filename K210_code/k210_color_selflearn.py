import sensor, image, time, lcd
from modules import ybserial

#==========================================================================
# K210程序：颜色识别模式（替代自主学习模式）
# 功能：识别红、绿、黄三种颜色小球并计数
# 协议：$C + R + G + Y + #
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
    """发送颜色计数数据: $C + R + G + Y + # (每种颜色限制最大9个)"""
    # 限制数量最大为9
    r = min(r, 9)
    g = min(g, 9)
    y = min(y, 9)
    send_buf = '$C' + str(r) + str(g) + str(y) + '#'
    ser.send(send_buf)
    print("Send:", send_buf)

#==========================================================================
# 主循环
#==========================================================================

print("K210 Color Detection Started!")
print("Detecting: RED, GREEN, YELLOW balls")

while True:
    clock.tick()
    img = sensor.snapshot()

    # 统计每种颜色的球数量
    red_count = 0
    green_count = 0
    yellow_count = 0

    # 检测每种颜色
    for threshold, color_name, draw_color in color_thresholds:
        blobs = img.find_blobs([threshold],
                               pixels_threshold=200,
                               area_threshold=200,
                               merge=True,
                               margin=10)

        for blob in blobs:
            # 绘制矩形框和十字标记
            img.draw_rectangle(blob.rect(), color=draw_color)
            img.draw_cross(blob.cx(), blob.cy(), color=draw_color)

            # 计数
            if color_name == "RED":
                red_count += 1
            elif color_name == "GREEN":
                green_count += 1
            elif color_name == "YELLOW":
                yellow_count += 1

            # 显示颜色名称
            img.draw_string(blob.x(), blob.y() - 15, color_name, color=draw_color, scale=1.5)

    # 显示统计信息
    fps = clock.fps()
    img.draw_string(0, 0, "FPS:%2.1f" % fps, color=(255, 255, 255), scale=2.0)
    img.draw_string(0, 30, "Red:%d" % red_count, color=(255, 0, 0), scale=2.0)
    img.draw_string(0, 60, "Green:%d" % green_count, color=(0, 255, 0), scale=2.0)
    img.draw_string(0, 90, "Yellow:%d" % yellow_count, color=(255, 255, 0), scale=2.0)

    total = red_count + green_count + yellow_count
    img.draw_string(0, 120, "Total:%d" % total, color=(255, 255, 255), scale=2.0)

    lcd.display(img)

    # 发送数据
    send_color_data(red_count, green_count, yellow_count)
